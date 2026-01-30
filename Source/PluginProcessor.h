#pragma once

#include <JuceHeader.h>
#include "DSP/MasteringEQ.h"
#include "DSP/MasteringCompressor.h"
#include "DSP/LoudnessMeter.h"

class MasterBusAudioProcessor : public juce::AudioProcessor
{
public:
    MasterBusAudioProcessor();
    ~MasterBusAudioProcessor() override;

    void prepareToPlay(double sampleRate, int samplesPerBlock) override;
    void releaseResources() override;

    bool isBusesLayoutSupported(const BusesLayout& layouts) const override;

    void processBlock(juce::AudioBuffer<float>&, juce::MidiBuffer&) override;
    using AudioProcessor::processBlock;

    juce::AudioProcessorEditor* createEditor() override;
    bool hasEditor() const override;

    const juce::String getName() const override;

    bool acceptsMidi() const override;
    bool producesMidi() const override;
    bool isMidiEffect() const override;
    double getTailLengthSeconds() const override;

    int getNumPrograms() override;
    int getCurrentProgram() override;
    void setCurrentProgram(int index) override;
    const juce::String getProgramName(int index) override;
    void changeProgramName(int index, const juce::String& newName) override;

    void getStateInformation(juce::MemoryBlock& destData) override;
    void setStateInformation(const void* data, int sizeInBytes) override;

    juce::AudioProcessorValueTreeState& getAPVTS() { return apvts; }

    // DSP access for editor
    MasteringEQ& getEQ() { return eq; }
    MasteringCompressor& getCompressor() { return compressor; }
    LoudnessMeter& getLoudnessMeter() { return loudnessMeter; }

    // Metering access
    float getInputLevel() const { return inputLevel.load(); }
    float getOutputLevel() const { return outputLevel.load(); }
    float getGainReduction() const { return compressor.getGainReduction(); }

    // For spectrum analyzer
    const juce::AudioBuffer<float>& getPreEQBuffer() const { return preEQBuffer; }
    const juce::AudioBuffer<float>& getPostProcessBuffer() const { return postProcessBuffer; }

    // A/B/C/D comparison
    void storeSettings(int slot);
    void recallSettings(int slot);
    int getCurrentSlot() const { return currentSettingsSlot; }

private:
    juce::AudioProcessorValueTreeState apvts;
    juce::AudioProcessorValueTreeState::ParameterLayout createParameterLayout();

    // DSP
    MasteringEQ eq;
    MasteringCompressor compressor;
    LoudnessMeter loudnessMeter;

    // Parameter pointers
    // EQ HPF
    std::atomic<float>* hpfFreq = nullptr;
    std::atomic<float>* hpfSlope = nullptr;
    std::atomic<float>* hpfEnabled = nullptr;

    // EQ LPF
    std::atomic<float>* lpfFreq = nullptr;
    std::atomic<float>* lpfSlope = nullptr;
    std::atomic<float>* lpfEnabled = nullptr;

    // EQ Low Shelf
    std::atomic<float>* lsFreq = nullptr;
    std::atomic<float>* lsGain = nullptr;
    std::atomic<float>* lsEnabled = nullptr;

    // EQ High Shelf
    std::atomic<float>* hsFreq = nullptr;
    std::atomic<float>* hsGain = nullptr;
    std::atomic<float>* hsEnabled = nullptr;

    // EQ Parametric Bands
    std::array<std::atomic<float>*, 4> bandFreq;
    std::array<std::atomic<float>*, 4> bandGain;
    std::array<std::atomic<float>*, 4> bandQ;
    std::array<std::atomic<float>*, 4> bandEnabled;

    // EQ Global
    std::atomic<float>* eqLinearPhase = nullptr;
    std::atomic<float>* eqMidSide = nullptr;
    std::atomic<float>* eqBypass = nullptr;

    // Compressor
    std::atomic<float>* compThreshold = nullptr;
    std::atomic<float>* compRatio = nullptr;
    std::atomic<float>* compAttack = nullptr;
    std::atomic<float>* compRelease = nullptr;
    std::atomic<float>* compKnee = nullptr;
    std::atomic<float>* compMakeup = nullptr;
    std::atomic<float>* compMix = nullptr;
    std::atomic<float>* compAutoRelease = nullptr;
    std::atomic<float>* compMode = nullptr;
    std::atomic<float>* compScHpf = nullptr;
    std::atomic<float>* compScListen = nullptr;
    std::atomic<float>* compStereoLink = nullptr;
    std::atomic<float>* compMidSide = nullptr;
    std::atomic<float>* compBypass = nullptr;

    // Global
    std::atomic<float>* outputGain = nullptr;
    std::atomic<float>* globalBypass = nullptr;

    // Metering
    std::atomic<float> inputLevel { 0.0f };
    std::atomic<float> outputLevel { 0.0f };

    // Buffers for spectrum analyzer
    juce::AudioBuffer<float> preEQBuffer;
    juce::AudioBuffer<float> postProcessBuffer;

    // A/B/C/D settings storage
    struct SettingsSlot
    {
        std::map<juce::String, float> parameterValues;
        bool isUsed = false;
    };
    std::array<SettingsSlot, 4> settingsSlots;
    int currentSettingsSlot = 0;

    JUCE_DECLARE_NON_COPYABLE_WITH_LEAK_DETECTOR(MasterBusAudioProcessor)
};
