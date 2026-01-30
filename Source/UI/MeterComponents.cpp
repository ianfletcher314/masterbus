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

    // Only draw level bar if there's actual signal (above threshold)
    if (displayLevel > noSignalThreshold)
    {
        float levelY = getYForDb(displayLevel);
        auto levelBounds = bounds.withTop(levelY);

        // Color gradient based on level - use gradient fill for smooth transition
        juce::ColourGradient gradient;
        gradient.isRadial = false;
        gradient.point1 = { bounds.getX(), bounds.getBottom() };
        gradient.point2 = { bounds.getX(), bounds.getY() };

        // Green at bottom, yellow in middle, red at top
        float yellowPos = ((-12.0f) - minDb) / (maxDb - minDb);
        float redPos = ((-3.0f) - minDb) / (maxDb - minDb);

        gradient.addColour(0.0, MasterBusLookAndFeel::Colors::meterGreen);
        gradient.addColour(yellowPos, MasterBusLookAndFeel::Colors::meterYellow);
        gradient.addColour(redPos, MasterBusLookAndFeel::Colors::meterRed);
        gradient.addColour(1.0, MasterBusLookAndFeel::Colors::meterRed);

        g.setGradientFill(gradient);
        g.fillRoundedRectangle(levelBounds, 2.0f);
    }

    // Peak hold line - only show if peak is above threshold
    if (peakHoldEnabled && displayPeak > noSignalThreshold)
    {
        float peakY = getYForDb(displayPeak);
        g.setColour(displayPeak > -3.0f ? MasterBusLookAndFeel::Colors::meterRed :
                    displayPeak > -12.0f ? MasterBusLookAndFeel::Colors::meterYellow :
                    MasterBusLookAndFeel::Colors::meterGreen);
        g.drawLine(bounds.getX() + 1.0f, peakY, bounds.getRight() - 1.0f, peakY, 2.0f);
    }

    // Scale markers with dB labels
    g.setFont(juce::Font(juce::FontOptions(8.0f)));
    float dbValues[] = { 0.0f, -6.0f, -12.0f, -24.0f, -36.0f, -48.0f };
    for (float db : dbValues)
    {
        if (db >= minDb && db <= maxDb)
        {
            float y = getYForDb(db);

            // Draw tick marks
            g.setColour(db == 0.0f ? MasterBusLookAndFeel::Colors::meterRed.withAlpha(0.5f) :
                        MasterBusLookAndFeel::Colors::gridLineMajor);
            g.drawLine(bounds.getX(), y, bounds.getX() + 4.0f, y, 1.0f);
            g.drawLine(bounds.getRight() - 4.0f, y, bounds.getRight(), y, 1.0f);
        }
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

    // GR bar (from top, grows downward) - only show when there's actual reduction
    if (displayGR > 0.1f)
    {
        float proportion = displayGR / maxGR;
        float barHeight = bounds.getHeight() * proportion;

        auto grBounds = bounds.withHeight(barHeight);

        // Use gradient: lighter orange at top (0dB), darker at bottom (max reduction)
        juce::ColourGradient gradient;
        gradient.isRadial = false;
        gradient.point1 = { bounds.getX(), bounds.getY() };
        gradient.point2 = { bounds.getX(), bounds.getY() + bounds.getHeight() };
        gradient.addColour(0.0, MasterBusLookAndFeel::Colors::compAccent.brighter(0.3f));
        gradient.addColour(0.5, MasterBusLookAndFeel::Colors::compAccent);
        gradient.addColour(1.0, MasterBusLookAndFeel::Colors::compAccent.darker(0.3f));

        g.setGradientFill(gradient);
        g.fillRoundedRectangle(grBounds, 2.0f);
    }

    // Scale markers with labels at key points
    float grValues[] = { 0.0f, 3.0f, 6.0f, 10.0f, 15.0f, 20.0f };
    for (float gr : grValues)
    {
        if (gr <= maxGR)
        {
            float proportion = gr / maxGR;
            float y = bounds.getHeight() * proportion;

            // Tick marks
            g.setColour(gr == 0.0f ? MasterBusLookAndFeel::Colors::gridLineMajor :
                        MasterBusLookAndFeel::Colors::gridLine);
            g.drawLine(bounds.getX(), y, bounds.getX() + 4.0f, y, 1.0f);
            g.drawLine(bounds.getRight() - 4.0f, y, bounds.getRight(), y, 1.0f);
        }
    }

    // Draw 0 indicator line at top when no reduction
    if (displayGR <= 0.1f)
    {
        g.setColour(MasterBusLookAndFeel::Colors::gridLineMajor);
        g.drawLine(bounds.getX() + 2.0f, bounds.getY() + 1.0f,
                   bounds.getRight() - 2.0f, bounds.getY() + 1.0f, 1.5f);
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

    // Check if there's actual signal (loudness above minimum threshold)
    bool hasSignal = momentaryLUFS > -70.0f;

    // Momentary bar - only draw if there's signal
    if (hasSignal && displayMomentary > -40.0f)
    {
        float momentaryY = getYForLufs(displayMomentary);
        auto momentaryBounds = meterBounds.withTop(momentaryY);

        // Gradient fill for momentary bar
        juce::ColourGradient gradient;
        gradient.isRadial = false;
        gradient.point1 = { meterBounds.getX(), meterBounds.getBottom() };
        gradient.point2 = { meterBounds.getX(), meterBounds.getY() };
        gradient.addColour(0.0, MasterBusLookAndFeel::Colors::accent.withAlpha(0.5f));
        gradient.addColour(0.7, MasterBusLookAndFeel::Colors::accent.withAlpha(0.7f));
        gradient.addColour(1.0, MasterBusLookAndFeel::Colors::accent);

        g.setGradientFill(gradient);
        g.fillRoundedRectangle(momentaryBounds, 1.0f);

        // Short-term line
        if (displayShortTerm > -40.0f)
        {
            float shortTermY = getYForLufs(displayShortTerm);
            g.setColour(MasterBusLookAndFeel::Colors::accent);
            g.drawLine(meterBounds.getX(), shortTermY, meterBounds.getRight(), shortTermY, 2.0f);
        }
    }

    // Target line - always show as reference
    float targetY = getYForLufs(targetLUFS);
    g.setColour(MasterBusLookAndFeel::Colors::meterYellow.withAlpha(0.6f));
    g.drawLine(bounds.getX() + 1.0f, targetY, meterBounds.getRight() + 2.0f, targetY, 1.5f);

    // Draw scale markers on meter
    g.setColour(MasterBusLookAndFeel::Colors::gridLine);
    float lufsMarkers[] = { 0.0f, -6.0f, -14.0f, -23.0f, -33.0f };
    for (float lufs : lufsMarkers)
    {
        float y = getYForLufs(lufs);
        g.drawLine(meterBounds.getX(), y, meterBounds.getX() + 3.0f, y);
    }

    // Text readouts
    float textX = meterWidth + 4.0f;
    float textWidth = bounds.getWidth() - textX - 2.0f;

    g.setFont(juce::Font(juce::FontOptions(9.0f)));

    // Integrated LUFS (most important - highlight)
    g.setColour(MasterBusLookAndFeel::Colors::textPrimary);
    juce::String intStr = integratedLUFS > -99.0f ?
        juce::String(integratedLUFS, 1) : "--.-";
    g.drawText("I:" + intStr, textX, 2.0f, textWidth, 13.0f, juce::Justification::left);

    // Short-term
    g.setColour(hasSignal ? MasterBusLookAndFeel::Colors::textSecondary : MasterBusLookAndFeel::Colors::textDim);
    juce::String stStr = shortTermLUFS > -99.0f ?
        juce::String(shortTermLUFS, 1) : "--.-";
    g.drawText("S:" + stStr, textX, 15.0f, textWidth, 13.0f, juce::Justification::left);

    // Momentary
    juce::String mStr = momentaryLUFS > -99.0f ?
        juce::String(momentaryLUFS, 1) : "--.-";
    g.drawText("M:" + mStr, textX, 28.0f, textWidth, 13.0f, juce::Justification::left);

    // Target reference
    g.setColour(MasterBusLookAndFeel::Colors::meterYellow.withAlpha(0.8f));
    g.drawText("T:" + juce::String(targetLUFS, 0), textX, 41.0f, textWidth, 13.0f, juce::Justification::left);

    // True Peak - at bottom, highlight if clipping
    bool isClipping = truePeakDb > -0.3f;
    g.setColour(isClipping ? MasterBusLookAndFeel::Colors::meterRed :
                (truePeakDb > -1.0f ? MasterBusLookAndFeel::Colors::meterYellow :
                 MasterBusLookAndFeel::Colors::textSecondary));
    juce::String tpStr = truePeakDb > -99.0f ?
        juce::String(truePeakDb, 1) : "--.-";
    g.drawText("TP:" + tpStr, textX, bounds.getHeight() - 14.0f, textWidth, 13.0f, juce::Justification::left);
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
    // When no signal, smoothly return to center (0)
    float target = hasSignal ? currentCorrelation : 0.0f;
    displayCorrelation = displayCorrelation * 0.9f + target * 0.1f;
    repaint();
}

void CorrelationMeter::setCorrelation(float correlation)
{
    currentCorrelation = std::clamp(correlation, -1.0f, 1.0f);
}

void CorrelationMeter::setHasSignal(bool signal)
{
    hasSignal = signal;
}

void CorrelationMeter::paint(juce::Graphics& g)
{
    auto bounds = getLocalBounds().toFloat();

    // Background
    g.setColour(MasterBusLookAndFeel::Colors::meterBackground);
    g.fillRoundedRectangle(bounds, 2.0f);

    // Center line (0) - always visible as reference
    float centerX = bounds.getCentreX();
    g.setColour(MasterBusLookAndFeel::Colors::gridLineMajor);
    g.drawLine(centerX, bounds.getY() + 2.0f, centerX, bounds.getBottom() - 2.0f, 1.5f);

    // Draw scale markers at -0.5 and +0.5
    float leftMark = centerX - (bounds.getWidth() * 0.225f);
    float rightMark = centerX + (bounds.getWidth() * 0.225f);
    g.setColour(MasterBusLookAndFeel::Colors::gridLine);
    g.drawLine(leftMark, bounds.getY() + 4.0f, leftMark, bounds.getBottom() - 4.0f);
    g.drawLine(rightMark, bounds.getY() + 4.0f, rightMark, bounds.getBottom() - 4.0f);

    // Only show colored indicator when there's signal
    if (hasSignal)
    {
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
    }
    else
    {
        // When no signal, show a dim center indicator
        g.setColour(MasterBusLookAndFeel::Colors::gridLine);
        g.fillRoundedRectangle(centerX - 2.0f, bounds.getY() + 3.0f, 4.0f, bounds.getHeight() - 6.0f, 1.0f);
    }

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
    // When no signal, smoothly return to center
    float target = hasSignal ? currentBalance : 0.0f;
    displayBalance = displayBalance * 0.85f + target * 0.15f;
    repaint();
}

void BalanceMeter::setBalance(float balance)
{
    currentBalance = std::clamp(balance, -1.0f, 1.0f);
}

void BalanceMeter::setHasSignal(bool signal)
{
    hasSignal = signal;
}

void BalanceMeter::paint(juce::Graphics& g)
{
    auto bounds = getLocalBounds().toFloat();

    // Background
    g.setColour(MasterBusLookAndFeel::Colors::meterBackground);
    g.fillRoundedRectangle(bounds, 2.0f);

    // Center line - always visible as reference
    float centerX = bounds.getCentreX();
    g.setColour(MasterBusLookAndFeel::Colors::gridLineMajor);
    g.drawLine(centerX, bounds.getY() + 2.0f, centerX, bounds.getBottom() - 2.0f, 1.5f);

    // Draw scale markers at 25% and 75%
    float leftMark = centerX - (bounds.getWidth() * 0.225f);
    float rightMark = centerX + (bounds.getWidth() * 0.225f);
    g.setColour(MasterBusLookAndFeel::Colors::gridLine);
    g.drawLine(leftMark, bounds.getY() + 4.0f, leftMark, bounds.getBottom() - 4.0f);
    g.drawLine(rightMark, bounds.getY() + 4.0f, rightMark, bounds.getBottom() - 4.0f);

    // Balance indicator - only show colored when there's signal
    float indicatorX = centerX + (displayBalance * bounds.getWidth() * 0.45f);

    if (hasSignal)
    {
        // Color based on how far off-center the balance is
        juce::Colour indicatorColour;
        float absBalance = std::abs(displayBalance);
        if (absBalance < 0.1f)
            indicatorColour = MasterBusLookAndFeel::Colors::meterGreen;  // Centered - good
        else if (absBalance < 0.3f)
            indicatorColour = MasterBusLookAndFeel::Colors::accent;  // Slight offset
        else
            indicatorColour = MasterBusLookAndFeel::Colors::meterYellow;  // Notable offset

        g.setColour(indicatorColour);
        g.fillRoundedRectangle(indicatorX - 3.0f, bounds.getY() + 2.0f, 6.0f, bounds.getHeight() - 4.0f, 2.0f);
    }
    else
    {
        // When no signal, show a dim center indicator
        g.setColour(MasterBusLookAndFeel::Colors::gridLine);
        g.fillRoundedRectangle(centerX - 2.0f, bounds.getY() + 3.0f, 4.0f, bounds.getHeight() - 6.0f, 1.0f);
    }

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

    // Configure labels with better styling
    for (auto* label : { &inputLabel, &outputLabel, &grLabel, &lufsLabel, &corrLabel, &balLabel })
    {
        label->setJustificationType(juce::Justification::centred);
        label->setColour(juce::Label::textColourId, MasterBusLookAndFeel::Colors::textPrimary);
        label->setFont(juce::Font(juce::FontOptions(10.0f).withStyle("Bold")));
        addAndMakeVisible(label);
    }

    // Add scale labels
    for (auto* label : { &inScaleLabel, &outScaleLabel, &grScaleLabel })
    {
        label->setJustificationType(juce::Justification::centred);
        label->setColour(juce::Label::textColourId, MasterBusLookAndFeel::Colors::textDim);
        label->setFont(juce::Font(juce::FontOptions(7.0f)));
        addAndMakeVisible(label);
    }
}

void MeterPanel::resized()
{
    auto bounds = getLocalBounds().reduced(6);
    int labelHeight = 14;
    int scaleHeight = 10;
    int meterSpacing = 4;
    int sectionSpacing = 8;

    // Top meters row (IN, OUT, GR)
    auto topRow = bounds.removeFromTop(bounds.getHeight() * 0.5f - sectionSpacing);
    int meterWidth = (topRow.getWidth() - meterSpacing * 2) / 3;

    auto inArea = topRow.removeFromLeft(meterWidth);
    inputLabel.setBounds(inArea.removeFromTop(labelHeight));
    inScaleLabel.setBounds(inArea.removeFromBottom(scaleHeight));
    inputMeter.setBounds(inArea.reduced(2, 0));

    topRow.removeFromLeft(meterSpacing);
    auto outArea = topRow.removeFromLeft(meterWidth);
    outputLabel.setBounds(outArea.removeFromTop(labelHeight));
    outScaleLabel.setBounds(outArea.removeFromBottom(scaleHeight));
    outputMeter.setBounds(outArea.reduced(2, 0));

    topRow.removeFromLeft(meterSpacing);
    auto grArea = topRow;
    grLabel.setBounds(grArea.removeFromTop(labelHeight));
    grScaleLabel.setBounds(grArea.removeFromBottom(scaleHeight));
    grMeter.setBounds(grArea.reduced(2, 0));

    bounds.removeFromTop(sectionSpacing);

    // Bottom row (LUFS, Correlation, Balance)
    auto bottomRow = bounds;
    int lufsWidth = bottomRow.getWidth() * 0.5f;

    auto lufsArea = bottomRow.removeFromLeft(lufsWidth);
    lufsLabel.setBounds(lufsArea.removeFromTop(labelHeight));
    loudnessMeter.setBounds(lufsArea.reduced(2, 2));

    bottomRow.removeFromLeft(meterSpacing);

    auto smallMetersArea = bottomRow;
    int smallMeterHeight = (smallMetersArea.getHeight() - labelHeight * 2 - meterSpacing) / 2;

    corrLabel.setBounds(smallMetersArea.removeFromTop(labelHeight));
    correlationMeter.setBounds(smallMetersArea.removeFromTop(smallMeterHeight).reduced(0, 1));
    smallMetersArea.removeFromTop(meterSpacing);
    balLabel.setBounds(smallMetersArea.removeFromTop(labelHeight));
    balanceMeter.setBounds(smallMetersArea.reduced(0, 1));
}

void MeterPanel::paint(juce::Graphics& g)
{
    auto bounds = getLocalBounds().toFloat();

    // Main panel background
    g.setColour(MasterBusLookAndFeel::Colors::sectionBackground);
    g.fillRoundedRectangle(bounds, 6.0f);

    // Panel border
    g.setColour(MasterBusLookAndFeel::Colors::gridLine);
    g.drawRoundedRectangle(bounds.reduced(0.5f), 6.0f, 1.0f);

    // Section divider between top and bottom meters
    auto innerBounds = bounds.reduced(6);
    float dividerY = innerBounds.getY() + innerBounds.getHeight() * 0.5f - 2.0f;
    g.setColour(MasterBusLookAndFeel::Colors::gridLine.withAlpha(0.5f));
    g.drawLine(innerBounds.getX() + 4.0f, dividerY,
               innerBounds.getRight() - 4.0f, dividerY, 1.0f);

    // Draw "METERS" header label at very top
    g.setColour(MasterBusLookAndFeel::Colors::textDim);
    g.setFont(juce::Font(juce::FontOptions(8.0f)));
    g.drawText("METERS", bounds.getX(), bounds.getY() + 1.0f, bounds.getWidth(), 10.0f, juce::Justification::centred);
}
