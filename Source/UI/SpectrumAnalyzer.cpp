#include "SpectrumAnalyzer.h"
#include "../DSP/DSPUtils.h"

SpectrumAnalyzer::SpectrumAnalyzer()
{
    sampleBuffer.resize(FFT_SIZE, 0.0f);
    fftData.resize(FFT_SIZE * 2, 0.0f);
    preBuffer.resize(FFT_SIZE, 0.0f);
    postBuffer.resize(FFT_SIZE, 0.0f);

    magnitudes.fill(-100.0f);
    smoothedMagnitudes.fill(-100.0f);
    peakMagnitudes.fill(-100.0f);
    preMagnitudes.fill(-100.0f);
    postMagnitudes.fill(-100.0f);
    smoothedPreMagnitudes.fill(-100.0f);
    smoothedPostMagnitudes.fill(-100.0f);

    startTimerHz(30); // 30fps update
}

SpectrumAnalyzer::~SpectrumAnalyzer()
{
    stopTimer();
}

void SpectrumAnalyzer::pushBuffer(const juce::AudioBuffer<float>& buffer)
{
    if (buffer.getNumChannels() < 1) return;

    const int numSamples = buffer.getNumSamples();
    const float* data = buffer.getReadPointer(0);

    // Mix to mono if stereo
    std::vector<float> monoData(numSamples);
    if (buffer.getNumChannels() > 1)
    {
        const float* right = buffer.getReadPointer(1);
        for (int i = 0; i < numSamples; ++i)
            monoData[i] = (data[i] + right[i]) * 0.5f;
        data = monoData.data();
    }

    // Push samples to circular buffer
    int currentWrite = writeIndex.load();
    for (int i = 0; i < numSamples; ++i)
    {
        sampleBuffer[currentWrite] = data[i];
        currentWrite = (currentWrite + 1) % FFT_SIZE;
    }
    writeIndex.store(currentWrite);
}

void SpectrumAnalyzer::pushPreBuffer(const juce::AudioBuffer<float>& buffer)
{
    if (buffer.getNumChannels() < 1) return;

    const int numSamples = buffer.getNumSamples();
    const float* data = buffer.getReadPointer(0);

    std::vector<float> monoData(numSamples);
    if (buffer.getNumChannels() > 1)
    {
        const float* right = buffer.getReadPointer(1);
        for (int i = 0; i < numSamples; ++i)
            monoData[i] = (data[i] + right[i]) * 0.5f;
        data = monoData.data();
    }

    int currentWrite = preWriteIndex.load();
    for (int i = 0; i < numSamples; ++i)
    {
        preBuffer[currentWrite] = data[i];
        currentWrite = (currentWrite + 1) % FFT_SIZE;
    }
    preWriteIndex.store(currentWrite);
}

void SpectrumAnalyzer::pushPostBuffer(const juce::AudioBuffer<float>& buffer)
{
    if (buffer.getNumChannels() < 1) return;

    const int numSamples = buffer.getNumSamples();
    const float* data = buffer.getReadPointer(0);

    std::vector<float> monoData(numSamples);
    if (buffer.getNumChannels() > 1)
    {
        const float* right = buffer.getReadPointer(1);
        for (int i = 0; i < numSamples; ++i)
            monoData[i] = (data[i] + right[i]) * 0.5f;
        data = monoData.data();
    }

    int currentWrite = postWriteIndex.load();
    for (int i = 0; i < numSamples; ++i)
    {
        postBuffer[currentWrite] = data[i];
        currentWrite = (currentWrite + 1) % FFT_SIZE;
    }
    postWriteIndex.store(currentWrite);
}

void SpectrumAnalyzer::processFFT(const std::vector<float>& buffer, std::array<float, NUM_BINS>& mags)
{
    // Copy and reorder from circular buffer
    int readIndex = writeIndex.load();
    for (int i = 0; i < FFT_SIZE; ++i)
    {
        int idx = (readIndex + i) % FFT_SIZE;
        fftData[i] = buffer[idx];
    }

    // Apply window
    window.multiplyWithWindowingTable(fftData.data(), FFT_SIZE);

    // Perform FFT
    fft.performFrequencyOnlyForwardTransform(fftData.data());

    // Convert to dB and apply slope compensation
    float referenceFreq = 1000.0f;

    for (int i = 0; i < NUM_BINS; ++i)
    {
        float magnitude = fftData[i];
        float db = DSPUtils::linearToDecibels(magnitude / static_cast<float>(FFT_SIZE));

        // Apply slope compensation
        if (slopeDbPerOctave > 0.0f && i > 0)
        {
            float freq = getFrequencyForBin(i);
            float octaves = std::log2(freq / referenceFreq);
            db += octaves * slopeDbPerOctave;
        }

        mags[i] = db;
    }
}

void SpectrumAnalyzer::timerCallback()
{
    // Process main buffer
    processFFT(sampleBuffer, magnitudes);

    // Smooth magnitudes with increased smoothing for cleaner look
    for (int i = 0; i < NUM_BINS; ++i)
    {
        smoothedMagnitudes[i] = smoothedMagnitudes[i] * smoothingFactor + magnitudes[i] * (1.0f - smoothingFactor);

        // Peak hold with decay
        if (magnitudes[i] > peakMagnitudes[i])
            peakMagnitudes[i] = magnitudes[i];
        else
            peakMagnitudes[i] *= peakDecayRate;
    }

    // Process pre/post if enabled
    if (showPre)
    {
        processFFT(preBuffer, preMagnitudes);
        for (int i = 0; i < NUM_BINS; ++i)
            smoothedPreMagnitudes[i] = smoothedPreMagnitudes[i] * smoothingFactor + preMagnitudes[i] * (1.0f - smoothingFactor);
    }

    if (showPost)
    {
        processFFT(postBuffer, postMagnitudes);
        for (int i = 0; i < NUM_BINS; ++i)
            smoothedPostMagnitudes[i] = smoothedPostMagnitudes[i] * smoothingFactor + postMagnitudes[i] * (1.0f - smoothingFactor);
    }

    repaint();
}

void SpectrumAnalyzer::paint(juce::Graphics& g)
{
    auto bounds = getLocalBounds().toFloat();

    // Dark background
    g.setColour(juce::Colour(0xff0d0d0d));
    g.fillRoundedRectangle(bounds, 6.0f);

    // Subtle border
    g.setColour(juce::Colour(0xff2a2a2a));
    g.drawRoundedRectangle(bounds.reduced(0.5f), 6.0f, 1.0f);

    // Draw grid first (behind spectrum)
    drawGrid(g);

    // Draw spectrums
    auto analyzerBounds = getAnalyzerBounds();
    g.reduceClipRegion(analyzerBounds.toNearestInt());

    if (showPre && showPost)
    {
        // Show both pre and post
        drawSpectrum(g, smoothedPreMagnitudes, juce::Colour(0xff505050), 0.4f, false);
        drawSpectrum(g, smoothedPostMagnitudes, juce::Colour(0xff00d4ff), 1.0f, true);
    }
    else if (showPost)
    {
        drawSpectrum(g, smoothedPostMagnitudes, juce::Colour(0xff00d4ff), 1.0f, true);
    }
    else if (showPre)
    {
        drawSpectrum(g, smoothedPreMagnitudes, juce::Colour(0xff00d4ff), 1.0f, true);
    }
    else
    {
        drawSpectrum(g, smoothedMagnitudes, juce::Colour(0xff00d4ff), 1.0f, true);
    }
}

void SpectrumAnalyzer::drawSpectrum(juce::Graphics& g, const std::array<float, NUM_BINS>& mags,
                                     juce::Colour colour, float alpha, bool drawFill)
{
    auto analyzerBounds = getAnalyzerBounds();

    // Build smooth path using cubic interpolation
    juce::Path path;
    juce::Path fillPath;
    bool pathStarted = false;

    float bottom = analyzerBounds.getBottom();
    float prevX = 0, prevY = 0;

    // Collect points for smoothing
    std::vector<juce::Point<float>> points;

    for (int i = 1; i < NUM_BINS; ++i)
    {
        float freq = getFrequencyForBin(i);
        if (freq < minFreq || freq > maxFreq) continue;

        float x = getXForFrequency(freq);
        float y = getYForDecibels(mags[i]);

        // Clamp y to analyzer bounds
        y = juce::jlimit(analyzerBounds.getY(), analyzerBounds.getBottom(), y);

        points.push_back({x, y});
    }

    if (points.size() < 2) return;

    // Apply additional smoothing by averaging neighboring points
    std::vector<juce::Point<float>> smoothedPoints;
    const int smoothWindow = 3;
    for (size_t i = 0; i < points.size(); ++i)
    {
        float avgX = 0, avgY = 0;
        int count = 0;
        for (int j = -smoothWindow; j <= smoothWindow; ++j)
        {
            int idx = static_cast<int>(i) + j;
            if (idx >= 0 && idx < static_cast<int>(points.size()))
            {
                avgX += points[idx].x;
                avgY += points[idx].y;
                count++;
            }
        }
        smoothedPoints.push_back({avgX / count, avgY / count});
    }

    // Create smooth curve using quadratic bezier approximation
    path.startNewSubPath(smoothedPoints[0]);
    fillPath.startNewSubPath(smoothedPoints[0].x, bottom);
    fillPath.lineTo(smoothedPoints[0]);

    for (size_t i = 1; i < smoothedPoints.size(); ++i)
    {
        auto& p0 = smoothedPoints[i - 1];
        auto& p1 = smoothedPoints[i];

        // Simple line for closely spaced points, quadratic for smoother transitions
        float midX = (p0.x + p1.x) * 0.5f;
        float midY = (p0.y + p1.y) * 0.5f;

        path.quadraticTo(p0.x, p0.y, midX, midY);
        fillPath.quadraticTo(p0.x, p0.y, midX, midY);
    }

    // Finish to last point
    path.lineTo(smoothedPoints.back());
    fillPath.lineTo(smoothedPoints.back());

    // Close fill path
    fillPath.lineTo(smoothedPoints.back().x, bottom);
    fillPath.closeSubPath();

    if (drawFill)
    {
        // Create gradient fill from white/cyan at top to transparent at bottom
        juce::ColourGradient gradient(
            colour.withAlpha(0.35f * alpha),
            analyzerBounds.getX(), analyzerBounds.getY(),
            colour.withAlpha(0.0f),
            analyzerBounds.getX(), analyzerBounds.getBottom(),
            false);

        g.setGradientFill(gradient);
        g.fillPath(fillPath);
    }

    // Draw line with slight glow effect
    if (drawFill)
    {
        // Glow layer
        g.setColour(colour.withAlpha(0.15f * alpha));
        g.strokePath(path, juce::PathStrokeType(4.0f, juce::PathStrokeType::curved, juce::PathStrokeType::rounded));
    }

    // Main line
    g.setColour(colour.withAlpha(0.9f * alpha));
    g.strokePath(path, juce::PathStrokeType(1.5f, juce::PathStrokeType::curved, juce::PathStrokeType::rounded));
}

void SpectrumAnalyzer::drawGrid(juce::Graphics& g)
{
    auto bounds = getLocalBounds().toFloat();
    auto analyzerBounds = getAnalyzerBounds();

    // Frequency grid lines - key frequencies
    std::array<float, 5> majorFreqLines = { 20.0f, 100.0f, 1000.0f, 10000.0f, 20000.0f };
    std::array<float, 4> minorFreqLines = { 50.0f, 200.0f, 500.0f, 2000.0f };

    g.setFont(juce::Font(juce::FontOptions(10.0f)));

    // Draw minor frequency lines (dimmer)
    g.setColour(juce::Colour(0xff1a1a1a));
    for (float freq : minorFreqLines)
    {
        float x = getXForFrequency(freq);
        g.drawLine(x, analyzerBounds.getY(), x, analyzerBounds.getBottom(), 0.5f);
    }

    // Draw major frequency lines and labels
    g.setColour(juce::Colour(0xff2a2a2a));
    for (float freq : majorFreqLines)
    {
        float x = getXForFrequency(freq);
        g.drawLine(x, analyzerBounds.getY(), x, analyzerBounds.getBottom(), 1.0f);

        // Labels at bottom
        juce::String label;
        if (freq >= 1000.0f)
            label = juce::String(static_cast<int>(freq / 1000.0f)) + "kHz";
        else
            label = juce::String(static_cast<int>(freq)) + "Hz";

        g.setColour(juce::Colour(0xff606060));
        g.drawText(label, static_cast<int>(x - 25), static_cast<int>(analyzerBounds.getBottom() + 2),
                   50, 16, juce::Justification::centred);
        g.setColour(juce::Colour(0xff2a2a2a));
    }

    // dB grid lines - every 10dB from -60 to 0
    std::array<float, 7> dbLines = { -60.0f, -50.0f, -40.0f, -30.0f, -20.0f, -10.0f, 0.0f };

    for (float db : dbLines)
    {
        float y = getYForDecibels(db);

        // Dim line for intermediate values, brighter for 0dB and -60dB
        if (db == 0.0f || db == -60.0f)
            g.setColour(juce::Colour(0xff2a2a2a));
        else
            g.setColour(juce::Colour(0xff1a1a1a));

        g.drawLine(analyzerBounds.getX(), y, analyzerBounds.getRight(), y, 0.5f);

        // Labels on left side
        g.setColour(juce::Colour(0xff606060));
        juce::String dbLabel = juce::String(static_cast<int>(db));
        g.drawText(dbLabel, 4, static_cast<int>(y - 8), 28, 16, juce::Justification::centredRight);
    }
}

juce::Rectangle<float> SpectrumAnalyzer::getAnalyzerBounds() const
{
    auto bounds = getLocalBounds().toFloat();
    // Leave space for labels: left for dB, bottom for Hz
    return bounds.reduced(35.0f, 8.0f).withTrimmedBottom(18.0f);
}

float SpectrumAnalyzer::getFrequencyForBin(int bin) const
{
    return static_cast<float>(bin) * static_cast<float>(sampleRate) / static_cast<float>(FFT_SIZE);
}

float SpectrumAnalyzer::getXForFrequency(float freq) const
{
    auto analyzerBounds = getAnalyzerBounds();
    float left = analyzerBounds.getX();
    float right = analyzerBounds.getRight();

    float logMin = std::log10(minFreq);
    float logMax = std::log10(maxFreq);
    float logFreq = std::log10(std::max(freq, 1.0f));

    float proportion = (logFreq - logMin) / (logMax - logMin);
    return left + proportion * (right - left);
}

float SpectrumAnalyzer::getYForDecibels(float db) const
{
    auto analyzerBounds = getAnalyzerBounds();
    float top = analyzerBounds.getY();
    float bottom = analyzerBounds.getBottom();

    float clampedDb = std::clamp(db, minDb, maxDb);
    float proportion = (clampedDb - minDb) / (maxDb - minDb);
    return bottom - proportion * (bottom - top);
}

void SpectrumAnalyzer::resized()
{
    // Nothing special needed
}

void SpectrumAnalyzer::setShowPre(bool show)
{
    showPre = show;
}

void SpectrumAnalyzer::setShowPost(bool show)
{
    showPost = show;
}

void SpectrumAnalyzer::setFFTSize(int size)
{
    // For simplicity, we're using fixed FFT size
    // A full implementation would recreate the FFT object
    juce::ignoreUnused(size);
}

void SpectrumAnalyzer::setSlope(float dbPerOctave)
{
    slopeDbPerOctave = dbPerOctave;
}

void SpectrumAnalyzer::setSmoothing(float smoothing)
{
    smoothingFactor = std::clamp(smoothing, 0.0f, 0.99f);
}

void SpectrumAnalyzer::setPeakHold(bool enable)
{
    peakHoldEnabled = enable;
    if (!enable)
        peakMagnitudes.fill(-100.0f);
}
