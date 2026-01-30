#include "MeterComponents.h"

//==============================================================================
// LevelMeter
//==============================================================================
LevelMeter::LevelMeter()
{
    startTimerHz(30);
}

LevelMeter::~LevelMeter()
{
    stopTimer();
}

void LevelMeter::timerCallback()
{
    // Smooth level display
    float targetLevel = currentLevel;
    if (targetLevel > displayLevel)
        displayLevel = targetLevel;
    else
        displayLevel = displayLevel * 0.9f + targetLevel * 0.1f;

    // Peak hold and decay
    if (peakHoldEnabled)
    {
        timeSincePeak += 1.0f / 30.0f;
        if (timeSincePeak > peakHoldTime)
        {
            displayPeak -= peakDecayRate / 30.0f;
            if (displayPeak < displayLevel)
                displayPeak = displayLevel;
        }
    }

    repaint();
}

void LevelMeter::setLevel(float levelDb)
{
    currentLevel = levelDb;
}

void LevelMeter::setPeakLevel(float peakDb)
{
    if (peakDb > displayPeak)
    {
        peakLevel = peakDb;
        displayPeak = peakDb;
        timeSincePeak = 0.0f;
    }
}

void LevelMeter::setRange(float min, float max)
{
    minDb = min;
    maxDb = max;
}

void LevelMeter::showPeakHold(bool show)
{
    peakHoldEnabled = show;
}

float LevelMeter::getYForDb(float db) const
{
    float clampedDb = std::clamp(db, minDb, maxDb);
    float proportion = (clampedDb - minDb) / (maxDb - minDb);
    return getHeight() * (1.0f - proportion);
}

void LevelMeter::paint(juce::Graphics& g)
{
    auto bounds = getLocalBounds().toFloat();

    // Background
    g.setColour(MasterBusLookAndFeel::Colors::meterBackground);
    g.fillRoundedRectangle(bounds, 2.0f);

    // Level bar
    float levelY = getYForDb(displayLevel);
    auto levelBounds = bounds.withTop(levelY);

    // Color gradient based on level
    juce::Colour levelColour;
    if (displayLevel > -3.0f)
        levelColour = MasterBusLookAndFeel::Colors::meterRed;
    else if (displayLevel > -12.0f)
        levelColour = MasterBusLookAndFeel::Colors::meterYellow;
    else
        levelColour = MasterBusLookAndFeel::Colors::meterGreen;

    g.setColour(levelColour);
    g.fillRoundedRectangle(levelBounds, 2.0f);

    // Peak hold line
    if (peakHoldEnabled && displayPeak > minDb)
    {
        float peakY = getYForDb(displayPeak);
        g.setColour(MasterBusLookAndFeel::Colors::meterRed);
        g.drawLine(bounds.getX() + 1.0f, peakY, bounds.getRight() - 1.0f, peakY, 2.0f);
    }

    // Scale markers
    g.setColour(MasterBusLookAndFeel::Colors::gridLine);
    for (float db = minDb; db <= maxDb; db += 12.0f)
    {
        float y = getYForDb(db);
        g.drawLine(bounds.getX(), y, bounds.getX() + 3.0f, y);
        g.drawLine(bounds.getRight() - 3.0f, y, bounds.getRight(), y);
    }
}

//==============================================================================
// GainReductionMeter
//==============================================================================
GainReductionMeter::GainReductionMeter()
{
    startTimerHz(30);
}

GainReductionMeter::~GainReductionMeter()
{
    stopTimer();
}

void GainReductionMeter::timerCallback()
{
    // Smooth display
    float diff = currentGR - displayGR;
    if (std::abs(diff) > 0.1f)
        displayGR += diff * 0.3f;
    else
        displayGR = currentGR;

    repaint();
}

void GainReductionMeter::setGainReduction(float grDb)
{
    currentGR = std::clamp(grDb, minGR, maxGR);
}

void GainReductionMeter::setRange(float min, float max)
{
    minGR = min;
    maxGR = max;
}

void GainReductionMeter::paint(juce::Graphics& g)
{
    auto bounds = getLocalBounds().toFloat();

    // Background
    g.setColour(MasterBusLookAndFeel::Colors::meterBackground);
    g.fillRoundedRectangle(bounds, 2.0f);

    // GR bar (from top, grows downward)
    if (displayGR > 0.01f)
    {
        float proportion = displayGR / maxGR;
        float barHeight = bounds.getHeight() * proportion;

        auto grBounds = bounds.withHeight(barHeight);
        g.setColour(MasterBusLookAndFeel::Colors::compAccent);
        g.fillRoundedRectangle(grBounds, 2.0f);
    }

    // Scale markers
    g.setColour(MasterBusLookAndFeel::Colors::gridLine);
    for (float gr = 0.0f; gr <= maxGR; gr += 5.0f)
    {
        float proportion = gr / maxGR;
        float y = bounds.getHeight() * proportion;
        g.drawLine(bounds.getX(), y, bounds.getX() + 3.0f, y);
        g.drawLine(bounds.getRight() - 3.0f, y, bounds.getRight(), y);
    }
}

//==============================================================================
// LoudnessMeterDisplay
//==============================================================================
LoudnessMeterDisplay::LoudnessMeterDisplay()
{
    startTimerHz(10);
}

LoudnessMeterDisplay::~LoudnessMeterDisplay()
{
    stopTimer();
}

void LoudnessMeterDisplay::timerCallback()
{
    // Smooth display values
    displayMomentary = displayMomentary * 0.7f + momentaryLUFS * 0.3f;
    displayShortTerm = displayShortTerm * 0.8f + shortTermLUFS * 0.2f;
    repaint();
}

void LoudnessMeterDisplay::setMomentary(float lufs)
{
    momentaryLUFS = lufs;
}

void LoudnessMeterDisplay::setShortTerm(float lufs)
{
    shortTermLUFS = lufs;
}

void LoudnessMeterDisplay::setIntegrated(float lufs)
{
    integratedLUFS = lufs;
}

void LoudnessMeterDisplay::setTarget(float target)
{
    targetLUFS = target;
}

void LoudnessMeterDisplay::setTruePeak(float tp)
{
    truePeakDb = tp;
}

float LoudnessMeterDisplay::getYForLufs(float lufs) const
{
    float minLufs = -40.0f;
    float maxLufs = 0.0f;
    float clampedLufs = std::clamp(lufs, minLufs, maxLufs);
    float proportion = (clampedLufs - minLufs) / (maxLufs - minLufs);
    return getHeight() * (1.0f - proportion);
}

void LoudnessMeterDisplay::paint(juce::Graphics& g)
{
    auto bounds = getLocalBounds().toFloat();

    // Background
    g.setColour(MasterBusLookAndFeel::Colors::meterBackground);
    g.fillRoundedRectangle(bounds, 2.0f);

    // Meter area
    float meterWidth = bounds.getWidth() * 0.4f;
    auto meterBounds = bounds.withWidth(meterWidth).reduced(2.0f, 2.0f);

    // Momentary bar
    float momentaryY = getYForLufs(displayMomentary);
    auto momentaryBounds = meterBounds.withTop(momentaryY);
    g.setColour(MasterBusLookAndFeel::Colors::accent.withAlpha(0.7f));
    g.fillRoundedRectangle(momentaryBounds, 1.0f);

    // Short-term line
    float shortTermY = getYForLufs(displayShortTerm);
    g.setColour(MasterBusLookAndFeel::Colors::accent);
    g.drawLine(meterBounds.getX(), shortTermY, meterBounds.getRight(), shortTermY, 2.0f);

    // Target line
    float targetY = getYForLufs(targetLUFS);
    g.setColour(MasterBusLookAndFeel::Colors::meterYellow.withAlpha(0.5f));
    g.drawLine(bounds.getX(), targetY, bounds.getRight(), targetY, 1.0f);

    // Text readouts
    float textX = meterWidth + 4.0f;
    float textWidth = bounds.getWidth() - textX - 2.0f;

    g.setFont(juce::Font(juce::FontOptions(10.0f)));

    // Integrated LUFS
    g.setColour(MasterBusLookAndFeel::Colors::textPrimary);
    juce::String intStr = integratedLUFS > -99.0f ?
        juce::String(integratedLUFS, 1) : "--.--";
    g.drawText("I:" + intStr, textX, 2.0f, textWidth, 14.0f, juce::Justification::left);

    // Short-term
    g.setColour(MasterBusLookAndFeel::Colors::textSecondary);
    juce::String stStr = shortTermLUFS > -99.0f ?
        juce::String(shortTermLUFS, 1) : "--.--";
    g.drawText("S:" + stStr, textX, 16.0f, textWidth, 14.0f, juce::Justification::left);

    // Momentary
    juce::String mStr = momentaryLUFS > -99.0f ?
        juce::String(momentaryLUFS, 1) : "--.--";
    g.drawText("M:" + mStr, textX, 30.0f, textWidth, 14.0f, juce::Justification::left);

    // True Peak
    g.setColour(truePeakDb > -1.0f ? MasterBusLookAndFeel::Colors::meterRed : MasterBusLookAndFeel::Colors::textSecondary);
    juce::String tpStr = truePeakDb > -99.0f ?
        juce::String(truePeakDb, 1) : "--.--";
    g.drawText("TP:" + tpStr, textX, bounds.getHeight() - 16.0f, textWidth, 14.0f, juce::Justification::left);
}

//==============================================================================
// CorrelationMeter
//==============================================================================
CorrelationMeter::CorrelationMeter()
{
    startTimerHz(30);
}

CorrelationMeter::~CorrelationMeter()
{
    stopTimer();
}

void CorrelationMeter::timerCallback()
{
    displayCorrelation = displayCorrelation * 0.9f + currentCorrelation * 0.1f;
    repaint();
}

void CorrelationMeter::setCorrelation(float correlation)
{
    currentCorrelation = std::clamp(correlation, -1.0f, 1.0f);
}

void CorrelationMeter::paint(juce::Graphics& g)
{
    auto bounds = getLocalBounds().toFloat();

    // Background
    g.setColour(MasterBusLookAndFeel::Colors::meterBackground);
    g.fillRoundedRectangle(bounds, 2.0f);

    // Center line (0)
    float centerX = bounds.getCentreX();
    g.setColour(MasterBusLookAndFeel::Colors::gridLine);
    g.drawLine(centerX, bounds.getY() + 2.0f, centerX, bounds.getBottom() - 2.0f);

    // Correlation indicator
    float indicatorX = centerX + (displayCorrelation * bounds.getWidth() * 0.45f);

    juce::Colour indicatorColour;
    if (displayCorrelation > 0.5f)
        indicatorColour = MasterBusLookAndFeel::Colors::meterGreen;
    else if (displayCorrelation > 0.0f)
        indicatorColour = MasterBusLookAndFeel::Colors::meterYellow;
    else
        indicatorColour = MasterBusLookAndFeel::Colors::meterRed;

    g.setColour(indicatorColour);
    g.fillRoundedRectangle(indicatorX - 3.0f, bounds.getY() + 2.0f, 6.0f, bounds.getHeight() - 4.0f, 2.0f);

    // Labels
    g.setColour(MasterBusLookAndFeel::Colors::textDim);
    g.setFont(juce::Font(juce::FontOptions(8.0f)));
    g.drawText("-1", bounds.getX() + 2.0f, bounds.getCentreY() - 5.0f, 12.0f, 10.0f, juce::Justification::centred);
    g.drawText("+1", bounds.getRight() - 14.0f, bounds.getCentreY() - 5.0f, 12.0f, 10.0f, juce::Justification::centred);
}

//==============================================================================
// BalanceMeter
//==============================================================================
BalanceMeter::BalanceMeter()
{
    startTimerHz(30);
}

BalanceMeter::~BalanceMeter()
{
    stopTimer();
}

void BalanceMeter::timerCallback()
{
    displayBalance = displayBalance * 0.85f + currentBalance * 0.15f;
    repaint();
}

void BalanceMeter::setBalance(float balance)
{
    currentBalance = std::clamp(balance, -1.0f, 1.0f);
}

void BalanceMeter::paint(juce::Graphics& g)
{
    auto bounds = getLocalBounds().toFloat();

    // Background
    g.setColour(MasterBusLookAndFeel::Colors::meterBackground);
    g.fillRoundedRectangle(bounds, 2.0f);

    // Center line
    float centerX = bounds.getCentreX();
    g.setColour(MasterBusLookAndFeel::Colors::gridLine);
    g.drawLine(centerX, bounds.getY() + 2.0f, centerX, bounds.getBottom() - 2.0f);

    // Balance indicator
    float indicatorX = centerX + (displayBalance * bounds.getWidth() * 0.45f);
    g.setColour(MasterBusLookAndFeel::Colors::accent);
    g.fillRoundedRectangle(indicatorX - 3.0f, bounds.getY() + 2.0f, 6.0f, bounds.getHeight() - 4.0f, 2.0f);

    // L/R labels
    g.setColour(MasterBusLookAndFeel::Colors::textDim);
    g.setFont(juce::Font(juce::FontOptions(8.0f)));
    g.drawText("L", bounds.getX() + 2.0f, bounds.getCentreY() - 5.0f, 10.0f, 10.0f, juce::Justification::centred);
    g.drawText("R", bounds.getRight() - 12.0f, bounds.getCentreY() - 5.0f, 10.0f, 10.0f, juce::Justification::centred);
}

//==============================================================================
// MeterPanel
//==============================================================================
MeterPanel::MeterPanel()
{
    addAndMakeVisible(inputMeter);
    addAndMakeVisible(outputMeter);
    addAndMakeVisible(grMeter);
    addAndMakeVisible(loudnessMeter);
    addAndMakeVisible(correlationMeter);
    addAndMakeVisible(balanceMeter);

    for (auto* label : { &inputLabel, &outputLabel, &grLabel, &lufsLabel, &corrLabel, &balLabel })
    {
        label->setJustificationType(juce::Justification::centred);
        label->setColour(juce::Label::textColourId, MasterBusLookAndFeel::Colors::textSecondary);
        label->setFont(juce::Font(juce::FontOptions(10.0f)));
        addAndMakeVisible(label);
    }
}

void MeterPanel::resized()
{
    auto bounds = getLocalBounds().reduced(5);
    int labelHeight = 15;
    int meterSpacing = 5;

    // Top meters row (IN, OUT, GR)
    auto topRow = bounds.removeFromTop(bounds.getHeight() * 0.5f - 5);
    int meterWidth = (topRow.getWidth() - meterSpacing * 2) / 3;

    auto inArea = topRow.removeFromLeft(meterWidth);
    inputLabel.setBounds(inArea.removeFromTop(labelHeight));
    inputMeter.setBounds(inArea);

    topRow.removeFromLeft(meterSpacing);
    auto outArea = topRow.removeFromLeft(meterWidth);
    outputLabel.setBounds(outArea.removeFromTop(labelHeight));
    outputMeter.setBounds(outArea);

    topRow.removeFromLeft(meterSpacing);
    auto grArea = topRow;
    grLabel.setBounds(grArea.removeFromTop(labelHeight));
    grMeter.setBounds(grArea);

    bounds.removeFromTop(5);

    // Bottom row (LUFS, Correlation, Balance)
    auto bottomRow = bounds;
    int lufsWidth = bottomRow.getWidth() * 0.5f;

    auto lufsArea = bottomRow.removeFromLeft(lufsWidth);
    lufsLabel.setBounds(lufsArea.removeFromTop(labelHeight));
    loudnessMeter.setBounds(lufsArea);

    bottomRow.removeFromLeft(meterSpacing);

    auto smallMetersArea = bottomRow;
    int smallMeterHeight = (smallMetersArea.getHeight() - labelHeight * 2 - meterSpacing) / 2;

    corrLabel.setBounds(smallMetersArea.removeFromTop(labelHeight));
    correlationMeter.setBounds(smallMetersArea.removeFromTop(smallMeterHeight));
    smallMetersArea.removeFromTop(meterSpacing);
    balLabel.setBounds(smallMetersArea.removeFromTop(labelHeight));
    balanceMeter.setBounds(smallMetersArea);
}

void MeterPanel::paint(juce::Graphics& g)
{
    auto bounds = getLocalBounds().toFloat();
    g.setColour(MasterBusLookAndFeel::Colors::sectionBackground);
    g.fillRoundedRectangle(bounds, 6.0f);

    g.setColour(MasterBusLookAndFeel::Colors::gridLine);
    g.drawRoundedRectangle(bounds.reduced(1.0f), 6.0f, 1.0f);
}
