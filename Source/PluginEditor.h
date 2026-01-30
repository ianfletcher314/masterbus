#pragma once

#include <JuceHeader.h>
#include "PluginProcessor.h"
#include "UI/LookAndFeel.h"
#include "UI/SpectrumAnalyzer.h"
#include "UI/MeterComponents.h"

class MasterBusAudioProcessorEditor : public juce::AudioProcessorEditor,
                                       public juce::Timer
{
public:
    MasterBusAudioProcessorEditor(MasterBusAudioProcessor&);
    ~MasterBusAudioProcessorEditor() override;

    void paint(juce::Graphics&) override;
    void resized() override;
    void timerCallback() override;

private:
    MasterBusAudioProcessor& audioProcessor;

    // Look and feel
    MasterBusLookAndFeel mainLookAndFeel;
    EQLookAndFeel eqLookAndFeel;
    CompressorLookAndFeel compLookAndFeel;

    // Spectrum analyzer
    SpectrumAnalyzer spectrumAnalyzer;
    juce::ToggleButton preButton { "Pre" };
    juce::ToggleButton postButton { "Post" };
    juce::ComboBox slopeSelector;

    // Meter panel
    MeterPanel meterPanel;

    // A/B/C/D buttons
    std::array<juce::TextButton, 4> abcdButtons;

    // Preset selector
    juce::ComboBox presetSelector;
    juce::TextButton saveButton { "Save" };

    // EQ Section
    juce::GroupComponent eqGroup;

    // HPF controls
    juce::Slider hpfFreqSlider;
    juce::ComboBox hpfSlopeBox;
    juce::ToggleButton hpfButton { "HPF" };
    juce::Label hpfLabel { {}, "HPF" };

    // LPF controls
    juce::Slider lpfFreqSlider;
    juce::ComboBox lpfSlopeBox;
    juce::ToggleButton lpfButton { "LPF" };
    juce::Label lpfLabel { {}, "LPF" };

    // Low Shelf
    juce::Slider lsFreqSlider, lsGainSlider;
    juce::ToggleButton lsButton { "LS" };
    juce::Label lsFreqLabel { {}, "Freq" };
    juce::Label lsGainLabel { {}, "Gain" };

    // High Shelf
    juce::Slider hsFreqSlider, hsGainSlider;
    juce::ToggleButton hsButton { "HS" };
    juce::Label hsFreqLabel { {}, "Freq" };
    juce::Label hsGainLabel { {}, "Gain" };

    // Parametric bands
    struct BandControls
    {
        juce::Slider freqSlider, gainSlider, qSlider;
        juce::ToggleButton enableButton;
        juce::Label freqLabel { {}, "Freq" };
        juce::Label gainLabel { {}, "Gain" };
        juce::Label qLabel { {}, "Q" };
    };
    std::array<BandControls, 4> bandControls;

    // EQ options
    juce::ToggleButton eqLinearPhaseButton { "Linear Phase" };
    juce::ToggleButton eqMidSideButton { "M/S" };
    juce::ToggleButton eqBypassButton { "Bypass" };

    // Compressor Section
    juce::GroupComponent compGroup;

    juce::Slider compThresholdSlider, compRatioSlider, compAttackSlider;
    juce::Slider compReleaseSlider, compKneeSlider, compMakeupSlider;
    juce::Slider compMixSlider, compScHpfSlider, compStereoLinkSlider;

    juce::Label compThreshLabel { {}, "Thresh" };
    juce::Label compRatioLabel { {}, "Ratio" };
    juce::Label compAttackLabel { {}, "Attack" };
    juce::Label compReleaseLabel { {}, "Release" };
    juce::Label compKneeLabel { {}, "Knee" };
    juce::Label compMakeupLabel { {}, "Makeup" };
    juce::Label compMixLabel { {}, "Mix" };
    juce::Label compScHpfLabel { {}, "SC HPF" };
    juce::Label compLinkLabel { {}, "Link" };

    juce::ComboBox compModeBox;
    juce::ToggleButton compAutoReleaseButton { "Auto Rel" };
    juce::ToggleButton compScListenButton { "SC Listen" };
    juce::ToggleButton compMidSideButton { "M/S" };
    juce::ToggleButton compBypassButton { "Bypass" };

    // Output
    juce::Slider outputGainSlider;
    juce::Label outputGainLabel { {}, "Output" };
    juce::ToggleButton globalBypassButton { "BYPASS" };
    juce::ToggleButton monoButton { "Mono" };
    juce::ToggleButton dimButton { "Dim" };

    // Parameter attachments
    // HPF
    std::unique_ptr<juce::AudioProcessorValueTreeState::SliderAttachment> hpfFreqAttachment;
    std::unique_ptr<juce::AudioProcessorValueTreeState::ComboBoxAttachment> hpfSlopeAttachment;
    std::unique_ptr<juce::AudioProcessorValueTreeState::ButtonAttachment> hpfEnabledAttachment;

    // LPF
    std::unique_ptr<juce::AudioProcessorValueTreeState::SliderAttachment> lpfFreqAttachment;
    std::unique_ptr<juce::AudioProcessorValueTreeState::ComboBoxAttachment> lpfSlopeAttachment;
    std::unique_ptr<juce::AudioProcessorValueTreeState::ButtonAttachment> lpfEnabledAttachment;

    // Low Shelf
    std::unique_ptr<juce::AudioProcessorValueTreeState::SliderAttachment> lsFreqAttachment;
    std::unique_ptr<juce::AudioProcessorValueTreeState::SliderAttachment> lsGainAttachment;
    std::unique_ptr<juce::AudioProcessorValueTreeState::ButtonAttachment> lsEnabledAttachment;

    // High Shelf
    std::unique_ptr<juce::AudioProcessorValueTreeState::SliderAttachment> hsFreqAttachment;
    std::unique_ptr<juce::AudioProcessorValueTreeState::SliderAttachment> hsGainAttachment;
    std::unique_ptr<juce::AudioProcessorValueTreeState::ButtonAttachment> hsEnabledAttachment;

    // Bands
    struct BandAttachments
    {
        std::unique_ptr<juce::AudioProcessorValueTreeState::SliderAttachment> freq;
        std::unique_ptr<juce::AudioProcessorValueTreeState::SliderAttachment> gain;
        std::unique_ptr<juce::AudioProcessorValueTreeState::SliderAttachment> q;
        std::unique_ptr<juce::AudioProcessorValueTreeState::ButtonAttachment> enabled;
    };
    std::array<BandAttachments, 4> bandAttachments;

    // EQ Global
    std::unique_ptr<juce::AudioProcessorValueTreeState::ButtonAttachment> eqLinearPhaseAttachment;
    std::unique_ptr<juce::AudioProcessorValueTreeState::ButtonAttachment> eqMidSideAttachment;
    std::unique_ptr<juce::AudioProcessorValueTreeState::ButtonAttachment> eqBypassAttachment;

    // Compressor
    std::unique_ptr<juce::AudioProcessorValueTreeState::SliderAttachment> compThresholdAttachment;
    std::unique_ptr<juce::AudioProcessorValueTreeState::SliderAttachment> compRatioAttachment;
    std::unique_ptr<juce::AudioProcessorValueTreeState::SliderAttachment> compAttackAttachment;
    std::unique_ptr<juce::AudioProcessorValueTreeState::SliderAttachment> compReleaseAttachment;
    std::unique_ptr<juce::AudioProcessorValueTreeState::SliderAttachment> compKneeAttachment;
    std::unique_ptr<juce::AudioProcessorValueTreeState::SliderAttachment> compMakeupAttachment;
    std::unique_ptr<juce::AudioProcessorValueTreeState::SliderAttachment> compMixAttachment;
    std::unique_ptr<juce::AudioProcessorValueTreeState::SliderAttachment> compScHpfAttachment;
    std::unique_ptr<juce::AudioProcessorValueTreeState::SliderAttachment> compStereoLinkAttachment;
    std::unique_ptr<juce::AudioProcessorValueTreeState::ComboBoxAttachment> compModeAttachment;
    std::unique_ptr<juce::AudioProcessorValueTreeState::ButtonAttachment> compAutoReleaseAttachment;
    std::unique_ptr<juce::AudioProcessorValueTreeState::ButtonAttachment> compScListenAttachment;
    std::unique_ptr<juce::AudioProcessorValueTreeState::ButtonAttachment> compMidSideAttachment;
    std::unique_ptr<juce::AudioProcessorValueTreeState::ButtonAttachment> compBypassAttachment;

    // Output
    std::unique_ptr<juce::AudioProcessorValueTreeState::SliderAttachment> outputGainAttachment;
    std::unique_ptr<juce::AudioProcessorValueTreeState::ButtonAttachment> globalBypassAttachment;

    void setupSlider(juce::Slider& slider, juce::Label& label, const juce::String& suffix = "");
    void setupRotarySlider(juce::Slider& slider);

    JUCE_DECLARE_NON_COPYABLE_WITH_LEAK_DETECTOR(MasterBusAudioProcessorEditor)
};
