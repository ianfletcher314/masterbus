#include "PluginProcessor.h"
#include "PluginEditor.h"

MasterBusAudioProcessorEditor::MasterBusAudioProcessorEditor(MasterBusAudioProcessor& p)
    : AudioProcessorEditor(&p), audioProcessor(p)
{
    setLookAndFeel(&mainLookAndFeel);

    // Set up spectrum analyzer
    addAndMakeVisible(spectrumAnalyzer);
    spectrumAnalyzer.setSampleRate(p.getSampleRate());

    addAndMakeVisible(preButton);
    addAndMakeVisible(postButton);
    preButton.onClick = [this] { spectrumAnalyzer.setShowPre(preButton.getToggleState()); };
    postButton.onClick = [this] { spectrumAnalyzer.setShowPost(postButton.getToggleState()); };
    postButton.setToggleState(true, juce::dontSendNotification);

    addAndMakeVisible(slopeSelector);
    slopeSelector.addItemList({ "0 dB/oct", "3 dB/oct", "4.5 dB/oct" }, 1);
    slopeSelector.setSelectedId(2);
    slopeSelector.onChange = [this] {
        float slopes[] = { 0.0f, 3.0f, 4.5f };
        spectrumAnalyzer.setSlope(slopes[slopeSelector.getSelectedId() - 1]);
    };

    // Meter panel
    addAndMakeVisible(meterPanel);

    // A/B/C/D buttons
    const char* abcdLabels[] = { "A", "B", "C", "D" };
    for (int i = 0; i < 4; ++i)
    {
        abcdButtons[i].setButtonText(abcdLabels[i]);
        abcdButtons[i].setClickingTogglesState(true);
        abcdButtons[i].setRadioGroupId(1001);
        abcdButtons[i].onClick = [this, i] {
            if (abcdButtons[i].getToggleState())
                audioProcessor.recallSettings(i);
            else
                audioProcessor.storeSettings(i);
        };
        addAndMakeVisible(abcdButtons[i]);
    }
    abcdButtons[0].setToggleState(true, juce::dontSendNotification);

    // EQ Group
    addAndMakeVisible(eqGroup);
    eqGroup.setText("EQ");
    eqGroup.setColour(juce::GroupComponent::outlineColourId, MasterBusLookAndFeel::Colors::eqAccent);
    eqGroup.setColour(juce::GroupComponent::textColourId, MasterBusLookAndFeel::Colors::eqAccent);

    // HPF
    hpfFreqSlider.setLookAndFeel(&eqLookAndFeel);
    setupRotarySlider(hpfFreqSlider);
    hpfSlopeBox.addItemList({ "6dB", "12dB", "18dB", "24dB" }, 1);
    addAndMakeVisible(hpfSlopeBox);
    addAndMakeVisible(hpfButton);
    addAndMakeVisible(hpfLabel);
    hpfLabel.setJustificationType(juce::Justification::centred);

    // LPF
    lpfFreqSlider.setLookAndFeel(&eqLookAndFeel);
    setupRotarySlider(lpfFreqSlider);
    lpfSlopeBox.addItemList({ "6dB", "12dB", "18dB", "24dB" }, 1);
    addAndMakeVisible(lpfSlopeBox);
    addAndMakeVisible(lpfButton);
    addAndMakeVisible(lpfLabel);
    lpfLabel.setJustificationType(juce::Justification::centred);

    // Low Shelf
    lsFreqSlider.setLookAndFeel(&eqLookAndFeel);
    lsGainSlider.setLookAndFeel(&eqLookAndFeel);
    setupRotarySlider(lsFreqSlider);
    setupRotarySlider(lsGainSlider);
    addAndMakeVisible(lsButton);
    addAndMakeVisible(lsFreqLabel);
    addAndMakeVisible(lsGainLabel);
    lsFreqLabel.setJustificationType(juce::Justification::centred);
    lsGainLabel.setJustificationType(juce::Justification::centred);

    // High Shelf
    hsFreqSlider.setLookAndFeel(&eqLookAndFeel);
    hsGainSlider.setLookAndFeel(&eqLookAndFeel);
    setupRotarySlider(hsFreqSlider);
    setupRotarySlider(hsGainSlider);
    addAndMakeVisible(hsButton);
    addAndMakeVisible(hsFreqLabel);
    addAndMakeVisible(hsGainLabel);
    hsFreqLabel.setJustificationType(juce::Justification::centred);
    hsGainLabel.setJustificationType(juce::Justification::centred);

    // Parametric bands
    const char* bandNames[] = { "1", "2", "3", "4" };
    for (int i = 0; i < 4; ++i)
    {
        bandControls[i].freqSlider.setLookAndFeel(&eqLookAndFeel);
        bandControls[i].gainSlider.setLookAndFeel(&eqLookAndFeel);
        bandControls[i].qSlider.setLookAndFeel(&eqLookAndFeel);
        setupRotarySlider(bandControls[i].freqSlider);
        setupRotarySlider(bandControls[i].gainSlider);
        setupRotarySlider(bandControls[i].qSlider);

        bandControls[i].enableButton.setButtonText(bandNames[i]);
        addAndMakeVisible(bandControls[i].enableButton);
        addAndMakeVisible(bandControls[i].freqLabel);
        addAndMakeVisible(bandControls[i].gainLabel);
        addAndMakeVisible(bandControls[i].qLabel);
        bandControls[i].freqLabel.setJustificationType(juce::Justification::centred);
        bandControls[i].gainLabel.setJustificationType(juce::Justification::centred);
        bandControls[i].qLabel.setJustificationType(juce::Justification::centred);
    }

    // EQ options
    addAndMakeVisible(eqLinearPhaseButton);
    addAndMakeVisible(eqMidSideButton);
    addAndMakeVisible(eqBypassButton);

    // Compressor Group
    addAndMakeVisible(compGroup);
    compGroup.setText("Compressor");
    compGroup.setColour(juce::GroupComponent::outlineColourId, MasterBusLookAndFeel::Colors::compAccent);
    compGroup.setColour(juce::GroupComponent::textColourId, MasterBusLookAndFeel::Colors::compAccent);

    // Compressor sliders
    compThresholdSlider.setLookAndFeel(&compLookAndFeel);
    compRatioSlider.setLookAndFeel(&compLookAndFeel);
    compAttackSlider.setLookAndFeel(&compLookAndFeel);
    compReleaseSlider.setLookAndFeel(&compLookAndFeel);
    compKneeSlider.setLookAndFeel(&compLookAndFeel);
    compMakeupSlider.setLookAndFeel(&compLookAndFeel);
    compMixSlider.setLookAndFeel(&compLookAndFeel);
    compScHpfSlider.setLookAndFeel(&compLookAndFeel);
    compStereoLinkSlider.setLookAndFeel(&compLookAndFeel);

    setupRotarySlider(compThresholdSlider);
    setupRotarySlider(compRatioSlider);
    setupRotarySlider(compAttackSlider);
    setupRotarySlider(compReleaseSlider);
    setupRotarySlider(compKneeSlider);
    setupRotarySlider(compMakeupSlider);
    setupRotarySlider(compMixSlider);
    setupRotarySlider(compScHpfSlider);
    setupRotarySlider(compStereoLinkSlider);

    for (auto* label : { &compThreshLabel, &compRatioLabel, &compAttackLabel, &compReleaseLabel,
                         &compKneeLabel, &compMakeupLabel, &compMixLabel, &compScHpfLabel, &compLinkLabel })
    {
        label->setJustificationType(juce::Justification::centred);
        addAndMakeVisible(label);
    }

    compModeBox.addItemList({ "Clean", "Glue", "Punch", "Vintage" }, 1);
    addAndMakeVisible(compModeBox);

    addAndMakeVisible(compAutoReleaseButton);
    addAndMakeVisible(compScListenButton);
    addAndMakeVisible(compMidSideButton);
    addAndMakeVisible(compBypassButton);

    // Output
    outputGainSlider.setLookAndFeel(&mainLookAndFeel);
    setupRotarySlider(outputGainSlider);
    addAndMakeVisible(outputGainLabel);
    outputGainLabel.setJustificationType(juce::Justification::centred);

    addAndMakeVisible(globalBypassButton);
    addAndMakeVisible(monoButton);
    addAndMakeVisible(dimButton);

    // Create attachments
    auto& apvts = audioProcessor.getAPVTS();

    // HPF
    hpfFreqAttachment = std::make_unique<juce::AudioProcessorValueTreeState::SliderAttachment>(apvts, "hpfFreq", hpfFreqSlider);
    hpfSlopeAttachment = std::make_unique<juce::AudioProcessorValueTreeState::ComboBoxAttachment>(apvts, "hpfSlope", hpfSlopeBox);
    hpfEnabledAttachment = std::make_unique<juce::AudioProcessorValueTreeState::ButtonAttachment>(apvts, "hpfEnabled", hpfButton);

    // LPF
    lpfFreqAttachment = std::make_unique<juce::AudioProcessorValueTreeState::SliderAttachment>(apvts, "lpfFreq", lpfFreqSlider);
    lpfSlopeAttachment = std::make_unique<juce::AudioProcessorValueTreeState::ComboBoxAttachment>(apvts, "lpfSlope", lpfSlopeBox);
    lpfEnabledAttachment = std::make_unique<juce::AudioProcessorValueTreeState::ButtonAttachment>(apvts, "lpfEnabled", lpfButton);

    // Low Shelf
    lsFreqAttachment = std::make_unique<juce::AudioProcessorValueTreeState::SliderAttachment>(apvts, "lsFreq", lsFreqSlider);
    lsGainAttachment = std::make_unique<juce::AudioProcessorValueTreeState::SliderAttachment>(apvts, "lsGain", lsGainSlider);
    lsEnabledAttachment = std::make_unique<juce::AudioProcessorValueTreeState::ButtonAttachment>(apvts, "lsEnabled", lsButton);

    // High Shelf
    hsFreqAttachment = std::make_unique<juce::AudioProcessorValueTreeState::SliderAttachment>(apvts, "hsFreq", hsFreqSlider);
    hsGainAttachment = std::make_unique<juce::AudioProcessorValueTreeState::SliderAttachment>(apvts, "hsGain", hsGainSlider);
    hsEnabledAttachment = std::make_unique<juce::AudioProcessorValueTreeState::ButtonAttachment>(apvts, "hsEnabled", hsButton);

    // Bands
    for (int i = 0; i < 4; ++i)
    {
        juce::String prefix = "band" + juce::String(i + 1);
        bandAttachments[i].freq = std::make_unique<juce::AudioProcessorValueTreeState::SliderAttachment>(
            apvts, prefix + "Freq", bandControls[i].freqSlider);
        bandAttachments[i].gain = std::make_unique<juce::AudioProcessorValueTreeState::SliderAttachment>(
            apvts, prefix + "Gain", bandControls[i].gainSlider);
        bandAttachments[i].q = std::make_unique<juce::AudioProcessorValueTreeState::SliderAttachment>(
            apvts, prefix + "Q", bandControls[i].qSlider);
        bandAttachments[i].enabled = std::make_unique<juce::AudioProcessorValueTreeState::ButtonAttachment>(
            apvts, prefix + "Enabled", bandControls[i].enableButton);
    }

    // EQ Global
    eqLinearPhaseAttachment = std::make_unique<juce::AudioProcessorValueTreeState::ButtonAttachment>(apvts, "eqLinearPhase", eqLinearPhaseButton);
    eqMidSideAttachment = std::make_unique<juce::AudioProcessorValueTreeState::ButtonAttachment>(apvts, "eqMidSide", eqMidSideButton);
    eqBypassAttachment = std::make_unique<juce::AudioProcessorValueTreeState::ButtonAttachment>(apvts, "eqBypass", eqBypassButton);

    // Compressor
    compThresholdAttachment = std::make_unique<juce::AudioProcessorValueTreeState::SliderAttachment>(apvts, "compThreshold", compThresholdSlider);
    compRatioAttachment = std::make_unique<juce::AudioProcessorValueTreeState::SliderAttachment>(apvts, "compRatio", compRatioSlider);
    compAttackAttachment = std::make_unique<juce::AudioProcessorValueTreeState::SliderAttachment>(apvts, "compAttack", compAttackSlider);
    compReleaseAttachment = std::make_unique<juce::AudioProcessorValueTreeState::SliderAttachment>(apvts, "compRelease", compReleaseSlider);
    compKneeAttachment = std::make_unique<juce::AudioProcessorValueTreeState::SliderAttachment>(apvts, "compKnee", compKneeSlider);
    compMakeupAttachment = std::make_unique<juce::AudioProcessorValueTreeState::SliderAttachment>(apvts, "compMakeup", compMakeupSlider);
    compMixAttachment = std::make_unique<juce::AudioProcessorValueTreeState::SliderAttachment>(apvts, "compMix", compMixSlider);
    compScHpfAttachment = std::make_unique<juce::AudioProcessorValueTreeState::SliderAttachment>(apvts, "compScHpf", compScHpfSlider);
    compStereoLinkAttachment = std::make_unique<juce::AudioProcessorValueTreeState::SliderAttachment>(apvts, "compStereoLink", compStereoLinkSlider);
    compModeAttachment = std::make_unique<juce::AudioProcessorValueTreeState::ComboBoxAttachment>(apvts, "compMode", compModeBox);
    compAutoReleaseAttachment = std::make_unique<juce::AudioProcessorValueTreeState::ButtonAttachment>(apvts, "compAutoRelease", compAutoReleaseButton);
    compScListenAttachment = std::make_unique<juce::AudioProcessorValueTreeState::ButtonAttachment>(apvts, "compScListen", compScListenButton);
    compMidSideAttachment = std::make_unique<juce::AudioProcessorValueTreeState::ButtonAttachment>(apvts, "compMidSide", compMidSideButton);
    compBypassAttachment = std::make_unique<juce::AudioProcessorValueTreeState::ButtonAttachment>(apvts, "compBypass", compBypassButton);

    // Output
    outputGainAttachment = std::make_unique<juce::AudioProcessorValueTreeState::SliderAttachment>(apvts, "outputGain", outputGainSlider);
    globalBypassAttachment = std::make_unique<juce::AudioProcessorValueTreeState::ButtonAttachment>(apvts, "globalBypass", globalBypassButton);

    // Start timer for metering updates
    startTimerHz(30);

    setSize(1000, 700);
}

MasterBusAudioProcessorEditor::~MasterBusAudioProcessorEditor()
{
    stopTimer();

    // Clear look and feel references
    hpfFreqSlider.setLookAndFeel(nullptr);
    lpfFreqSlider.setLookAndFeel(nullptr);
    lsFreqSlider.setLookAndFeel(nullptr);
    lsGainSlider.setLookAndFeel(nullptr);
    hsFreqSlider.setLookAndFeel(nullptr);
    hsGainSlider.setLookAndFeel(nullptr);

    for (auto& band : bandControls)
    {
        band.freqSlider.setLookAndFeel(nullptr);
        band.gainSlider.setLookAndFeel(nullptr);
        band.qSlider.setLookAndFeel(nullptr);
    }

    compThresholdSlider.setLookAndFeel(nullptr);
    compRatioSlider.setLookAndFeel(nullptr);
    compAttackSlider.setLookAndFeel(nullptr);
    compReleaseSlider.setLookAndFeel(nullptr);
    compKneeSlider.setLookAndFeel(nullptr);
    compMakeupSlider.setLookAndFeel(nullptr);
    compMixSlider.setLookAndFeel(nullptr);
    compScHpfSlider.setLookAndFeel(nullptr);
    compStereoLinkSlider.setLookAndFeel(nullptr);
    outputGainSlider.setLookAndFeel(nullptr);

    setLookAndFeel(nullptr);
}

void MasterBusAudioProcessorEditor::setupRotarySlider(juce::Slider& slider)
{
    slider.setSliderStyle(juce::Slider::RotaryHorizontalVerticalDrag);
    slider.setTextBoxStyle(juce::Slider::TextBoxBelow, false, 60, 18);
    addAndMakeVisible(slider);
}

void MasterBusAudioProcessorEditor::setupSlider(juce::Slider& slider, juce::Label& label, const juce::String& suffix)
{
    slider.setSliderStyle(juce::Slider::LinearVertical);
    slider.setTextBoxStyle(juce::Slider::TextBoxBelow, false, 50, 18);
    if (!suffix.isEmpty())
        slider.setTextValueSuffix(suffix);
    addAndMakeVisible(slider);
    label.setJustificationType(juce::Justification::centred);
    addAndMakeVisible(label);
}

void MasterBusAudioProcessorEditor::timerCallback()
{
    // Update meters
    auto& meter = audioProcessor.getLoudnessMeter();
    meterPanel.getInputMeter().setLevel(audioProcessor.getInputLevel());
    meterPanel.getOutputMeter().setLevel(audioProcessor.getOutputLevel());
    meterPanel.getGRMeter().setGainReduction(audioProcessor.getGainReduction());
    meterPanel.getLoudnessMeter().setMomentary(meter.getMomentaryLoudness());
    meterPanel.getLoudnessMeter().setShortTerm(meter.getShortTermLoudness());
    meterPanel.getLoudnessMeter().setIntegrated(meter.getIntegratedLoudness());
    meterPanel.getLoudnessMeter().setTruePeak(meter.getTruePeakLevel());
    meterPanel.getCorrelationMeter().setCorrelation(meter.getStereoCorrelation());
    meterPanel.getBalanceMeter().setBalance(meter.getStereoBalance());

    // Update spectrum analyzer
    spectrumAnalyzer.pushPreBuffer(audioProcessor.getPreEQBuffer());
    spectrumAnalyzer.pushPostBuffer(audioProcessor.getPostProcessBuffer());
}

void MasterBusAudioProcessorEditor::paint(juce::Graphics& g)
{
    g.fillAll(MasterBusLookAndFeel::Colors::background);

    // Header
    auto headerArea = getLocalBounds().removeFromTop(40);
    g.setColour(MasterBusLookAndFeel::Colors::panelBackground);
    g.fillRect(headerArea);

    g.setColour(MasterBusLookAndFeel::Colors::textPrimary);
    g.setFont(juce::Font(juce::FontOptions(24.0f).withStyle("Bold")));
    g.drawText("MASTERBUS", headerArea.reduced(10, 0), juce::Justification::centredLeft);

    // Version
    g.setFont(juce::Font(juce::FontOptions(12.0f)));
    g.setColour(MasterBusLookAndFeel::Colors::textDim);
    g.drawText("v1.0", headerArea.reduced(10, 0).removeFromRight(50), juce::Justification::centredRight);
}

void MasterBusAudioProcessorEditor::resized()
{
    auto bounds = getLocalBounds();

    // Header area (A/B/C/D buttons)
    auto headerArea = bounds.removeFromTop(40);
    auto abcdArea = headerArea.removeFromRight(200).reduced(5);
    int abcdWidth = abcdArea.getWidth() / 4;
    for (int i = 0; i < 4; ++i)
        abcdButtons[i].setBounds(abcdArea.removeFromLeft(abcdWidth).reduced(2));

    // Main content
    auto contentArea = bounds.reduced(10);

    // Spectrum analyzer at top
    auto analyzerArea = contentArea.removeFromTop(180);
    spectrumAnalyzer.setBounds(analyzerArea.reduced(0, 0));

    // Analyzer controls
    auto analyzerControls = analyzerArea.removeFromBottom(25);
    preButton.setBounds(analyzerControls.removeFromLeft(50));
    postButton.setBounds(analyzerControls.removeFromLeft(50));
    analyzerControls.removeFromLeft(10);
    slopeSelector.setBounds(analyzerControls.removeFromLeft(100));

    contentArea.removeFromTop(10);

    // Bottom section: EQ, Compressor, Meters
    auto bottomArea = contentArea;

    // Meters on right
    auto meterArea = bottomArea.removeFromRight(150);
    meterPanel.setBounds(meterArea);

    bottomArea.removeFromRight(10);

    // EQ and Compressor side by side
    int halfWidth = bottomArea.getWidth() / 2;

    // EQ Section
    auto eqArea = bottomArea.removeFromLeft(halfWidth - 5);
    eqGroup.setBounds(eqArea);
    auto eqContent = eqArea.reduced(10, 20);

    // EQ controls layout
    int knobSize = 55;
    int labelHeight = 18;
    int rowHeight = knobSize + labelHeight + 5;

    // Row 1: HPF, LPF, Low Shelf, High Shelf
    auto row1 = eqContent.removeFromTop(rowHeight + 30);

    // HPF
    auto hpfArea = row1.removeFromLeft(80);
    hpfLabel.setBounds(hpfArea.removeFromTop(labelHeight));
    hpfButton.setBounds(hpfArea.removeFromTop(20));
    hpfFreqSlider.setBounds(hpfArea.removeFromTop(knobSize));
    hpfSlopeBox.setBounds(hpfArea.removeFromTop(20).reduced(5, 0));

    row1.removeFromLeft(5);

    // LPF
    auto lpfArea = row1.removeFromLeft(80);
    lpfLabel.setBounds(lpfArea.removeFromTop(labelHeight));
    lpfButton.setBounds(lpfArea.removeFromTop(20));
    lpfFreqSlider.setBounds(lpfArea.removeFromTop(knobSize));
    lpfSlopeBox.setBounds(lpfArea.removeFromTop(20).reduced(5, 0));

    row1.removeFromLeft(10);

    // Low Shelf
    auto lsArea = row1.removeFromLeft(130);
    lsButton.setBounds(lsArea.removeFromTop(20).withWidth(40));
    auto lsKnobs = lsArea.removeFromTop(knobSize);
    lsFreqSlider.setBounds(lsKnobs.removeFromLeft(65));
    lsGainSlider.setBounds(lsKnobs.removeFromLeft(65));
    auto lsLabels = lsArea.removeFromTop(labelHeight);
    lsFreqLabel.setBounds(lsLabels.removeFromLeft(65));
    lsGainLabel.setBounds(lsLabels.removeFromLeft(65));

    row1.removeFromLeft(10);

    // High Shelf
    auto hsArea = row1.removeFromLeft(130);
    hsButton.setBounds(hsArea.removeFromTop(20).withWidth(40));
    auto hsKnobs = hsArea.removeFromTop(knobSize);
    hsFreqSlider.setBounds(hsKnobs.removeFromLeft(65));
    hsGainSlider.setBounds(hsKnobs.removeFromLeft(65));
    auto hsLabels = hsArea.removeFromTop(labelHeight);
    hsFreqLabel.setBounds(hsLabels.removeFromLeft(65));
    hsGainLabel.setBounds(hsLabels.removeFromLeft(65));

    eqContent.removeFromTop(10);

    // Row 2: Parametric bands
    auto row2 = eqContent.removeFromTop(rowHeight * 2);
    int bandWidth = row2.getWidth() / 4;

    for (int i = 0; i < 4; ++i)
    {
        auto bandArea = row2.removeFromLeft(bandWidth);
        bandControls[i].enableButton.setBounds(bandArea.removeFromTop(20).withWidth(30).withX(bandArea.getX() + bandWidth/2 - 15));

        auto knobRow1 = bandArea.removeFromTop(knobSize);
        bandControls[i].freqSlider.setBounds(knobRow1);
        bandControls[i].freqLabel.setBounds(bandArea.removeFromTop(labelHeight));

        auto knobRow2 = bandArea.removeFromTop(knobSize);
        int halfKnob = knobRow2.getWidth() / 2;
        bandControls[i].gainSlider.setBounds(knobRow2.removeFromLeft(halfKnob));
        bandControls[i].qSlider.setBounds(knobRow2);

        auto labelRow = bandArea.removeFromTop(labelHeight);
        bandControls[i].gainLabel.setBounds(labelRow.removeFromLeft(halfKnob));
        bandControls[i].qLabel.setBounds(labelRow);
    }

    // EQ Options
    auto eqOptionsArea = eqContent.removeFromTop(30);
    eqLinearPhaseButton.setBounds(eqOptionsArea.removeFromLeft(100));
    eqMidSideButton.setBounds(eqOptionsArea.removeFromLeft(60));
    eqBypassButton.setBounds(eqOptionsArea.removeFromRight(70));

    bottomArea.removeFromLeft(10);

    // Compressor Section
    auto compArea = bottomArea;
    compGroup.setBounds(compArea);
    auto compContent = compArea.reduced(10, 20);

    // Comp Row 1: Threshold, Ratio, Knee
    auto compRow1 = compContent.removeFromTop(rowHeight);
    int compKnobWidth = compRow1.getWidth() / 3;

    auto threshArea = compRow1.removeFromLeft(compKnobWidth);
    compThresholdSlider.setBounds(threshArea.removeFromTop(knobSize));
    compThreshLabel.setBounds(threshArea.removeFromTop(labelHeight));

    auto ratioArea = compRow1.removeFromLeft(compKnobWidth);
    compRatioSlider.setBounds(ratioArea.removeFromTop(knobSize));
    compRatioLabel.setBounds(ratioArea.removeFromTop(labelHeight));

    auto kneeArea = compRow1;
    compKneeSlider.setBounds(kneeArea.removeFromTop(knobSize));
    compKneeLabel.setBounds(kneeArea.removeFromTop(labelHeight));

    compContent.removeFromTop(5);

    // Comp Row 2: Attack, Release, Makeup
    auto compRow2 = compContent.removeFromTop(rowHeight);

    auto attackArea = compRow2.removeFromLeft(compKnobWidth);
    compAttackSlider.setBounds(attackArea.removeFromTop(knobSize));
    compAttackLabel.setBounds(attackArea.removeFromTop(labelHeight));

    auto releaseArea = compRow2.removeFromLeft(compKnobWidth);
    compReleaseSlider.setBounds(releaseArea.removeFromTop(knobSize));
    compReleaseLabel.setBounds(releaseArea.removeFromTop(labelHeight));

    auto makeupArea = compRow2;
    compMakeupSlider.setBounds(makeupArea.removeFromTop(knobSize));
    compMakeupLabel.setBounds(makeupArea.removeFromTop(labelHeight));

    compContent.removeFromTop(5);

    // Comp Row 3: Mix, SC HPF, Stereo Link
    auto compRow3 = compContent.removeFromTop(rowHeight);

    auto mixArea = compRow3.removeFromLeft(compKnobWidth);
    compMixSlider.setBounds(mixArea.removeFromTop(knobSize));
    compMixLabel.setBounds(mixArea.removeFromTop(labelHeight));

    auto scHpfArea = compRow3.removeFromLeft(compKnobWidth);
    compScHpfSlider.setBounds(scHpfArea.removeFromTop(knobSize));
    compScHpfLabel.setBounds(scHpfArea.removeFromTop(labelHeight));

    auto linkArea = compRow3;
    compStereoLinkSlider.setBounds(linkArea.removeFromTop(knobSize));
    compLinkLabel.setBounds(linkArea.removeFromTop(labelHeight));

    compContent.removeFromTop(5);

    // Comp Row 4: Mode selector and options
    auto compRow4 = compContent.removeFromTop(30);
    compModeBox.setBounds(compRow4.removeFromLeft(100));
    compRow4.removeFromLeft(10);
    compAutoReleaseButton.setBounds(compRow4.removeFromLeft(80));
    compScListenButton.setBounds(compRow4.removeFromLeft(80));

    compContent.removeFromTop(5);

    // Comp Row 5: More options
    auto compRow5 = compContent.removeFromTop(30);
    compMidSideButton.setBounds(compRow5.removeFromLeft(60));
    compBypassButton.setBounds(compRow5.removeFromRight(70));

    compContent.removeFromTop(10);

    // Output section
    auto outputArea = compContent.removeFromTop(rowHeight);
    int outputWidth = outputArea.getWidth() / 3;

    auto outGainArea = outputArea.removeFromLeft(outputWidth);
    outputGainSlider.setBounds(outGainArea.removeFromTop(knobSize));
    outputGainLabel.setBounds(outGainArea.removeFromTop(labelHeight));

    auto bypassArea = outputArea.removeFromLeft(outputWidth);
    globalBypassButton.setBounds(bypassArea.reduced(10, 15));

    auto monitorArea = outputArea;
    monoButton.setBounds(monitorArea.removeFromTop(25).reduced(5, 2));
    dimButton.setBounds(monitorArea.removeFromTop(25).reduced(5, 2));
}
