#include "MasteringEQ.h"

//==============================================================================
// BiquadFilter
//==============================================================================
void BiquadFilter::setCoefficients(const DSPUtils::BiquadCoeffs& c)
{
    coeffs = c;
}

void BiquadFilter::reset()
{
    x1 = x2 = y1 = y2 = 0.0f;
}

float BiquadFilter::processSample(float input)
{
    float output = coeffs.b0 * input + coeffs.b1 * x1 + coeffs.b2 * x2
                   - coeffs.a1 * y1 - coeffs.a2 * y2;
    x2 = x1;
    x1 = input;
    y2 = y1;
    y1 = output;
    return output;
}

//==============================================================================
// MultiStageFilter
//==============================================================================
void MultiStageFilter::prepare(double sampleRate)
{
    currentSampleRate = sampleRate;
    reset();
}

void MultiStageFilter::setParameters(Type type, float freq, int order)
{
    filterType = type;
    frequency = std::clamp(freq, 10.0f, static_cast<float>(currentSampleRate * 0.45));
    filterOrder = std::clamp(order, 1, 4);
    updateCoefficients();
}

void MultiStageFilter::reset()
{
    for (auto& stage : stages)
        stage.reset();
}

void MultiStageFilter::updateCoefficients()
{
    int numStages = filterOrder;
    for (int i = 0; i < numStages; ++i)
    {
        float Q = DSPUtils::calculateButterworthQ(filterOrder, i);
        DSPUtils::BiquadCoeffs c;

        if (filterType == Type::HighPass)
            c = DSPUtils::calculateHighPass(static_cast<float>(currentSampleRate), frequency, Q);
        else
            c = DSPUtils::calculateLowPass(static_cast<float>(currentSampleRate), frequency, Q);

        stages[i].setCoefficients(c);
    }
}

float MultiStageFilter::processSample(float input)
{
    if (!enabled) return input;

    float output = input;
    for (int i = 0; i < filterOrder; ++i)
        output = stages[i].processSample(output);
    return output;
}

//==============================================================================
// ParametricBand
//==============================================================================
void ParametricBand::prepare(double sampleRate)
{
    currentSampleRate = sampleRate;
    updateCoefficients();
}

void ParametricBand::setParameters(float freq, float gain, float q)
{
    frequency = std::clamp(freq, 20.0f, static_cast<float>(currentSampleRate * 0.45));
    gainDb = std::clamp(gain, -18.0f, 18.0f);
    qFactor = std::clamp(q, 0.1f, 10.0f);
    updateCoefficients();
}

void ParametricBand::reset()
{
    filter.reset();
}

void ParametricBand::updateCoefficients()
{
    auto c = DSPUtils::calculatePeakingEQ(static_cast<float>(currentSampleRate), frequency, qFactor, gainDb);
    filter.setCoefficients(c);
}

float ParametricBand::processSample(float input)
{
    if (!enabled || std::abs(gainDb) < 0.01f) return input;
    return filter.processSample(input);
}

//==============================================================================
// ShelfBand
//==============================================================================
void ShelfBand::prepare(double sampleRate)
{
    currentSampleRate = sampleRate;
    updateCoefficients();
}

void ShelfBand::setParameters(Type type, float freq, float gain)
{
    shelfType = type;
    frequency = std::clamp(freq, 20.0f, static_cast<float>(currentSampleRate * 0.45));
    gainDb = std::clamp(gain, -12.0f, 12.0f);
    updateCoefficients();
}

void ShelfBand::reset()
{
    filter.reset();
}

void ShelfBand::updateCoefficients()
{
    DSPUtils::BiquadCoeffs c;
    if (shelfType == Type::Low)
        c = DSPUtils::calculateLowShelf(static_cast<float>(currentSampleRate), frequency, gainDb);
    else
        c = DSPUtils::calculateHighShelf(static_cast<float>(currentSampleRate), frequency, gainDb);
    filter.setCoefficients(c);
}

float ShelfBand::processSample(float input)
{
    if (!enabled || std::abs(gainDb) < 0.01f) return input;
    return filter.processSample(input);
}

//==============================================================================
// MasteringEQ
//==============================================================================
MasteringEQ::MasteringEQ()
{
    // Set default band frequencies
    // Band 1: 30-300Hz (Low)
    // Band 2: 100Hz-1kHz (Low-Mid)
    // Band 3: 300Hz-5kHz (Mid)
    // Band 4: 1kHz-10kHz (High-Mid)
    for (auto& ch : channels)
    {
        ch.parametric[0].setParameters(80.0f, 0.0f, 1.0f);
        ch.parametric[1].setParameters(300.0f, 0.0f, 1.0f);
        ch.parametric[2].setParameters(1000.0f, 0.0f, 1.0f);
        ch.parametric[3].setParameters(4000.0f, 0.0f, 1.0f);

        ch.lowShelf.setParameters(ShelfBand::Type::Low, 100.0f, 0.0f);
        ch.highShelf.setParameters(ShelfBand::Type::High, 8000.0f, 0.0f);

        ch.highPass.setParameters(MultiStageFilter::Type::HighPass, 20.0f, 2);
        ch.highPass.setEnabled(false);
        ch.lowPass.setParameters(MultiStageFilter::Type::LowPass, 20000.0f, 2);
        ch.lowPass.setEnabled(false);
    }
}

void MasteringEQ::prepare(double sampleRate, int samplesPerBlock)
{
    currentSampleRate = sampleRate;
    currentBlockSize = samplesPerBlock;

    for (auto& ch : channels)
    {
        ch.highPass.prepare(sampleRate);
        ch.lowPass.prepare(sampleRate);
        ch.lowShelf.prepare(sampleRate);
        ch.highShelf.prepare(sampleRate);
        for (auto& band : ch.parametric)
            band.prepare(sampleRate);
    }

    if (linearPhaseMode)
        prepareLinearPhase();
}

void MasteringEQ::reset()
{
    for (auto& ch : channels)
    {
        ch.highPass.reset();
        ch.lowPass.reset();
        ch.lowShelf.reset();
        ch.highShelf.reset();
        for (auto& band : ch.parametric)
            band.reset();
    }
}

void MasteringEQ::process(juce::AudioBuffer<float>& buffer)
{
    if (bypassed) return;

    if (linearPhaseMode)
        processLinearPhase(buffer);
    else if (midSideMode)
        processMidSide(buffer);
    else
        processMinimumPhase(buffer);

    // Apply output gain
    if (std::abs(outputGainLinear - 1.0f) > 0.0001f)
        buffer.applyGain(outputGainLinear);
}

void MasteringEQ::processMinimumPhase(juce::AudioBuffer<float>& buffer)
{
    const int numChannels = buffer.getNumChannels();
    const int numSamples = buffer.getNumSamples();

    for (int ch = 0; ch < std::min(numChannels, 2); ++ch)
    {
        float* data = buffer.getWritePointer(ch);
        for (int i = 0; i < numSamples; ++i)
        {
            data[i] = processSampleThroughEQ(data[i], ch);
        }
    }
}

void MasteringEQ::processMidSide(juce::AudioBuffer<float>& buffer)
{
    if (buffer.getNumChannels() < 2) return;

    const int numSamples = buffer.getNumSamples();
    float* left = buffer.getWritePointer(0);
    float* right = buffer.getWritePointer(1);

    for (int i = 0; i < numSamples; ++i)
    {
        // Encode to M/S
        float mid = (left[i] + right[i]) * 0.5f;
        float side = (left[i] - right[i]) * 0.5f;

        // Process mid through channel 0, side through channel 1
        mid = processSampleThroughEQ(mid, 0);
        side = processSampleThroughEQ(side, 1);

        // Decode back to L/R
        left[i] = mid + side;
        right[i] = mid - side;
    }
}

void MasteringEQ::processLinearPhase(juce::AudioBuffer<float>& buffer)
{
    // Simplified linear phase implementation using overlap-add
    // For a full implementation, you would use FFT-based convolution
    // This is a placeholder that uses minimum phase for now
    processMinimumPhase(buffer);
}

void MasteringEQ::prepareLinearPhase()
{
    const int fftSize = fft.getSize();
    fftBuffer.resize(fftSize * 2, 0.0f);
    irBuffer.resize(fftSize, 0.0f);
    overlapBuffer.resize(fftSize, 0.0f);
    linearPhaseReady = true;
}

void MasteringEQ::updateLinearPhaseIR()
{
    // Generate impulse response from current EQ settings
    // This would be implemented for proper linear phase processing
}

float MasteringEQ::processSampleThroughEQ(float input, int channel)
{
    auto& ch = channels[channel];

    float output = input;

    // Signal flow: HPF -> Low Shelf -> Parametric bands -> High Shelf -> LPF
    output = ch.highPass.processSample(output);
    output = ch.lowShelf.processSample(output);

    for (auto& band : ch.parametric)
        output = band.processSample(output);

    output = ch.highShelf.processSample(output);
    output = ch.lowPass.processSample(output);

    return output;
}

void MasteringEQ::encodeToMidSide(float& left, float& right)
{
    float mid = (left + right) * 0.5f;
    float side = (left - right) * 0.5f;
    left = mid;
    right = side;
}

void MasteringEQ::decodeFromMidSide(float& mid, float& side)
{
    float left = mid + side;
    float right = mid - side;
    mid = left;
    side = right;
}

// HPF controls
void MasteringEQ::setHighPassFrequency(float freq)
{
    for (auto& ch : channels)
        ch.highPass.setParameters(MultiStageFilter::Type::HighPass, freq, 2);
}

void MasteringEQ::setHighPassSlope(int slopeDb)
{
    int order = slopeDb / 6;
    for (auto& ch : channels)
        ch.highPass.setParameters(MultiStageFilter::Type::HighPass,
            ch.highPass.isEnabled() ? 30.0f : 20.0f, order);
}

void MasteringEQ::setHighPassEnabled(bool enabled)
{
    for (auto& ch : channels)
        ch.highPass.setEnabled(enabled);
}

// LPF controls
void MasteringEQ::setLowPassFrequency(float freq)
{
    for (auto& ch : channels)
        ch.lowPass.setParameters(MultiStageFilter::Type::LowPass, freq, 2);
}

void MasteringEQ::setLowPassSlope(int slopeDb)
{
    int order = slopeDb / 6;
    for (auto& ch : channels)
        ch.lowPass.setParameters(MultiStageFilter::Type::LowPass, 20000.0f, order);
}

void MasteringEQ::setLowPassEnabled(bool enabled)
{
    for (auto& ch : channels)
        ch.lowPass.setEnabled(enabled);
}

// Shelf controls
void MasteringEQ::setLowShelfFrequency(float freq)
{
    for (auto& ch : channels)
        ch.lowShelf.setParameters(ShelfBand::Type::Low, freq, ch.lowShelf.getGain());
}

void MasteringEQ::setLowShelfGain(float gainDb)
{
    for (auto& ch : channels)
        ch.lowShelf.setParameters(ShelfBand::Type::Low, ch.lowShelf.getFrequency(), gainDb);
}

void MasteringEQ::setLowShelfEnabled(bool enabled)
{
    for (auto& ch : channels)
        ch.lowShelf.setEnabled(enabled);
}

void MasteringEQ::setHighShelfFrequency(float freq)
{
    for (auto& ch : channels)
        ch.highShelf.setParameters(ShelfBand::Type::High, freq, ch.highShelf.getGain());
}

void MasteringEQ::setHighShelfGain(float gainDb)
{
    for (auto& ch : channels)
        ch.highShelf.setParameters(ShelfBand::Type::High, ch.highShelf.getFrequency(), gainDb);
}

void MasteringEQ::setHighShelfEnabled(bool enabled)
{
    for (auto& ch : channels)
        ch.highShelf.setEnabled(enabled);
}

// Parametric band controls
void MasteringEQ::setBandFrequency(int band, float freq)
{
    if (band < 0 || band >= NUM_PARAMETRIC_BANDS) return;
    for (auto& ch : channels)
    {
        auto& b = ch.parametric[band];
        b.setParameters(freq, b.getGain(), b.getQ());
    }
}

void MasteringEQ::setBandGain(int band, float gainDb)
{
    if (band < 0 || band >= NUM_PARAMETRIC_BANDS) return;
    for (auto& ch : channels)
    {
        auto& b = ch.parametric[band];
        b.setParameters(b.getFrequency(), gainDb, b.getQ());
    }
}

void MasteringEQ::setBandQ(int band, float q)
{
    if (band < 0 || band >= NUM_PARAMETRIC_BANDS) return;
    for (auto& ch : channels)
    {
        auto& b = ch.parametric[band];
        b.setParameters(b.getFrequency(), b.getGain(), q);
    }
}

void MasteringEQ::setBandEnabled(int band, bool enabled)
{
    if (band < 0 || band >= NUM_PARAMETRIC_BANDS) return;
    for (auto& ch : channels)
        ch.parametric[band].setEnabled(enabled);
}

// Global controls
void MasteringEQ::setLinearPhase(bool useLinearPhase)
{
    linearPhaseMode = useLinearPhase;
    if (linearPhaseMode && !linearPhaseReady)
        prepareLinearPhase();
}

void MasteringEQ::setMidSideMode(bool useMidSide)
{
    midSideMode = useMidSide;
}

void MasteringEQ::setBypass(bool shouldBypass)
{
    bypassed = shouldBypass;
}

void MasteringEQ::setOutputGain(float gainDb)
{
    outputGainLinear = DSPUtils::decibelsToLinear(gainDb);
}

std::array<float, 512> MasteringEQ::getMagnitudeResponse(float sampleRate) const
{
    std::array<float, 512> response;
    response.fill(0.0f);

    // Calculate magnitude response at 512 frequency points
    for (int i = 0; i < 512; ++i)
    {
        // Logarithmic frequency scale from 20Hz to 20kHz
        float freq = 20.0f * std::pow(1000.0f, static_cast<float>(i) / 511.0f);

        // This is a simplified response - proper implementation would
        // calculate actual filter response at each frequency
        response[i] = 0.0f; // dB
    }

    return response;
}
