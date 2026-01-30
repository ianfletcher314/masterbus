#include "MasteringCompressor.h"

MasteringCompressor::MasteringCompressor()
{
    updateCoefficients();
}

void MasteringCompressor::prepare(double sampleRate, int samplesPerBlock)
{
    currentSampleRate = sampleRate;
    currentBlockSize = samplesPerBlock;
    updateCoefficients();
    reset();
}

void MasteringCompressor::reset()
{
    envelopeL = 0.0f;
    envelopeR = 0.0f;
    gainReductionL = 0.0f;
    gainReductionR = 0.0f;
    autoReleaseEnvelope = 0.0f;
    saturationState = 0.0f;
    currentGainReduction.store(0.0f);

    // Reset sidechain HPF states
    scHpfStateL = {};
    scHpfStateR = {};
}

void MasteringCompressor::updateCoefficients()
{
    attackCoeff = DSPUtils::calculateCoefficient(currentSampleRate, attackMs);
    releaseCoeff = DSPUtils::calculateCoefficient(currentSampleRate, releaseMs);
    makeupLinear = DSPUtils::decibelsToLinear(makeupGain);

    // Update sidechain HPF
    scHpfCoeffs = DSPUtils::calculateHighPass(static_cast<float>(currentSampleRate), sidechainHPFFreq, 0.707f);
}

float MasteringCompressor::computeGain(float inputDb)
{
    float gainReductionDb = 0.0f;

    if (kneeDb > 0.0f)
    {
        // Soft knee compression
        float halfKnee = kneeDb / 2.0f;
        float kneeStart = threshold - halfKnee;
        float kneeEnd = threshold + halfKnee;

        if (inputDb < kneeStart)
        {
            // Below knee - no compression
            gainReductionDb = 0.0f;
        }
        else if (inputDb > kneeEnd)
        {
            // Above knee - full compression
            float overDb = inputDb - threshold;
            gainReductionDb = overDb * (1.0f - 1.0f / ratio);
        }
        else
        {
            // In knee - gradual transition
            float kneeRatio = (inputDb - kneeStart) / kneeDb;
            float currentRatio = 1.0f + (ratio - 1.0f) * kneeRatio * kneeRatio;
            float overDb = inputDb - threshold;
            gainReductionDb = overDb * (1.0f - 1.0f / currentRatio);
        }
    }
    else
    {
        // Hard knee compression
        if (inputDb > threshold)
        {
            float overDb = inputDb - threshold;
            gainReductionDb = overDb * (1.0f - 1.0f / ratio);
        }
    }

    return gainReductionDb;
}

float MasteringCompressor::computeAutoRelease(float inputLevel)
{
    // Program-dependent release time
    // Higher input levels = faster release, lower levels = slower release
    float minRelease = 50.0f;
    float maxRelease = 500.0f;

    // Smooth the input level for auto-release calculation
    float smoothCoeff = DSPUtils::calculateCoefficient(currentSampleRate, 100.0f);
    autoReleaseEnvelope += smoothCoeff * (inputLevel - autoReleaseEnvelope);

    // Map envelope to release time
    float releaseTime = maxRelease - (maxRelease - minRelease) * std::min(1.0f, autoReleaseEnvelope);
    return DSPUtils::calculateCoefficient(currentSampleRate, releaseTime);
}

void MasteringCompressor::applySaturation(float& sample, Mode mode)
{
    switch (mode)
    {
        case Mode::Clean:
            // No saturation
            break;

        case Mode::Glue:
            // Subtle harmonic warmth - soft saturation
            sample = std::tanh(sample * 1.1f) * 0.91f;
            break;

        case Mode::Punch:
            // Transient enhancement - asymmetric soft clip
            {
                float x = sample * 1.2f;
                if (x > 0)
                    sample = std::tanh(x) * 0.95f;
                else
                    sample = std::tanh(x * 0.8f) * 1.05f;
            }
            break;

        case Mode::Vintage:
            // Tube-style saturation with slight frequency-dependent behavior
            {
                float drive = 1.3f;
                float x = sample * drive;
                // Asymmetric soft saturation
                if (x > 0)
                    sample = x / (1.0f + std::abs(x * 0.5f));
                else
                    sample = x / (1.0f + std::abs(x * 0.7f));

                // Add subtle second harmonic
                saturationState = saturationState * 0.99f + sample * 0.01f;
                sample = sample + saturationState * 0.02f;
            }
            break;
    }
}

void MasteringCompressor::process(juce::AudioBuffer<float>& buffer)
{
    if (bypassed) return;

    const int numChannels = buffer.getNumChannels();
    const int numSamples = buffer.getNumSamples();

    if (numChannels < 1) return;

    // Calculate input level for metering
    float inLevel = 0.0f;
    for (int ch = 0; ch < numChannels; ++ch)
        inLevel = std::max(inLevel, buffer.getMagnitude(ch, 0, numSamples));
    inputLevel.store(inLevel);

    float* leftData = buffer.getWritePointer(0);
    float* rightData = numChannels > 1 ? buffer.getWritePointer(1) : nullptr;

    float maxGR = 0.0f;

    for (int i = 0; i < numSamples; ++i)
    {
        float inputL = leftData[i];
        float inputR = rightData ? rightData[i] : inputL;
        float dryL = inputL;
        float dryR = inputR;

        // M/S encoding if enabled
        if (midSideMode && rightData)
        {
            float mid = (inputL + inputR) * 0.5f;
            float side = (inputL - inputR) * 0.5f;
            inputL = mid;
            inputR = side;
        }

        // Apply sidechain HPF
        float scL = scHpfCoeffs.b0 * inputL + scHpfCoeffs.b1 * scHpfStateL.x1 + scHpfCoeffs.b2 * scHpfStateL.x2
                    - scHpfCoeffs.a1 * scHpfStateL.y1 - scHpfCoeffs.a2 * scHpfStateL.y2;
        scHpfStateL.x2 = scHpfStateL.x1;
        scHpfStateL.x1 = inputL;
        scHpfStateL.y2 = scHpfStateL.y1;
        scHpfStateL.y1 = scL;

        float scR = inputR;
        if (rightData)
        {
            scR = scHpfCoeffs.b0 * inputR + scHpfCoeffs.b1 * scHpfStateR.x1 + scHpfCoeffs.b2 * scHpfStateR.x2
                  - scHpfCoeffs.a1 * scHpfStateR.y1 - scHpfCoeffs.a2 * scHpfStateR.y2;
            scHpfStateR.x2 = scHpfStateR.x1;
            scHpfStateR.x1 = inputR;
            scHpfStateR.y2 = scHpfStateR.y1;
            scHpfStateR.y1 = scR;
        }

        // If sidechain listen is enabled, output sidechain signal
        if (sidechainListen)
        {
            leftData[i] = scL;
            if (rightData) rightData[i] = scR;
            continue;
        }

        // Get input levels (absolute for envelope)
        float levelL = std::abs(scL);
        float levelR = std::abs(scR);

        // Stereo linking
        float linkedLevel;
        if (stereoLink >= 1.0f)
        {
            linkedLevel = std::max(levelL, levelR);
        }
        else if (stereoLink <= 0.0f)
        {
            // Independent processing handled below
            linkedLevel = levelL; // Won't be used
        }
        else
        {
            float maxLevel = std::max(levelL, levelR);
            linkedLevel = levelL + stereoLink * (maxLevel - levelL);
        }

        // Get release coefficient (auto or fixed)
        float releaseCoeffToUse = autoRelease ? computeAutoRelease(linkedLevel) : releaseCoeff;

        // Envelope follower for left/linked
        if (stereoLink > 0.0f)
        {
            if (linkedLevel > envelopeL)
                envelopeL += attackCoeff * (linkedLevel - envelopeL);
            else
                envelopeL += releaseCoeffToUse * (linkedLevel - envelopeL);

            // Calculate gain reduction
            float inputDb = DSPUtils::linearToDecibels(envelopeL);
            gainReductionL = computeGain(inputDb);
            gainReductionR = gainReductionL;
        }
        else
        {
            // Independent L/R processing
            if (levelL > envelopeL)
                envelopeL += attackCoeff * (levelL - envelopeL);
            else
                envelopeL += releaseCoeffToUse * (levelL - envelopeL);

            if (levelR > envelopeR)
                envelopeR += attackCoeff * (levelR - envelopeR);
            else
                envelopeR += releaseCoeffToUse * (levelR - envelopeR);

            float inputDbL = DSPUtils::linearToDecibels(envelopeL);
            float inputDbR = DSPUtils::linearToDecibels(envelopeR);
            gainReductionL = computeGain(inputDbL);
            gainReductionR = computeGain(inputDbR);
        }

        // Track max gain reduction for metering
        maxGR = std::max(maxGR, std::max(gainReductionL, gainReductionR));

        // Apply gain reduction
        float gainL = DSPUtils::decibelsToLinear(-gainReductionL);
        float gainR = DSPUtils::decibelsToLinear(-gainReductionR);

        float outputL = inputL * gainL * makeupLinear;
        float outputR = inputR * gainR * makeupLinear;

        // Apply saturation based on mode
        if (currentMode != Mode::Clean)
        {
            applySaturation(outputL, currentMode);
            applySaturation(outputR, currentMode);
        }

        // M/S decoding if enabled
        if (midSideMode && rightData)
        {
            float mid = outputL;
            float side = outputR;
            outputL = mid + side;
            outputR = mid - side;
        }

        // Wet/dry mix (parallel compression)
        leftData[i] = dryL * (1.0f - mix) + outputL * mix;
        if (rightData)
            rightData[i] = dryR * (1.0f - mix) + outputR * mix;
    }

    currentGainReduction.store(maxGR);

    // Calculate output level for metering
    float outLevel = 0.0f;
    for (int ch = 0; ch < numChannels; ++ch)
        outLevel = std::max(outLevel, buffer.getMagnitude(ch, 0, numSamples));
    outputLevel.store(outLevel);
}

// Parameter setters
void MasteringCompressor::setThreshold(float thresholdDb)
{
    threshold = std::clamp(thresholdDb, -40.0f, 0.0f);
}

void MasteringCompressor::setRatio(float newRatio)
{
    ratio = std::clamp(newRatio, 1.0f, 10.0f);
}

void MasteringCompressor::setAttack(float newAttackMs)
{
    attackMs = std::clamp(newAttackMs, 0.1f, 100.0f);
    updateCoefficients();
}

void MasteringCompressor::setRelease(float newReleaseMs)
{
    releaseMs = std::clamp(newReleaseMs, 50.0f, 2000.0f);
    updateCoefficients();
}

void MasteringCompressor::setKnee(float newKneeDb)
{
    kneeDb = std::clamp(newKneeDb, 0.0f, 20.0f);
}

void MasteringCompressor::setMakeupGain(float gainDb)
{
    makeupGain = std::clamp(gainDb, 0.0f, 12.0f);
    makeupLinear = DSPUtils::decibelsToLinear(makeupGain);
}

void MasteringCompressor::setMix(float mixPercent)
{
    mix = std::clamp(mixPercent / 100.0f, 0.0f, 1.0f);
}

void MasteringCompressor::setAutoRelease(bool enabled)
{
    autoRelease = enabled;
}

void MasteringCompressor::setMode(Mode mode)
{
    currentMode = mode;
}

void MasteringCompressor::setSidechainHPF(float freq)
{
    sidechainHPFFreq = std::clamp(freq, 20.0f, 300.0f);
    scHpfCoeffs = DSPUtils::calculateHighPass(static_cast<float>(currentSampleRate), sidechainHPFFreq, 0.707f);
}

void MasteringCompressor::setSidechainListen(bool enabled)
{
    sidechainListen = enabled;
}

void MasteringCompressor::setStereoLink(float linkPercent)
{
    stereoLink = std::clamp(linkPercent / 100.0f, 0.0f, 1.0f);
}

void MasteringCompressor::setMidSideMode(bool enabled)
{
    midSideMode = enabled;
}

void MasteringCompressor::setBypass(bool shouldBypass)
{
    bypassed = shouldBypass;
}
