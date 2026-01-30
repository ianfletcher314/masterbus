#include "PluginProcessor.h"
#include "PluginEditor.h"

MasterBusAudioProcessor::MasterBusAudioProcessor()
     : AudioProcessor(BusesProperties()
                      .withInput("Input", juce::AudioChannelSet::stereo(), true)
                      .withOutput("Output", juce::AudioChannelSet::stereo(), true)),
       apvts(*this, nullptr, "Parameters", createParameterLayout())
{
    // Get parameter pointers
    // HPF
    hpfFreq = apvts.getRawParameterValue("hpfFreq");
    hpfSlope = apvts.getRawParameterValue("hpfSlope");
    hpfEnabled = apvts.getRawParameterValue("hpfEnabled");

    // LPF
    lpfFreq = apvts.getRawParameterValue("lpfFreq");
    lpfSlope = apvts.getRawParameterValue("lpfSlope");
    lpfEnabled = apvts.getRawParameterValue("lpfEnabled");

    // Low Shelf
    lsFreq = apvts.getRawParameterValue("lsFreq");
    lsGain = apvts.getRawParameterValue("lsGain");
    lsEnabled = apvts.getRawParameterValue("lsEnabled");

    // High Shelf
    hsFreq = apvts.getRawParameterValue("hsFreq");
    hsGain = apvts.getRawParameterValue("hsGain");
    hsEnabled = apvts.getRawParameterValue("hsEnabled");

    // Parametric bands
    for (int i = 0; i < 4; ++i)
    {
        bandFreq[i] = apvts.getRawParameterValue("band" + juce::String(i + 1) + "Freq");
        bandGain[i] = apvts.getRawParameterValue("band" + juce::String(i + 1) + "Gain");
        bandQ[i] = apvts.getRawParameterValue("band" + juce::String(i + 1) + "Q");
        bandEnabled[i] = apvts.getRawParameterValue("band" + juce::String(i + 1) + "Enabled");
    }

    // EQ Global
    eqLinearPhase = apvts.getRawParameterValue("eqLinearPhase");
    eqMidSide = apvts.getRawParameterValue("eqMidSide");
    eqBypass = apvts.getRawParameterValue("eqBypass");

    // Compressor
    compThreshold = apvts.getRawParameterValue("compThreshold");
    compRatio = apvts.getRawParameterValue("compRatio");
    compAttack = apvts.getRawParameterValue("compAttack");
    compRelease = apvts.getRawParameterValue("compRelease");
    compKnee = apvts.getRawParameterValue("compKnee");
    compMakeup = apvts.getRawParameterValue("compMakeup");
    compMix = apvts.getRawParameterValue("compMix");
    compAutoRelease = apvts.getRawParameterValue("compAutoRelease");
    compMode = apvts.getRawParameterValue("compMode");
    compScHpf = apvts.getRawParameterValue("compScHpf");
    compScListen = apvts.getRawParameterValue("compScListen");
    compStereoLink = apvts.getRawParameterValue("compStereoLink");
    compMidSide = apvts.getRawParameterValue("compMidSide");
    compBypass = apvts.getRawParameterValue("compBypass");

    // Global
    outputGain = apvts.getRawParameterValue("outputGain");
    globalBypass = apvts.getRawParameterValue("globalBypass");
}

MasterBusAudioProcessor::~MasterBusAudioProcessor() {}

juce::AudioProcessorValueTreeState::ParameterLayout MasterBusAudioProcessor::createParameterLayout()
{
    std::vector<std::unique_ptr<juce::RangedAudioParameter>> params;

    // === HPF ===
    params.push_back(std::make_unique<juce::AudioParameterFloat>(
        juce::ParameterID("hpfFreq", 1), "HPF Freq",
        juce::NormalisableRange<float>(10.0f, 300.0f, 1.0f, 0.4f), 20.0f,
        juce::AudioParameterFloatAttributes().withLabel("Hz")));
    params.push_back(std::make_unique<juce::AudioParameterChoice>(
        juce::ParameterID("hpfSlope", 1), "HPF Slope",
        juce::StringArray{ "6dB", "12dB", "18dB", "24dB" }, 1));
    params.push_back(std::make_unique<juce::AudioParameterBool>(
        juce::ParameterID("hpfEnabled", 1), "HPF Enabled", false));

    // === LPF ===
    params.push_back(std::make_unique<juce::AudioParameterFloat>(
        juce::ParameterID("lpfFreq", 1), "LPF Freq",
        juce::NormalisableRange<float>(5000.0f, 22000.0f, 1.0f, 0.4f), 20000.0f,
        juce::AudioParameterFloatAttributes().withLabel("Hz")));
    params.push_back(std::make_unique<juce::AudioParameterChoice>(
        juce::ParameterID("lpfSlope", 1), "LPF Slope",
        juce::StringArray{ "6dB", "12dB", "18dB", "24dB" }, 1));
    params.push_back(std::make_unique<juce::AudioParameterBool>(
        juce::ParameterID("lpfEnabled", 1), "LPF Enabled", false));

    // === Low Shelf ===
    params.push_back(std::make_unique<juce::AudioParameterFloat>(
        juce::ParameterID("lsFreq", 1), "Low Shelf Freq",
        juce::NormalisableRange<float>(20.0f, 500.0f, 1.0f, 0.4f), 100.0f,
        juce::AudioParameterFloatAttributes().withLabel("Hz")));
    params.push_back(std::make_unique<juce::AudioParameterFloat>(
        juce::ParameterID("lsGain", 1), "Low Shelf Gain",
        juce::NormalisableRange<float>(-12.0f, 12.0f, 0.1f), 0.0f,
        juce::AudioParameterFloatAttributes().withLabel("dB")));
    params.push_back(std::make_unique<juce::AudioParameterBool>(
        juce::ParameterID("lsEnabled", 1), "Low Shelf Enabled", true));

    // === High Shelf ===
    params.push_back(std::make_unique<juce::AudioParameterFloat>(
        juce::ParameterID("hsFreq", 1), "High Shelf Freq",
        juce::NormalisableRange<float>(2000.0f, 20000.0f, 1.0f, 0.4f), 8000.0f,
        juce::AudioParameterFloatAttributes().withLabel("Hz")));
    params.push_back(std::make_unique<juce::AudioParameterFloat>(
        juce::ParameterID("hsGain", 1), "High Shelf Gain",
        juce::NormalisableRange<float>(-12.0f, 12.0f, 0.1f), 0.0f,
        juce::AudioParameterFloatAttributes().withLabel("dB")));
    params.push_back(std::make_unique<juce::AudioParameterBool>(
        juce::ParameterID("hsEnabled", 1), "High Shelf Enabled", true));

    // === Parametric Bands ===
    struct BandDefaults { float freq; const char* name; };
    BandDefaults bandDefaults[4] = {
        { 80.0f, "Low" },
        { 300.0f, "Low-Mid" },
        { 1000.0f, "Mid" },
        { 4000.0f, "High-Mid" }
    };

    for (int i = 0; i < 4; ++i)
    {
        juce::String prefix = "band" + juce::String(i + 1);
        juce::String bandName = bandDefaults[i].name;

        params.push_back(std::make_unique<juce::AudioParameterFloat>(
            juce::ParameterID(prefix + "Freq", 1), bandName + " Freq",
            juce::NormalisableRange<float>(20.0f, 20000.0f, 1.0f, 0.3f), bandDefaults[i].freq,
            juce::AudioParameterFloatAttributes().withLabel("Hz")));
        params.push_back(std::make_unique<juce::AudioParameterFloat>(
            juce::ParameterID(prefix + "Gain", 1), bandName + " Gain",
            juce::NormalisableRange<float>(-18.0f, 18.0f, 0.1f), 0.0f,
            juce::AudioParameterFloatAttributes().withLabel("dB")));
        params.push_back(std::make_unique<juce::AudioParameterFloat>(
            juce::ParameterID(prefix + "Q", 1), bandName + " Q",
            juce::NormalisableRange<float>(0.1f, 10.0f, 0.01f, 0.5f), 1.0f));
        params.push_back(std::make_unique<juce::AudioParameterBool>(
            juce::ParameterID(prefix + "Enabled", 1), bandName + " Enabled", true));
    }

    // === EQ Global ===
    params.push_back(std::make_unique<juce::AudioParameterBool>(
        juce::ParameterID("eqLinearPhase", 1), "EQ Linear Phase", false));
    params.push_back(std::make_unique<juce::AudioParameterBool>(
        juce::ParameterID("eqMidSide", 1), "EQ Mid/Side", false));
    params.push_back(std::make_unique<juce::AudioParameterBool>(
        juce::ParameterID("eqBypass", 1), "EQ Bypass", false));

    // === Compressor ===
    params.push_back(std::make_unique<juce::AudioParameterFloat>(
        juce::ParameterID("compThreshold", 1), "Comp Threshold",
        juce::NormalisableRange<float>(-40.0f, 0.0f, 0.1f), -20.0f,
        juce::AudioParameterFloatAttributes().withLabel("dB")));
    params.push_back(std::make_unique<juce::AudioParameterFloat>(
        juce::ParameterID("compRatio", 1), "Comp Ratio",
        juce::NormalisableRange<float>(1.0f, 10.0f, 0.1f, 0.5f), 2.0f,
        juce::AudioParameterFloatAttributes().withLabel(":1")));
    params.push_back(std::make_unique<juce::AudioParameterFloat>(
        juce::ParameterID("compAttack", 1), "Comp Attack",
        juce::NormalisableRange<float>(0.1f, 100.0f, 0.1f, 0.4f), 10.0f,
        juce::AudioParameterFloatAttributes().withLabel("ms")));
    params.push_back(std::make_unique<juce::AudioParameterFloat>(
        juce::ParameterID("compRelease", 1), "Comp Release",
        juce::NormalisableRange<float>(50.0f, 2000.0f, 1.0f, 0.4f), 200.0f,
        juce::AudioParameterFloatAttributes().withLabel("ms")));
    params.push_back(std::make_unique<juce::AudioParameterFloat>(
        juce::ParameterID("compKnee", 1), "Comp Knee",
        juce::NormalisableRange<float>(0.0f, 20.0f, 0.1f), 6.0f,
        juce::AudioParameterFloatAttributes().withLabel("dB")));
    params.push_back(std::make_unique<juce::AudioParameterFloat>(
        juce::ParameterID("compMakeup", 1), "Comp Makeup",
        juce::NormalisableRange<float>(0.0f, 12.0f, 0.1f), 0.0f,
        juce::AudioParameterFloatAttributes().withLabel("dB")));
    params.push_back(std::make_unique<juce::AudioParameterFloat>(
        juce::ParameterID("compMix", 1), "Comp Mix",
        juce::NormalisableRange<float>(0.0f, 100.0f, 1.0f), 100.0f,
        juce::AudioParameterFloatAttributes().withLabel("%")));
    params.push_back(std::make_unique<juce::AudioParameterBool>(
        juce::ParameterID("compAutoRelease", 1), "Comp Auto Release", false));
    params.push_back(std::make_unique<juce::AudioParameterChoice>(
        juce::ParameterID("compMode", 1), "Comp Mode",
        juce::StringArray{ "Clean", "Glue", "Punch", "Vintage" }, 0));
    params.push_back(std::make_unique<juce::AudioParameterFloat>(
        juce::ParameterID("compScHpf", 1), "Comp SC HPF",
        juce::NormalisableRange<float>(20.0f, 300.0f, 1.0f, 0.4f), 60.0f,
        juce::AudioParameterFloatAttributes().withLabel("Hz")));
    params.push_back(std::make_unique<juce::AudioParameterBool>(
        juce::ParameterID("compScListen", 1), "Comp SC Listen", false));
    params.push_back(std::make_unique<juce::AudioParameterFloat>(
        juce::ParameterID("compStereoLink", 1), "Comp Stereo Link",
        juce::NormalisableRange<float>(0.0f, 100.0f, 1.0f), 100.0f,
        juce::AudioParameterFloatAttributes().withLabel("%")));
    params.push_back(std::make_unique<juce::AudioParameterBool>(
        juce::ParameterID("compMidSide", 1), "Comp Mid/Side", false));
    params.push_back(std::make_unique<juce::AudioParameterBool>(
        juce::ParameterID("compBypass", 1), "Comp Bypass", false));

    // === Global ===
    params.push_back(std::make_unique<juce::AudioParameterFloat>(
        juce::ParameterID("outputGain", 1), "Output Gain",
        juce::NormalisableRange<float>(-12.0f, 12.0f, 0.1f), 0.0f,
        juce::AudioParameterFloatAttributes().withLabel("dB")));
    params.push_back(std::make_unique<juce::AudioParameterBool>(
        juce::ParameterID("globalBypass", 1), "Global Bypass", false));

    return { params.begin(), params.end() };
}

const juce::String MasterBusAudioProcessor::getName() const { return JucePlugin_Name; }
bool MasterBusAudioProcessor::acceptsMidi() const { return false; }
bool MasterBusAudioProcessor::producesMidi() const { return false; }
bool MasterBusAudioProcessor::isMidiEffect() const { return false; }
double MasterBusAudioProcessor::getTailLengthSeconds() const { return 0.0; }
int MasterBusAudioProcessor::getNumPrograms() { return 1; }
int MasterBusAudioProcessor::getCurrentProgram() { return 0; }
void MasterBusAudioProcessor::setCurrentProgram(int index) { juce::ignoreUnused(index); }
const juce::String MasterBusAudioProcessor::getProgramName(int index) { juce::ignoreUnused(index); return {}; }
void MasterBusAudioProcessor::changeProgramName(int index, const juce::String& newName) { juce::ignoreUnused(index, newName); }

void MasterBusAudioProcessor::prepareToPlay(double sampleRate, int samplesPerBlock)
{
    eq.prepare(sampleRate, samplesPerBlock);
    compressor.prepare(sampleRate, samplesPerBlock);
    loudnessMeter.prepare(sampleRate, samplesPerBlock);

    preEQBuffer.setSize(2, samplesPerBlock);
    postProcessBuffer.setSize(2, samplesPerBlock);
}

void MasterBusAudioProcessor::releaseResources()
{
    eq.reset();
    compressor.reset();
    loudnessMeter.reset();
}

bool MasterBusAudioProcessor::isBusesLayoutSupported(const BusesLayout& layouts) const
{
    if (layouts.getMainOutputChannelSet() != juce::AudioChannelSet::mono()
        && layouts.getMainOutputChannelSet() != juce::AudioChannelSet::stereo())
        return false;
    if (layouts.getMainOutputChannelSet() != layouts.getMainInputChannelSet())
        return false;
    return true;
}

void MasterBusAudioProcessor::processBlock(juce::AudioBuffer<float>& buffer, juce::MidiBuffer& midiMessages)
{
    juce::ignoreUnused(midiMessages);
    juce::ScopedNoDenormals noDenormals;

    auto totalNumInputChannels = getTotalNumInputChannels();
    auto totalNumOutputChannels = getTotalNumOutputChannels();

    for (auto i = totalNumInputChannels; i < totalNumOutputChannels; ++i)
        buffer.clear(i, 0, buffer.getNumSamples());

    // Global bypass
    if (globalBypass->load() > 0.5f)
    {
        loudnessMeter.process(buffer);
        return;
    }

    // Measure input level
    float inLevel = 0.0f;
    for (int ch = 0; ch < totalNumInputChannels; ++ch)
        inLevel = std::max(inLevel, buffer.getMagnitude(ch, 0, buffer.getNumSamples()));
    inputLevel.store(DSPUtils::linearToDecibels(inLevel));

    // Store pre-EQ buffer for spectrum analyzer
    preEQBuffer.makeCopyOf(buffer);

    // Update EQ parameters
    eq.setHighPassFrequency(hpfFreq->load());
    eq.setHighPassSlope((static_cast<int>(hpfSlope->load()) + 1) * 6);
    eq.setHighPassEnabled(hpfEnabled->load() > 0.5f);

    eq.setLowPassFrequency(lpfFreq->load());
    eq.setLowPassSlope((static_cast<int>(lpfSlope->load()) + 1) * 6);
    eq.setLowPassEnabled(lpfEnabled->load() > 0.5f);

    eq.setLowShelfFrequency(lsFreq->load());
    eq.setLowShelfGain(lsGain->load());
    eq.setLowShelfEnabled(lsEnabled->load() > 0.5f);

    eq.setHighShelfFrequency(hsFreq->load());
    eq.setHighShelfGain(hsGain->load());
    eq.setHighShelfEnabled(hsEnabled->load() > 0.5f);

    for (int i = 0; i < 4; ++i)
    {
        eq.setBandFrequency(i, bandFreq[i]->load());
        eq.setBandGain(i, bandGain[i]->load());
        eq.setBandQ(i, bandQ[i]->load());
        eq.setBandEnabled(i, bandEnabled[i]->load() > 0.5f);
    }

    eq.setLinearPhase(eqLinearPhase->load() > 0.5f);
    eq.setMidSideMode(eqMidSide->load() > 0.5f);
    eq.setBypass(eqBypass->load() > 0.5f);

    // Process EQ
    eq.process(buffer);

    // Update compressor parameters
    compressor.setThreshold(compThreshold->load());
    compressor.setRatio(compRatio->load());
    compressor.setAttack(compAttack->load());
    compressor.setRelease(compRelease->load());
    compressor.setKnee(compKnee->load());
    compressor.setMakeupGain(compMakeup->load());
    compressor.setMix(compMix->load());
    compressor.setAutoRelease(compAutoRelease->load() > 0.5f);
    compressor.setMode(static_cast<MasteringCompressor::Mode>(static_cast<int>(compMode->load())));
    compressor.setSidechainHPF(compScHpf->load());
    compressor.setSidechainListen(compScListen->load() > 0.5f);
    compressor.setStereoLink(compStereoLink->load());
    compressor.setMidSideMode(compMidSide->load() > 0.5f);
    compressor.setBypass(compBypass->load() > 0.5f);

    // Process compressor
    compressor.process(buffer);

    // Apply output gain
    float outGain = DSPUtils::decibelsToLinear(outputGain->load());
    buffer.applyGain(outGain);

    // Store post-process buffer for spectrum analyzer
    postProcessBuffer.makeCopyOf(buffer);

    // Update loudness metering
    loudnessMeter.process(buffer);

    // Measure output level
    float outLevel = 0.0f;
    for (int ch = 0; ch < totalNumInputChannels; ++ch)
        outLevel = std::max(outLevel, buffer.getMagnitude(ch, 0, buffer.getNumSamples()));
    outputLevel.store(DSPUtils::linearToDecibels(outLevel));
}

void MasterBusAudioProcessor::getStateInformation(juce::MemoryBlock& destData)
{
    auto state = apvts.copyState();
    std::unique_ptr<juce::XmlElement> xml(state.createXml());
    copyXmlToBinary(*xml, destData);
}

void MasterBusAudioProcessor::setStateInformation(const void* data, int sizeInBytes)
{
    std::unique_ptr<juce::XmlElement> xml(getXmlFromBinary(data, sizeInBytes));
    if (xml != nullptr && xml->hasTagName(apvts.state.getType()))
        apvts.replaceState(juce::ValueTree::fromXml(*xml));
}

void MasterBusAudioProcessor::storeSettings(int slot)
{
    if (slot < 0 || slot >= 4) return;

    settingsSlots[slot].parameterValues.clear();
    for (auto* param : getParameters())
    {
        if (auto* p = dynamic_cast<juce::RangedAudioParameter*>(param))
            settingsSlots[slot].parameterValues[p->getParameterID()] = p->getValue();
    }
    settingsSlots[slot].isUsed = true;
    currentSettingsSlot = slot;
}

void MasterBusAudioProcessor::recallSettings(int slot)
{
    if (slot < 0 || slot >= 4) return;
    if (!settingsSlots[slot].isUsed) return;

    for (auto* param : getParameters())
    {
        if (auto* p = dynamic_cast<juce::RangedAudioParameter*>(param))
        {
            auto it = settingsSlots[slot].parameterValues.find(p->getParameterID());
            if (it != settingsSlots[slot].parameterValues.end())
                p->setValueNotifyingHost(it->second);
        }
    }
    currentSettingsSlot = slot;
}

juce::AudioProcessorEditor* MasterBusAudioProcessor::createEditor()
{
    return new MasterBusAudioProcessorEditor(*this);
}

bool MasterBusAudioProcessor::hasEditor() const { return true; }

juce::AudioProcessor* JUCE_CALLTYPE createPluginFilter()
{
    return new MasterBusAudioProcessor();
}
