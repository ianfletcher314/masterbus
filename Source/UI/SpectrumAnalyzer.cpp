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

    // Smooth magnitudes
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

    // Background
    g.setColour(MasterBusLookAndFeel::Colors::panelBackground);
    g.fillRoundedRectangle(bounds, 4.0f);

    // Draw grid
    drawGrid(g);

    // Draw spectrums
    auto analyzerBounds = bounds.reduced(30.0f, 20.0f);
    g.reduceClipRegion(analyzerBounds.toNearestInt());

    if (showPre && showPost)
    {
        // Show both pre and post
        drawSpectrum(g, smoothedPreMagnitudes, MasterBusLookAndFeel::Colors::textDim, 0.5f);
        drawSpectrum(g, smoothedPostMagnitudes, MasterBusLookAndFeel::Colors::eqAccent, 0.8f);
    }
    else if (showPost)
    {
        drawSpectrum(g, smoothedPostMagnitudes, MasterBusLookAndFeel::Colors::eqAccent, 0.8f);
    }
    else if (showPre)
    {
        drawSpectrum(g, smoothedPreMagnitudes, MasterBusLookAndFeel::Colors::textDim, 0.8f);
    }
    else
    {
        drawSpectrum(g, smoothedMagnitudes, MasterBusLookAndFeel::Colors::accent, 0.8f);
    }

    // Draw peak hold
    if (peakHoldEnabled)
    {
        juce::Path peakPath;
        bool pathStarted = false;

        for (int i = 1; i < NUM_BINS; ++i)
        {
            float freq = getFrequencyForBin(i);
            if (freq < minFreq || freq > maxFreq) continue;

            float x = getXForFrequency(freq);
            float y = getYForDecibels(peakMagnitudes[i]);

            if (!pathStarted)
            {
                peakPath.startNewSubPath(x, y);
                pathStarted = true;
            }
            else
            {
                peakPath.lineTo(x, y);
            }
        }

        g.setColour(MasterBusLookAndFeel::Colors::meterYellow.withAlpha(0.5f));
        g.strokePath(peakPath, juce::PathStrokeType(1.0f));
    }
}

void SpectrumAnalyzer::drawSpectrum(juce::Graphics& g, const std::array<float, NUM_BINS>& mags,
                                     juce::Colour colour, float alpha)
{
    juce::Path path;
    juce::Path fillPath;
    bool pathStarted = false;

    float bottom = getYForDecibels(minDb);

    for (int i = 1; i < NUM_BINS; ++i)
    {
        float freq = getFrequencyForBin(i);
        if (freq < minFreq || freq > maxFreq) continue;

        float x = getXForFrequency(freq);
        float y = getYForDecibels(mags[i]);

        if (!pathStarted)
        {
            path.startNewSubPath(x, y);
            fillPath.startNewSubPath(x, bottom);
            fillPath.lineTo(x, y);
            pathStarted = true;
        }
        else
        {
            path.lineTo(x, y);
            fillPath.lineTo(x, y);
        }
    }

    // Close fill path
    if (pathStarted)
    {
        float lastX = getXForFrequency(maxFreq);
        fillPath.lineTo(lastX, bottom);
        fillPath.closeSubPath();

        // Draw fill
        g.setColour(colour.withAlpha(alpha * 0.2f));
        g.fillPath(fillPath);

        // Draw line
        g.setColour(colour.withAlpha(alpha));
        g.strokePath(path, juce::PathStrokeType(1.5f));
    }
}

void SpectrumAnalyzer::drawGrid(juce::Graphics& g)
{
    auto bounds = getLocalBounds().toFloat();

    // Frequency grid lines
    std::array<float, 10> freqLines = { 20.0f, 50.0f, 100.0f, 200.0f, 500.0f,
                                         1000.0f, 2000.0f, 5000.0f, 10000.0f, 20000.0f };

    g.setColour(MasterBusLookAndFeel::Colors::gridLine);
    g.setFont(juce::Font(juce::FontOptions(10.0f)));

    for (float freq : freqLines)
    {
        float x = getXForFrequency(freq);
        g.drawLine(x, bounds.getY() + 15.0f, x, bounds.getBottom() - 20.0f, 0.5f);

        // Labels
        juce::String label;
        if (freq >= 1000.0f)
            label = juce::String(static_cast<int>(freq / 1000.0f)) + "k";
        else
            label = juce::String(static_cast<int>(freq));

        g.setColour(MasterBusLookAndFeel::Colors::textDim);
        g.drawText(label, static_cast<int>(x - 15), static_cast<int>(bounds.getBottom() - 18),
                   30, 15, juce::Justification::centred);
        g.setColour(MasterBusLookAndFeel::Colors::gridLine);
    }

    // dB grid lines
    for (float db = minDb; db <= maxDb; db += 12.0f)
    {
        float y = getYForDecibels(db);
        g.drawLine(bounds.getX() + 25.0f, y, bounds.getRight() - 5.0f, y, 0.5f);

        // Labels
        g.setColour(MasterBusLookAndFeel::Colors::textDim);
        g.drawText(juce::String(static_cast<int>(db)), 2, static_cast<int>(y - 7), 22, 14,
                   juce::Justification::centredRight);
        g.setColour(MasterBusLookAndFeel::Colors::gridLine);
    }
}

float SpectrumAnalyzer::getFrequencyForBin(int bin) const
{
    return static_cast<float>(bin) * static_cast<float>(sampleRate) / static_cast<float>(FFT_SIZE);
}

float SpectrumAnalyzer::getXForFrequency(float freq) const
{
    auto bounds = getLocalBounds().toFloat();
    float left = bounds.getX() + 30.0f;
    float right = bounds.getRight() - 5.0f;

    float logMin = std::log10(minFreq);
    float logMax = std::log10(maxFreq);
    float logFreq = std::log10(std::max(freq, 1.0f));

    float proportion = (logFreq - logMin) / (logMax - logMin);
    return left + proportion * (right - left);
}

float SpectrumAnalyzer::getYForDecibels(float db) const
{
    auto bounds = getLocalBounds().toFloat();
    float top = bounds.getY() + 15.0f;
    float bottom = bounds.getBottom() - 20.0f;

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
