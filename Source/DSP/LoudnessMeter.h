#pragma once

#include <JuceHeader.h>
#include "DSPUtils.h"
#include <vector>
#include <deque>
#include <atomic>

// ITU-R BS.1770-4 compliant loudness meter
class LoudnessMeter
{
public:
    LoudnessMeter();

    void prepare(double sampleRate, int samplesPerBlock);
    void process(const juce::AudioBuffer<float>& buffer);
    void reset();

    // LUFS measurements
    float getMomentaryLoudness() const { return momentaryLUFS.load(); }   // 400ms window
    float getShortTermLoudness() const { return shortTermLUFS.load(); }   // 3s window
    float getIntegratedLoudness() const { return integratedLUFS.load(); } // From start

    // Peak measurements
    float getPeakLevel() const { return peakLevel.load(); }
    float getTruePeakLevel() const { return truePeakLevel.load(); }

    // Dynamic range
    float getDynamicRange() const { return dynamicRange.load(); }

    // Stereo analysis
    float getStereoCorrelation() const { return stereoCorrelation.load(); }
    float getStereoBalance() const { return stereoBalance.load(); }

    // Loudness range (LRA)
    float getLoudnessRange() const { return loudnessRange.load(); }

    // Reset integrated measurement
    void resetIntegrated();

private:
    void applyKWeighting(const float* input, float* output, int numSamples, int channel);
    void updateMomentary();
    void updateShortTerm();
    void updateIntegrated();
    void calculateTruePeak(const float* input, int numSamples, int channel);
    void calculateCorrelation(const float* left, const float* right, int numSamples);

    double currentSampleRate = 44100.0;
    int currentBlockSize = 512;

    // K-weighting filter coefficients (two-stage shelving)
    // Stage 1: High shelf at ~1500Hz (+4dB)
    struct KWeightingState
    {
        // Stage 1 (high shelf)
        float s1_x1 = 0.0f, s1_x2 = 0.0f;
        float s1_y1 = 0.0f, s1_y2 = 0.0f;
        // Stage 2 (high-pass)
        float s2_x1 = 0.0f, s2_x2 = 0.0f;
        float s2_y1 = 0.0f, s2_y2 = 0.0f;
    };

    KWeightingState kWeightL, kWeightR;
    DSPUtils::BiquadCoeffs kShelfCoeffs;  // Pre-filter high shelf
    DSPUtils::BiquadCoeffs kHpfCoeffs;    // High-pass filter

    // True peak detection (4x oversampling)
    std::vector<float> oversampleBuffer;
    juce::dsp::Oversampling<float> oversampler { 2, 2, juce::dsp::Oversampling<float>::filterHalfBandPolyphaseIIR };

    // Momentary buffer (400ms)
    std::deque<float> momentaryBuffer;
    int momentarySamples = 0;

    // Short-term buffer (3s)
    std::deque<float> shortTermBuffer;
    int shortTermSamples = 0;

    // Integrated loudness (gated)
    std::vector<float> integratedBlocks;
    float integratedSum = 0.0f;
    int integratedBlockCount = 0;

    // Loudness range tracking
    std::vector<float> lraBlocks;

    // Metering values (atomic for thread safety)
    std::atomic<float> momentaryLUFS { -100.0f };
    std::atomic<float> shortTermLUFS { -100.0f };
    std::atomic<float> integratedLUFS { -100.0f };
    std::atomic<float> peakLevel { -100.0f };
    std::atomic<float> truePeakLevel { -100.0f };
    std::atomic<float> dynamicRange { 0.0f };
    std::atomic<float> stereoCorrelation { 1.0f };
    std::atomic<float> stereoBalance { 0.0f };
    std::atomic<float> loudnessRange { 0.0f };

    // Block processing for integrated/LRA (100ms blocks)
    static constexpr int BLOCK_DURATION_MS = 100;
    int samplesPerBlock100ms = 0;
    int currentBlockSamples = 0;
    float currentBlockSum = 0.0f;

    // Gating thresholds (EBU R128)
    static constexpr float ABSOLUTE_GATE = -70.0f;  // LUFS
    static constexpr float RELATIVE_GATE = -10.0f;  // dB below ungated

    // Sample buffer for K-weighted processing
    std::vector<float> kWeightedL, kWeightedR;
};
