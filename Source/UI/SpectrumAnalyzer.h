#pragma once

#include <JuceHeader.h>
#include "LookAndFeel.h"
#include <array>
#include <atomic>

class SpectrumAnalyzer : public juce::Component, public juce::Timer
{
public:
    static constexpr int FFT_ORDER = 11;
    static constexpr int FFT_SIZE = 1 << FFT_ORDER; // 2048
    static constexpr int NUM_BINS = FFT_SIZE / 2;

    SpectrumAnalyzer();
    ~SpectrumAnalyzer() override;

    void paint(juce::Graphics& g) override;
    void resized() override;
    void timerCallback() override;

    // Push audio data for analysis
    void pushBuffer(const juce::AudioBuffer<float>& buffer);

    // Pre/Post toggle
    void setShowPre(bool showPre);
    void setShowPost(bool showPost);
    void pushPreBuffer(const juce::AudioBuffer<float>& buffer);
    void pushPostBuffer(const juce::AudioBuffer<float>& buffer);

    // Settings
    void setFFTSize(int size);     // 1024, 2048, 4096, 8192, 16384
    void setSlope(float dbPerOctave); // 0, 3, 4.5 dB/octave
    void setSmoothing(float smoothing); // 0-1
    void setPeakHold(bool enable);

    // Sample rate (needed for frequency calculation)
    void setSampleRate(double rate) { sampleRate = rate; }

private:
    void processFFT(const std::vector<float>& buffer, std::array<float, NUM_BINS>& magnitudes);
    void drawSpectrum(juce::Graphics& g, const std::array<float, NUM_BINS>& magnitudes,
                      juce::Colour colour, float alpha);
    void drawGrid(juce::Graphics& g);
    float getFrequencyForBin(int bin) const;
    float getXForFrequency(float freq) const;
    float getYForDecibels(float db) const;

    juce::dsp::FFT fft { FFT_ORDER };
    juce::dsp::WindowingFunction<float> window { FFT_SIZE, juce::dsp::WindowingFunction<float>::hann };

    // Sample buffers (circular)
    std::vector<float> sampleBuffer;
    std::atomic<int> writeIndex { 0 };

    // FFT working buffers
    std::vector<float> fftData;
    std::array<float, NUM_BINS> magnitudes;
    std::array<float, NUM_BINS> smoothedMagnitudes;
    std::array<float, NUM_BINS> peakMagnitudes;

    // Pre/Post comparison
    bool showPre = false;
    bool showPost = true;
    std::vector<float> preBuffer;
    std::vector<float> postBuffer;
    std::array<float, NUM_BINS> preMagnitudes;
    std::array<float, NUM_BINS> postMagnitudes;
    std::array<float, NUM_BINS> smoothedPreMagnitudes;
    std::array<float, NUM_BINS> smoothedPostMagnitudes;
    std::atomic<int> preWriteIndex { 0 };
    std::atomic<int> postWriteIndex { 0 };

    double sampleRate = 44100.0;
    float smoothingFactor = 0.7f;
    float slopeDbPerOctave = 3.0f;
    bool peakHoldEnabled = true;

    // Display range
    float minDb = -90.0f;
    float maxDb = 6.0f;
    float minFreq = 20.0f;
    float maxFreq = 20000.0f;

    // Peak decay
    float peakDecayRate = 0.9995f;

    JUCE_DECLARE_NON_COPYABLE_WITH_LEAK_DETECTOR(SpectrumAnalyzer)
};
