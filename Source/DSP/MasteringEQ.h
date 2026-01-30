#pragma once

#include <JuceHeader.h>
#include "DSPUtils.h"
#include <array>

// Single biquad filter section
class BiquadFilter
{
public:
    void setCoefficients(const DSPUtils::BiquadCoeffs& coeffs);
    void reset();
    float processSample(float input);

private:
    DSPUtils::BiquadCoeffs coeffs;
    float x1 = 0.0f, x2 = 0.0f;
    float y1 = 0.0f, y2 = 0.0f;
};

// Multi-stage filter for higher order filters (6/12/18/24 dB)
class MultiStageFilter
{
public:
    enum class Type { HighPass, LowPass };

    void prepare(double sampleRate);
    void setParameters(Type type, float freq, int order);
    void reset();
    float processSample(float input);
    bool isEnabled() const { return enabled; }
    void setEnabled(bool shouldEnable) { enabled = shouldEnable; }

private:
    void updateCoefficients();

    double currentSampleRate = 44100.0;
    Type filterType = Type::HighPass;
    float frequency = 20.0f;
    int filterOrder = 2; // 1=6dB, 2=12dB, 3=18dB, 4=24dB
    bool enabled = false;

    std::array<BiquadFilter, 4> stages;
};

// Parametric EQ band
class ParametricBand
{
public:
    void prepare(double sampleRate);
    void setParameters(float freq, float gain, float q);
    void reset();
    float processSample(float input);
    bool isEnabled() const { return enabled; }
    void setEnabled(bool shouldEnable) { enabled = shouldEnable; }

    float getFrequency() const { return frequency; }
    float getGain() const { return gainDb; }
    float getQ() const { return qFactor; }

private:
    void updateCoefficients();

    double currentSampleRate = 44100.0;
    float frequency = 1000.0f;
    float gainDb = 0.0f;
    float qFactor = 1.0f;
    bool enabled = true;

    BiquadFilter filter;
};

// Shelf EQ band
class ShelfBand
{
public:
    enum class Type { Low, High };

    void prepare(double sampleRate);
    void setParameters(Type type, float freq, float gain);
    void reset();
    float processSample(float input);
    bool isEnabled() const { return enabled; }
    void setEnabled(bool shouldEnable) { enabled = shouldEnable; }

    float getFrequency() const { return frequency; }
    float getGain() const { return gainDb; }

private:
    void updateCoefficients();

    double currentSampleRate = 44100.0;
    Type shelfType = Type::Low;
    float frequency = 100.0f;
    float gainDb = 0.0f;
    bool enabled = true;

    BiquadFilter filter;
};

// Complete mastering EQ processor
class MasteringEQ
{
public:
    static constexpr int NUM_PARAMETRIC_BANDS = 4;

    MasteringEQ();

    void prepare(double sampleRate, int samplesPerBlock);
    void process(juce::AudioBuffer<float>& buffer);
    void reset();

    // HPF: 10Hz-300Hz, 6/12/18/24dB slopes
    void setHighPassFrequency(float freq);
    void setHighPassSlope(int slopeDb); // 6, 12, 18, or 24
    void setHighPassEnabled(bool enabled);

    // LPF: 5kHz-22kHz, 6/12/18/24dB slopes
    void setLowPassFrequency(float freq);
    void setLowPassSlope(int slopeDb);
    void setLowPassEnabled(bool enabled);

    // Low shelf: 20Hz-500Hz
    void setLowShelfFrequency(float freq);
    void setLowShelfGain(float gainDb);
    void setLowShelfEnabled(bool enabled);

    // High shelf: 2kHz-20kHz
    void setHighShelfFrequency(float freq);
    void setHighShelfGain(float gainDb);
    void setHighShelfEnabled(bool enabled);

    // Parametric bands
    void setBandFrequency(int band, float freq);
    void setBandGain(int band, float gainDb);
    void setBandQ(int band, float q);
    void setBandEnabled(int band, bool enabled);

    // Global controls
    void setLinearPhase(bool useLinearPhase);
    void setMidSideMode(bool useMidSide);
    void setBypass(bool shouldBypass);
    void setOutputGain(float gainDb);

    bool isLinearPhase() const { return linearPhaseMode; }
    bool isMidSideMode() const { return midSideMode; }
    bool isBypassed() const { return bypassed; }

    // For UI spectrum display
    std::array<float, 512> getMagnitudeResponse(float sampleRate) const;

private:
    void processMinimumPhase(juce::AudioBuffer<float>& buffer);
    void processLinearPhase(juce::AudioBuffer<float>& buffer);
    void processMidSide(juce::AudioBuffer<float>& buffer);
    void encodeToMidSide(float& left, float& right);
    void decodeFromMidSide(float& mid, float& side);
    float processSampleThroughEQ(float input, int channel);

    double currentSampleRate = 44100.0;
    int currentBlockSize = 512;
    bool linearPhaseMode = false;
    bool midSideMode = false;
    bool bypassed = false;
    float outputGainLinear = 1.0f;

    // Per-channel filters (L/R or M/S)
    struct ChannelEQ
    {
        MultiStageFilter highPass;
        MultiStageFilter lowPass;
        ShelfBand lowShelf;
        ShelfBand highShelf;
        std::array<ParametricBand, NUM_PARAMETRIC_BANDS> parametric;
    };

    std::array<ChannelEQ, 2> channels;

    // Linear phase processing
    juce::dsp::FFT fft { 12 }; // 4096 samples
    std::vector<float> fftBuffer;
    std::vector<float> irBuffer;
    std::vector<float> overlapBuffer;
    bool linearPhaseReady = false;
    void prepareLinearPhase();
    void updateLinearPhaseIR();
};
