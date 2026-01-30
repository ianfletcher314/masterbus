#pragma once

#include <JuceHeader.h>
#include "DSPUtils.h"
#include <array>

class MasteringCompressor
{
public:
    enum class Mode
    {
        Clean,      // Transparent, minimal coloration
        Glue,       // Subtle harmonic warmth
        Punch,      // Enhanced transients
        Vintage     // Modeled on classic hardware
    };

    MasteringCompressor();

    void prepare(double sampleRate, int samplesPerBlock);
    void process(juce::AudioBuffer<float>& buffer);
    void reset();

    // Main compressor controls
    void setThreshold(float thresholdDb);       // -40dB to 0dB
    void setRatio(float ratio);                 // 1:1 to 10:1
    void setAttack(float attackMs);             // 0.1ms to 100ms
    void setRelease(float releaseMs);           // 50ms to 2000ms
    void setKnee(float kneeDb);                 // 0 to 20dB (soft knee)
    void setMakeupGain(float gainDb);           // 0dB to 12dB
    void setMix(float mixPercent);              // 0-100% (parallel compression)
    void setAutoRelease(bool enabled);
    void setMode(Mode mode);

    // Sidechain controls
    void setSidechainHPF(float freq);           // 20Hz to 300Hz
    void setSidechainListen(bool enabled);

    // Stereo controls
    void setStereoLink(float linkPercent);      // 0-100%
    void setMidSideMode(bool enabled);

    // Global
    void setBypass(bool shouldBypass);

    // Metering
    float getGainReduction() const { return currentGainReduction.load(); }
    float getInputLevel() const { return inputLevel.load(); }
    float getOutputLevel() const { return outputLevel.load(); }

    // Getters for UI
    float getThreshold() const { return threshold; }
    float getRatio() const { return ratio; }
    float getAttack() const { return attackMs; }
    float getRelease() const { return releaseMs; }
    float getKnee() const { return kneeDb; }
    float getMakeupGain() const { return makeupGain; }
    Mode getMode() const { return currentMode; }

private:
    float computeGain(float inputDb);
    float processSampleStereoLinked(float inputL, float inputR, float& outputL, float& outputR);
    float computeAutoRelease(float inputLevel);
    void updateCoefficients();
    void applySaturation(float& sample, Mode mode);

    // Parameters
    float threshold = -20.0f;
    float ratio = 4.0f;
    float attackMs = 10.0f;
    float releaseMs = 100.0f;
    float kneeDb = 0.0f;
    float makeupGain = 0.0f;
    float mix = 1.0f;
    float stereoLink = 1.0f;
    float sidechainHPFFreq = 60.0f;
    bool autoRelease = false;
    bool sidechainListen = false;
    bool midSideMode = false;
    bool bypassed = false;
    Mode currentMode = Mode::Clean;

    // Coefficients
    float attackCoeff = 0.0f;
    float releaseCoeff = 0.0f;
    float makeupLinear = 1.0f;

    // State
    double currentSampleRate = 44100.0;
    int currentBlockSize = 512;

    // Envelope followers (L/R or M/S)
    float envelopeL = 0.0f;
    float envelopeR = 0.0f;
    float gainReductionL = 0.0f;
    float gainReductionR = 0.0f;

    // Sidechain HPF
    struct BiquadState
    {
        float x1 = 0.0f, x2 = 0.0f;
        float y1 = 0.0f, y2 = 0.0f;
    };
    BiquadState scHpfStateL, scHpfStateR;
    DSPUtils::BiquadCoeffs scHpfCoeffs;

    // Auto-release state
    float autoReleaseEnvelope = 0.0f;

    // Metering (atomic for thread safety)
    std::atomic<float> currentGainReduction { 0.0f };
    std::atomic<float> inputLevel { 0.0f };
    std::atomic<float> outputLevel { 0.0f };

    // Saturation state for vintage mode
    float saturationState = 0.0f;
};
