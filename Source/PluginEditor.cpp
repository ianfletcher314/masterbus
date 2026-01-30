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

    // Panel toggle buttons
    addAndMakeVisible(eqToggleButton);
    addAndMakeVisible(compToggleButton);
    eqToggleButton.setClickingTogglesState(true);
    compToggleButton.setClickingTogglesState(true);
    eqToggleButton.onClick = [this] {
        eqPanelVisible = eqToggleButton.getToggleState();
        updatePanelVisibility();
        resized();
    };
    compToggleButton.onClick = [this] {
        compPanelVisible = compToggleButton.getToggleState();
        updatePanelVisibility();
        resized();
    };

    // Setup collapsible panels
    addChildComponent(eqPanel);
    addChildComponent(compPanel);
    eqPanel.setContentComponent(&eqContent);
    compPanel.setContentComponent(&compContent);

    // HPF
    hpfFreqSlider.setLookAndFeel(&eqLookAndFeel);
    setupRotarySlider(hpfFreqSlider);
    hpfSlopeBox.addItemList({ "6dB", "12dB", "18dB", "24dB" }, 1);
    eqContent.addAndMakeVisible(hpfSlopeBox);
    eqContent.addAndMakeVisible(hpfButton);
    eqContent.addAndMakeVisible(hpfLabel);
    hpfLabel.setJustificationType(juce::Justification::centred);

    // LPF
    lpfFreqSlider.setLookAndFeel(&eqLookAndFeel);
    setupRotarySlider(lpfFreqSlider);
    lpfSlopeBox.addItemList({ "6dB", "12dB", "18dB", "24dB" }, 1);
    eqContent.addAndMakeVisible(lpfSlopeBox);
    eqContent.addAndMakeVisible(lpfButton);
    eqContent.addAndMakeVisible(lpfLabel);
    lpfLabel.setJustificationType(juce::Justification::centred);

    // Low Shelf
    lsFreqSlider.setLookAndFeel(&eqLookAndFeel);
    lsGainSlider.setLookAndFeel(&eqLookAndFeel);
    setupRotarySlider(lsFreqSlider);
    setupRotarySlider(lsGainSlider);
    eqContent.addAndMakeVisible(lsButton);
    eqContent.addAndMakeVisible(lsFreqLabel);
    eqContent.addAndMakeVisible(lsGainLabel);
    lsFreqLabel.setJustificationType(juce::Justification::centred);
    lsGainLabel.setJustificationType(juce::Justification::centred);

    // High Shelf
    hsFreqSlider.setLookAndFeel(&eqLookAndFeel);
    hsGainSlider.setLookAndFeel(&eqLookAndFeel);
    setupRotarySlider(hsFreqSlider);
    setupRotarySlider(hsGainSlider);
    eqContent.addAndMakeVisible(hsButton);
    eqContent.addAndMakeVisible(hsFreqLabel);
    eqContent.addAndMakeVisible(hsGainLabel);
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
        eqContent.addAndMakeVisible(bandControls[i].enableButton);
        eqContent.addAndMakeVisible(bandControls[i].freqLabel);
        eqContent.addAndMakeVisible(bandControls[i].gainLabel);
        eqContent.addAndMakeVisible(bandControls[i].qLabel);
        bandControls[i].freqLabel.setJustificationType(juce::Justification::centred);
        bandControls[i].gainLabel.setJustificationType(juce::Justification::centred);
        bandControls[i].qLabel.setJustificationType(juce::Justification::centred);
    }

    // EQ options
    eqContent.addAndMakeVisible(eqLinearPhaseButton);
    eqContent.addAndMakeVisible(eqMidSideButton);
    eqContent.addAndMakeVisible(eqBypassButton);

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
        compContent.addAndMakeVisible(label);
    }

    compModeBox.addItemList({ "Clean", "Glue", "Punch", "Vintage" }, 1);
    compContent.addAndMakeVisible(compModeBox);

    compContent.addAndMakeVisible(compAutoReleaseButton);
    compContent.addAndMakeVisible(compScListenButton);
    compContent.addAndMakeVisible(compMidSideButton);
    compContent.addAndMakeVisible(compBypassButton);

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
    // Note: We add sliders to parent based on which section they belong to
}

void MasterBusAudioProcessorEditor::setupSlider(juce::Slider& slider, juce::Label& label, const juce::String& suffix)
{
    slider.setSliderStyle(juce::Slider::LinearVertical);
    slider.setTextBoxStyle(juce::Slider::TextBoxBelow, false, 50, 18);
    if (!suffix.isEmpty())
        slider.setTextValueSuffix(suffix);
    label.setJustificationType(juce::Justification::centred);
}

void MasterBusAudioProcessorEditor::updatePanelVisibility()
{
    eqPanel.setVisible(eqPanelVisible);
    compPanel.setVisible(compPanelVisible);
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

    // Header area
    auto headerArea = bounds.removeFromTop(40);

    // A/B/C/D buttons in header
    auto abcdArea = headerArea.removeFromRight(200).reduced(5);
    int abcdWidth = abcdArea.getWidth() / 4;
    for (int i = 0; i < 4; ++i)
        abcdButtons[i].setBounds(abcdArea.removeFromLeft(abcdWidth).reduced(2));

    // Panel toggle buttons in header
    auto toggleArea = headerArea.removeFromRight(160).reduced(5);
    eqToggleButton.setBounds(toggleArea.removeFromLeft(75).reduced(2));
    compToggleButton.setBounds(toggleArea.removeFromLeft(75).reduced(2));

    // Main content area
    auto contentArea = bounds.reduced(10);

    // Meters on the right side (narrow strip)
    auto meterArea = contentArea.removeFromRight(100);
    meterPanel.setBounds(meterArea);

    contentArea.removeFromRight(10);

    // Bottom toolbar area for output controls
    auto bottomBar = contentArea.removeFromBottom(50);

    // Analyzer controls at bottom of analyzer area
    auto analyzerControlsArea = contentArea.removeFromBottom(30);
    preButton.setBounds(analyzerControlsArea.removeFromLeft(50).reduced(2));
    postButton.setBounds(analyzerControlsArea.removeFromLeft(50).reduced(2));
    analyzerControlsArea.removeFromLeft(10);
    slopeSelector.setBounds(analyzerControlsArea.removeFromLeft(100).reduced(2));

    // Output controls in bottom bar
    int outputKnobSize = 45;
    auto outputArea = bottomBar.removeFromLeft(outputKnobSize + 10);
    outputGainSlider.setBounds(outputArea.removeFromTop(outputKnobSize));
    outputGainLabel.setBounds(outputArea);

    bottomBar.removeFromLeft(10);
    globalBypassButton.setBounds(bottomBar.removeFromLeft(70).reduced(2, 10));
    bottomBar.removeFromLeft(5);
    monoButton.setBounds(bottomBar.removeFromLeft(50).reduced(2, 10));
    dimButton.setBounds(bottomBar.removeFromLeft(50).reduced(2, 10));

    // Spectrum analyzer takes the majority of remaining space (~70%)
    spectrumAnalyzer.setBounds(contentArea);

    // Calculate panel positions (overlay on top of analyzer)
    int panelWidth = 450;
    int panelHeight = 350;

    if (eqPanelVisible)
    {
        auto eqBounds = contentArea.withWidth(panelWidth).withHeight(panelHeight);
        eqBounds.setX(contentArea.getX() + 10);
        eqBounds.setY(contentArea.getY() + 10);
        eqPanel.setBounds(eqBounds);
        layoutEQContent();
    }

    if (compPanelVisible)
    {
        auto compBounds = contentArea.withWidth(panelWidth).withHeight(panelHeight);
        compBounds.setX(contentArea.getRight() - panelWidth - 10);
        compBounds.setY(contentArea.getY() + 10);
        compPanel.setBounds(compBounds);
        layoutCompContent();
    }
}

void MasterBusAudioProcessorEditor::layoutEQContent()
{
    auto bounds = eqContent.getLocalBounds();
    int knobSize = 50;
    int labelHeight = 16;
    int rowHeight = knobSize + labelHeight + 5;

    // Row 1: HPF, LPF, Low Shelf, High Shelf
    auto row1 = bounds.removeFromTop(rowHeight + 25);

    // HPF
    auto hpfArea = row1.removeFromLeft(70);
    hpfLabel.setBounds(hpfArea.removeFromTop(labelHeight));
    hpfButton.setBounds(hpfArea.removeFromTop(18));
    hpfFreqSlider.setBounds(hpfArea.removeFromTop(knobSize));
    hpfSlopeBox.setBounds(hpfArea.removeFromTop(18).reduced(2, 0));

    row1.removeFromLeft(5);

    // LPF
    auto lpfArea = row1.removeFromLeft(70);
    lpfLabel.setBounds(lpfArea.removeFromTop(labelHeight));
    lpfButton.setBounds(lpfArea.removeFromTop(18));
    lpfFreqSlider.setBounds(lpfArea.removeFromTop(knobSize));
    lpfSlopeBox.setBounds(lpfArea.removeFromTop(18).reduced(2, 0));

    row1.removeFromLeft(10);

    // Low Shelf
    auto lsArea = row1.removeFromLeft(110);
    lsButton.setBounds(lsArea.removeFromTop(18).withWidth(35));
    auto lsKnobs = lsArea.removeFromTop(knobSize);
    lsFreqSlider.setBounds(lsKnobs.removeFromLeft(55));
    lsGainSlider.setBounds(lsKnobs.removeFromLeft(55));
    auto lsLabels = lsArea.removeFromTop(labelHeight);
    lsFreqLabel.setBounds(lsLabels.removeFromLeft(55));
    lsGainLabel.setBounds(lsLabels.removeFromLeft(55));

    row1.removeFromLeft(10);

    // High Shelf
    auto hsArea = row1.removeFromLeft(110);
    hsButton.setBounds(hsArea.removeFromTop(18).withWidth(35));
    auto hsKnobs = hsArea.removeFromTop(knobSize);
    hsFreqSlider.setBounds(hsKnobs.removeFromLeft(55));
    hsGainSlider.setBounds(hsKnobs.removeFromLeft(55));
    auto hsLabels = hsArea.removeFromTop(labelHeight);
    hsFreqLabel.setBounds(hsLabels.removeFromLeft(55));
    hsGainLabel.setBounds(hsLabels.removeFromLeft(55));

    bounds.removeFromTop(5);

    // Row 2: Parametric bands (2x2 layout)
    auto row2 = bounds.removeFromTop(rowHeight * 2);
    int bandWidth = row2.getWidth() / 4;

    for (int i = 0; i < 4; ++i)
    {
        auto bandArea = row2.removeFromLeft(bandWidth);
        bandControls[i].enableButton.setBounds(bandArea.removeFromTop(18).withWidth(25).withX(bandArea.getX() + bandWidth/2 - 12));

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

    bounds.removeFromTop(5);

    // EQ Options row
    auto optionsRow = bounds.removeFromTop(25);
    eqLinearPhaseButton.setBounds(optionsRow.removeFromLeft(90).reduced(2));
    eqMidSideButton.setBounds(optionsRow.removeFromLeft(50).reduced(2));
    eqBypassButton.setBounds(optionsRow.removeFromRight(60).reduced(2));

    // Add sliders to eqContent
    eqContent.addAndMakeVisible(hpfFreqSlider);
    eqContent.addAndMakeVisible(lpfFreqSlider);
    eqContent.addAndMakeVisible(lsFreqSlider);
    eqContent.addAndMakeVisible(lsGainSlider);
    eqContent.addAndMakeVisible(hsFreqSlider);
    eqContent.addAndMakeVisible(hsGainSlider);
    for (int i = 0; i < 4; ++i)
    {
        eqContent.addAndMakeVisible(bandControls[i].freqSlider);
        eqContent.addAndMakeVisible(bandControls[i].gainSlider);
        eqContent.addAndMakeVisible(bandControls[i].qSlider);
    }
}

void MasterBusAudioProcessorEditor::layoutCompContent()
{
    auto bounds = compContent.getLocalBounds();
    int knobSize = 50;
    int labelHeight = 16;
    int rowHeight = knobSize + labelHeight + 5;

    // Row 1: Threshold, Ratio, Knee
    auto row1 = bounds.removeFromTop(rowHeight);
    int compKnobWidth = row1.getWidth() / 3;

    auto threshArea = row1.removeFromLeft(compKnobWidth);
    compThresholdSlider.setBounds(threshArea.removeFromTop(knobSize));
    compThreshLabel.setBounds(threshArea.removeFromTop(labelHeight));

    auto ratioArea = row1.removeFromLeft(compKnobWidth);
    compRatioSlider.setBounds(ratioArea.removeFromTop(knobSize));
    compRatioLabel.setBounds(ratioArea.removeFromTop(labelHeight));

    auto kneeArea = row1;
    compKneeSlider.setBounds(kneeArea.removeFromTop(knobSize));
    compKneeLabel.setBounds(kneeArea.removeFromTop(labelHeight));

    bounds.removeFromTop(5);

    // Row 2: Attack, Release, Makeup
    auto row2 = bounds.removeFromTop(rowHeight);

    auto attackArea = row2.removeFromLeft(compKnobWidth);
    compAttackSlider.setBounds(attackArea.removeFromTop(knobSize));
    compAttackLabel.setBounds(attackArea.removeFromTop(labelHeight));

    auto releaseArea = row2.removeFromLeft(compKnobWidth);
    compReleaseSlider.setBounds(releaseArea.removeFromTop(knobSize));
    compReleaseLabel.setBounds(releaseArea.removeFromTop(labelHeight));

    auto makeupArea = row2;
    compMakeupSlider.setBounds(makeupArea.removeFromTop(knobSize));
    compMakeupLabel.setBounds(makeupArea.removeFromTop(labelHeight));

    bounds.removeFromTop(5);

    // Row 3: Mix, SC HPF, Stereo Link
    auto row3 = bounds.removeFromTop(rowHeight);

    auto mixArea = row3.removeFromLeft(compKnobWidth);
    compMixSlider.setBounds(mixArea.removeFromTop(knobSize));
    compMixLabel.setBounds(mixArea.removeFromTop(labelHeight));

    auto scHpfArea = row3.removeFromLeft(compKnobWidth);
    compScHpfSlider.setBounds(scHpfArea.removeFromTop(knobSize));
    compScHpfLabel.setBounds(scHpfArea.removeFromTop(labelHeight));

    auto linkArea = row3;
    compStereoLinkSlider.setBounds(linkArea.removeFromTop(knobSize));
    compLinkLabel.setBounds(linkArea.removeFromTop(labelHeight));

    bounds.removeFromTop(5);

    // Row 4: Mode selector and options
    auto row4 = bounds.removeFromTop(25);
    compModeBox.setBounds(row4.removeFromLeft(90).reduced(2));
    row4.removeFromLeft(5);
    compAutoReleaseButton.setBounds(row4.removeFromLeft(70).reduced(2));
    compScListenButton.setBounds(row4.removeFromLeft(70).reduced(2));

    bounds.removeFromTop(5);

    // Row 5: More options
    auto row5 = bounds.removeFromTop(25);
    compMidSideButton.setBounds(row5.removeFromLeft(50).reduced(2));
    compBypassButton.setBounds(row5.removeFromRight(60).reduced(2));

    // Add sliders to compContent
    compContent.addAndMakeVisible(compThresholdSlider);
    compContent.addAndMakeVisible(compRatioSlider);
    compContent.addAndMakeVisible(compAttackSlider);
    compContent.addAndMakeVisible(compReleaseSlider);
    compContent.addAndMakeVisible(compKneeSlider);
    compContent.addAndMakeVisible(compMakeupSlider);
    compContent.addAndMakeVisible(compMixSlider);
    compContent.addAndMakeVisible(compScHpfSlider);
    compContent.addAndMakeVisible(compStereoLinkSlider);
}
