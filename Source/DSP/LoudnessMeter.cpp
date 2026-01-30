#include "LoudnessMeter.h"
#include <algorithm>
#include <numeric>
#include <cmath>

LoudnessMeter::LoudnessMeter()
{
}

void LoudnessMeter::prepare(double sampleRate, int samplesPerBlock)
{
    currentSampleRate = sampleRate;
    currentBlockSize = samplesPerBlock;

    // Calculate K-weighting filter coefficients (ITU-R BS.1770-4)
    // Stage 1: High shelf boosting high frequencies
    // f0 = 1681.97Hz, gain = +3.999dB, Q = 0.7072
    {
        float f0 = 1681.97f;
        float G = 3.999f;
        float Q = 0.7072f;
        float K = std::tan(DSPUtils::PI * f0 / static_cast<float>(sampleRate));
        float Vh = std::pow(10.0f, G / 20.0f);
        float Vb = std::pow(Vh, 0.5f);

        float a0 = 1.0f + K / Q + K * K;
        kShelfCoeffs.b0 = (Vh + Vb * K / Q + K * K) / a0;
        kShelfCoeffs.b1 = 2.0f * (K * K - Vh) / a0;
        kShelfCoeffs.b2 = (Vh - Vb * K / Q + K * K) / a0;
        kShelfCoeffs.a1 = 2.0f * (K * K - 1.0f) / a0;
        kShelfCoeffs.a2 = (1.0f - K / Q + K * K) / a0;
    }

    // Stage 2: High-pass filter
    // f0 = 38.1355Hz, Q = 0.5003
    {
        float f0 = 38.1355f;
        float Q = 0.5003f;
        kHpfCoeffs = DSPUtils::calculateHighPass(static_cast<float>(sampleRate), f0, Q);
    }

    // Calculate buffer sizes
    momentarySamples = static_cast<int>(sampleRate * 0.4);   // 400ms
    shortTermSamples = static_cast<int>(sampleRate * 3.0);   // 3s
    samplesPerBlock100ms = static_cast<int>(sampleRate * 0.1); // 100ms blocks

    // Prepare oversampler for true peak detection
    oversampler.initProcessing(samplesPerBlock);
    oversampleBuffer.resize(samplesPerBlock * 4);

    // Allocate working buffers
    kWeightedL.resize(samplesPerBlock);
    kWeightedR.resize(samplesPerBlock);

    reset();
}

void LoudnessMeter::reset()
{
    momentaryBuffer.clear();
    shortTermBuffer.clear();
    integratedBlocks.clear();
    lraBlocks.clear();

    kWeightL = {};
    kWeightR = {};

    integratedSum = 0.0f;
    integratedBlockCount = 0;
    currentBlockSamples = 0;
    currentBlockSum = 0.0f;

    momentaryLUFS.store(-100.0f);
    shortTermLUFS.store(-100.0f);
    integratedLUFS.store(-100.0f);
    peakLevel.store(-100.0f);
    truePeakLevel.store(-100.0f);
    stereoCorrelation.store(1.0f);
    stereoBalance.store(0.0f);
}

void LoudnessMeter::resetIntegrated()
{
    integratedBlocks.clear();
    lraBlocks.clear();
    integratedSum = 0.0f;
    integratedBlockCount = 0;
    integratedLUFS.store(-100.0f);
    loudnessRange.store(0.0f);
}

void LoudnessMeter::applyKWeighting(const float* input, float* output, int numSamples, int channel)
{
    KWeightingState& state = (channel == 0) ? kWeightL : kWeightR;

    for (int i = 0; i < numSamples; ++i)
    {
        // Stage 1: High shelf
        float s1_out = kShelfCoeffs.b0 * input[i] + kShelfCoeffs.b1 * state.s1_x1 + kShelfCoeffs.b2 * state.s1_x2
                       - kShelfCoeffs.a1 * state.s1_y1 - kShelfCoeffs.a2 * state.s1_y2;
        state.s1_x2 = state.s1_x1;
        state.s1_x1 = input[i];
        state.s1_y2 = state.s1_y1;
        state.s1_y1 = s1_out;

        // Stage 2: High-pass
        float s2_out = kHpfCoeffs.b0 * s1_out + kHpfCoeffs.b1 * state.s2_x1 + kHpfCoeffs.b2 * state.s2_x2
                       - kHpfCoeffs.a1 * state.s2_y1 - kHpfCoeffs.a2 * state.s2_y2;
        state.s2_x2 = state.s2_x1;
        state.s2_x1 = s1_out;
        state.s2_y2 = state.s2_y1;
        state.s2_y1 = s2_out;

        output[i] = s2_out;
    }
}

void LoudnessMeter::calculateTruePeak(const float* input, int numSamples, int channel)
{
    // Simple peak detection without full oversampling for efficiency
    // A proper implementation would use 4x oversampling
    float maxPeak = 0.0f;
    for (int i = 0; i < numSamples; ++i)
    {
        maxPeak = std::max(maxPeak, std::abs(input[i]));
    }

    float currentTruePeak = truePeakLevel.load();
    float peakDb = DSPUtils::linearToDecibels(maxPeak);
    if (peakDb > currentTruePeak)
        truePeakLevel.store(peakDb);
}

void LoudnessMeter::calculateCorrelation(const float* left, const float* right, int numSamples)
{
    float sumLR = 0.0f;
    float sumL2 = 0.0f;
    float sumR2 = 0.0f;
    float sumL = 0.0f;
    float sumR = 0.0f;

    for (int i = 0; i < numSamples; ++i)
    {
        sumLR += left[i] * right[i];
        sumL2 += left[i] * left[i];
        sumR2 += right[i] * right[i];
        sumL += left[i];
        sumR += right[i];
    }

    float denominator = std::sqrt(sumL2 * sumR2);
    float correlation = (denominator > 0.0001f) ? sumLR / denominator : 0.0f;
    stereoCorrelation.store(std::clamp(correlation, -1.0f, 1.0f));

    // Stereo balance (-1 = full left, +1 = full right)
    float levelL = sumL2 > 0.0f ? std::sqrt(sumL2 / numSamples) : 0.0f;
    float levelR = sumR2 > 0.0f ? std::sqrt(sumR2 / numSamples) : 0.0f;
    float totalLevel = levelL + levelR;
    float balance = (totalLevel > 0.0001f) ? (levelR - levelL) / totalLevel : 0.0f;
    stereoBalance.store(balance);
}

void LoudnessMeter::process(const juce::AudioBuffer<float>& buffer)
{
    const int numChannels = buffer.getNumChannels();
    const int numSamples = buffer.getNumSamples();

    if (numChannels < 1 || numSamples < 1) return;

    const float* leftData = buffer.getReadPointer(0);
    const float* rightData = numChannels > 1 ? buffer.getReadPointer(1) : leftData;

    // Peak level detection
    float maxPeak = 0.0f;
    for (int ch = 0; ch < numChannels; ++ch)
    {
        const float* data = buffer.getReadPointer(ch);
        for (int i = 0; i < numSamples; ++i)
            maxPeak = std::max(maxPeak, std::abs(data[i]));
    }
    float currentPeak = peakLevel.load();
    float newPeakDb = DSPUtils::linearToDecibels(maxPeak);
    if (newPeakDb > currentPeak)
        peakLevel.store(newPeakDb);
    else
        peakLevel.store(currentPeak * 0.99f + newPeakDb * 0.01f); // Slow decay

    // True peak
    calculateTruePeak(leftData, numSamples, 0);
    if (numChannels > 1)
        calculateTruePeak(rightData, numSamples, 1);

    // Stereo correlation and balance
    if (numChannels > 1)
        calculateCorrelation(leftData, rightData, numSamples);

    // Apply K-weighting
    applyKWeighting(leftData, kWeightedL.data(), numSamples, 0);
    if (numChannels > 1)
        applyKWeighting(rightData, kWeightedR.data(), numSamples, 1);
    else
        std::copy(kWeightedL.begin(), kWeightedL.begin() + numSamples, kWeightedR.begin());

    // Calculate mean square for this block (both channels)
    float sumSquares = 0.0f;
    for (int i = 0; i < numSamples; ++i)
    {
        sumSquares += kWeightedL[i] * kWeightedL[i];
        sumSquares += kWeightedR[i] * kWeightedR[i];
    }
    float meanSquare = sumSquares / (2.0f * numSamples);

    // Add to momentary buffer
    for (int i = 0; i < numSamples; ++i)
    {
        float samplePower = (kWeightedL[i] * kWeightedL[i] + kWeightedR[i] * kWeightedR[i]) * 0.5f;
        momentaryBuffer.push_back(samplePower);
        shortTermBuffer.push_back(samplePower);
    }

    // Trim buffers to window size
    while (static_cast<int>(momentaryBuffer.size()) > momentarySamples)
        momentaryBuffer.pop_front();
    while (static_cast<int>(shortTermBuffer.size()) > shortTermSamples)
        shortTermBuffer.pop_front();

    // Update momentary loudness (400ms window)
    updateMomentary();

    // Update short-term loudness (3s window)
    updateShortTerm();

    // Update 100ms blocks for integrated/LRA
    currentBlockSum += meanSquare * numSamples;
    currentBlockSamples += numSamples;

    if (currentBlockSamples >= samplesPerBlock100ms)
    {
        float blockMeanSquare = currentBlockSum / currentBlockSamples;
        float blockLUFS = -0.691f + 10.0f * std::log10(std::max(blockMeanSquare, 1e-10f));

        // Store for integrated calculation (with gating)
        if (blockLUFS > ABSOLUTE_GATE)
        {
            integratedBlocks.push_back(blockMeanSquare);
            lraBlocks.push_back(blockLUFS);
        }

        currentBlockSum = 0.0f;
        currentBlockSamples = 0;

        // Update integrated loudness
        updateIntegrated();
    }
}

void LoudnessMeter::updateMomentary()
{
    if (momentaryBuffer.empty()) return;

    float sum = 0.0f;
    for (float val : momentaryBuffer)
        sum += val;

    float meanSquare = sum / momentaryBuffer.size();
    float lufs = -0.691f + 10.0f * std::log10(std::max(meanSquare, 1e-10f));
    momentaryLUFS.store(lufs);
}

void LoudnessMeter::updateShortTerm()
{
    if (shortTermBuffer.empty()) return;

    float sum = 0.0f;
    for (float val : shortTermBuffer)
        sum += val;

    float meanSquare = sum / shortTermBuffer.size();
    float lufs = -0.691f + 10.0f * std::log10(std::max(meanSquare, 1e-10f));
    shortTermLUFS.store(lufs);
}

void LoudnessMeter::updateIntegrated()
{
    if (integratedBlocks.empty())
    {
        integratedLUFS.store(-100.0f);
        return;
    }

    // First pass: calculate ungated loudness
    float sumAll = 0.0f;
    for (float ms : integratedBlocks)
        sumAll += ms;
    float ungatedMean = sumAll / integratedBlocks.size();
    float ungatedLUFS = -0.691f + 10.0f * std::log10(std::max(ungatedMean, 1e-10f));

    // Second pass: apply relative gate
    float relativeGate = ungatedLUFS + RELATIVE_GATE;
    float gatedSum = 0.0f;
    int gatedCount = 0;

    for (float ms : integratedBlocks)
    {
        float blockLUFS = -0.691f + 10.0f * std::log10(std::max(ms, 1e-10f));
        if (blockLUFS > relativeGate)
        {
            gatedSum += ms;
            gatedCount++;
        }
    }

    if (gatedCount > 0)
    {
        float gatedMean = gatedSum / gatedCount;
        float intLUFS = -0.691f + 10.0f * std::log10(std::max(gatedMean, 1e-10f));
        integratedLUFS.store(intLUFS);

        // Calculate loudness range (LRA)
        if (lraBlocks.size() > 10)
        {
            std::vector<float> sortedLRA = lraBlocks;
            std::sort(sortedLRA.begin(), sortedLRA.end());

            // Use 10th and 95th percentiles
            size_t lowIdx = sortedLRA.size() / 10;
            size_t highIdx = sortedLRA.size() * 95 / 100;
            float lra = sortedLRA[highIdx] - sortedLRA[lowIdx];
            loudnessRange.store(lra);

            // Dynamic range estimation
            dynamicRange.store(std::min(lra, 20.0f));
        }
    }
}
