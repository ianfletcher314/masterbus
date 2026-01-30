#pragma once

#include <JuceHeader.h>
#include "LookAndFeel.h"

//==============================================================================
// Vertical level meter with peak hold and color zones
class LevelMeter : public juce::Component, public juce::Timer
{
public:
    LevelMeter();
    ~LevelMeter() override;

    void paint(juce::Graphics& g) override;
    void timerCallback() override;

    void setLevel(float levelDb);
    void setPeakLevel(float peakDb);
    void setRange(float minDb, float maxDb);
    void showPeakHold(bool show);

private:
    float currentLevel = -100.0f;
    float displayLevel = -100.0f;
    float peakLevel = -100.0f;
    float displayPeak = -100.0f;
    float minDb = -60.0f;
    float maxDb = 6.0f;
    bool peakHoldEnabled = true;

    float peakHoldTime = 2.0f; // seconds
    float peakDecayRate = 20.0f; // dB per second
    float timeSincePeak = 0.0f;

    float getYForDb(float db) const;
};

//==============================================================================
// Gain reduction meter (shows compression amount)
class GainReductionMeter : public juce::Component, public juce::Timer
{
public:
    GainReductionMeter();
    ~GainReductionMeter() override;

    void paint(juce::Graphics& g) override;
    void timerCallback() override;

    void setGainReduction(float grDb);
    void setRange(float minGR, float maxGR);

private:
    float currentGR = 0.0f;
    float displayGR = 0.0f;
    float minGR = 0.0f;
    float maxGR = 20.0f;
};

//==============================================================================
// LUFS meter with target line
class LoudnessMeterDisplay : public juce::Component, public juce::Timer
{
public:
    LoudnessMeterDisplay();
    ~LoudnessMeterDisplay() override;

    void paint(juce::Graphics& g) override;
    void timerCallback() override;

    void setMomentary(float lufs);
    void setShortTerm(float lufs);
    void setIntegrated(float lufs);
    void setTarget(float targetLufs);
    void setTruePeak(float truePeakDb);

private:
    float momentaryLUFS = -100.0f;
    float shortTermLUFS = -100.0f;
    float integratedLUFS = -100.0f;
    float truePeakDb = -100.0f;
    float targetLUFS = -14.0f;

    float displayMomentary = -100.0f;
    float displayShortTerm = -100.0f;

    float getYForLufs(float lufs) const;
};

//==============================================================================
// Stereo correlation meter (-1 to +1)
class CorrelationMeter : public juce::Component, public juce::Timer
{
public:
    CorrelationMeter();
    ~CorrelationMeter() override;

    void paint(juce::Graphics& g) override;
    void timerCallback() override;

    void setCorrelation(float correlation);

private:
    float currentCorrelation = 1.0f;
    float displayCorrelation = 1.0f;
};

//==============================================================================
// Stereo balance meter
class BalanceMeter : public juce::Component, public juce::Timer
{
public:
    BalanceMeter();
    ~BalanceMeter() override;

    void paint(juce::Graphics& g) override;
    void timerCallback() override;

    void setBalance(float balance); // -1 = left, +1 = right

private:
    float currentBalance = 0.0f;
    float displayBalance = 0.0f;
};

//==============================================================================
// Combined meter section panel
class MeterPanel : public juce::Component
{
public:
    MeterPanel();
    void resized() override;
    void paint(juce::Graphics& g) override;

    LevelMeter& getInputMeter() { return inputMeter; }
    LevelMeter& getOutputMeter() { return outputMeter; }
    GainReductionMeter& getGRMeter() { return grMeter; }
    LoudnessMeterDisplay& getLoudnessMeter() { return loudnessMeter; }
    CorrelationMeter& getCorrelationMeter() { return correlationMeter; }
    BalanceMeter& getBalanceMeter() { return balanceMeter; }

private:
    LevelMeter inputMeter;
    LevelMeter outputMeter;
    GainReductionMeter grMeter;
    LoudnessMeterDisplay loudnessMeter;
    CorrelationMeter correlationMeter;
    BalanceMeter balanceMeter;

    juce::Label inputLabel { {}, "IN" };
    juce::Label outputLabel { {}, "OUT" };
    juce::Label grLabel { {}, "GR" };
    juce::Label lufsLabel { {}, "LUFS" };
    juce::Label corrLabel { {}, "CORR" };
    juce::Label balLabel { {}, "BAL" };
};
