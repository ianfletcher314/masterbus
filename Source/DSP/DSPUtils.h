#pragma once

#include <cmath>
#include <algorithm>

namespace DSPUtils
{
    constexpr float PI = 3.14159265358979323846f;
    constexpr float TWOPI = 6.28318530717958647692f;

    inline float linearToDecibels(float linear)
    {
        return linear > 0.0f ? 20.0f * std::log10(linear) : -100.0f;
    }

    inline float decibelsToLinear(float dB)
    {
        return std::pow(10.0f, dB / 20.0f);
    }

    inline float mapRange(float value, float inMin, float inMax, float outMin, float outMax)
    {
        return outMin + (outMax - outMin) * (value - inMin) / (inMax - inMin);
    }

    inline float softClip(float sample)
    {
        return std::tanh(sample);
    }

    inline float hardClip(float sample, float threshold = 1.0f)
    {
        return std::clamp(sample, -threshold, threshold);
    }

    // Calculate one-pole filter coefficient for given time constant
    inline float calculateCoefficient(double sampleRate, float timeMs)
    {
        if (timeMs <= 0.0f) return 1.0f;
        return 1.0f - std::exp(-1.0f / (static_cast<float>(sampleRate) * timeMs * 0.001f));
    }

    // Biquad filter coefficients
    struct BiquadCoeffs
    {
        float b0 = 1.0f, b1 = 0.0f, b2 = 0.0f;
        float a1 = 0.0f, a2 = 0.0f;
    };

    // Calculate biquad coefficients for various filter types
    inline BiquadCoeffs calculateLowPass(float sampleRate, float freq, float Q)
    {
        BiquadCoeffs c;
        float w0 = TWOPI * freq / sampleRate;
        float cosW0 = std::cos(w0);
        float sinW0 = std::sin(w0);
        float alpha = sinW0 / (2.0f * Q);

        float a0 = 1.0f + alpha;
        c.b0 = ((1.0f - cosW0) / 2.0f) / a0;
        c.b1 = (1.0f - cosW0) / a0;
        c.b2 = ((1.0f - cosW0) / 2.0f) / a0;
        c.a1 = (-2.0f * cosW0) / a0;
        c.a2 = (1.0f - alpha) / a0;
        return c;
    }

    inline BiquadCoeffs calculateHighPass(float sampleRate, float freq, float Q)
    {
        BiquadCoeffs c;
        float w0 = TWOPI * freq / sampleRate;
        float cosW0 = std::cos(w0);
        float sinW0 = std::sin(w0);
        float alpha = sinW0 / (2.0f * Q);

        float a0 = 1.0f + alpha;
        c.b0 = ((1.0f + cosW0) / 2.0f) / a0;
        c.b1 = (-(1.0f + cosW0)) / a0;
        c.b2 = ((1.0f + cosW0) / 2.0f) / a0;
        c.a1 = (-2.0f * cosW0) / a0;
        c.a2 = (1.0f - alpha) / a0;
        return c;
    }

    inline BiquadCoeffs calculatePeakingEQ(float sampleRate, float freq, float Q, float gainDb)
    {
        BiquadCoeffs c;
        float A = std::pow(10.0f, gainDb / 40.0f);
        float w0 = TWOPI * freq / sampleRate;
        float cosW0 = std::cos(w0);
        float sinW0 = std::sin(w0);
        float alpha = sinW0 / (2.0f * Q);

        float a0 = 1.0f + alpha / A;
        c.b0 = (1.0f + alpha * A) / a0;
        c.b1 = (-2.0f * cosW0) / a0;
        c.b2 = (1.0f - alpha * A) / a0;
        c.a1 = (-2.0f * cosW0) / a0;
        c.a2 = (1.0f - alpha / A) / a0;
        return c;
    }

    inline BiquadCoeffs calculateLowShelf(float sampleRate, float freq, float gainDb, float S = 1.0f)
    {
        BiquadCoeffs c;
        float A = std::pow(10.0f, gainDb / 40.0f);
        float w0 = TWOPI * freq / sampleRate;
        float cosW0 = std::cos(w0);
        float sinW0 = std::sin(w0);
        float alpha = sinW0 / 2.0f * std::sqrt((A + 1.0f / A) * (1.0f / S - 1.0f) + 2.0f);
        float sqrtA2alpha = 2.0f * std::sqrt(A) * alpha;

        float a0 = (A + 1.0f) + (A - 1.0f) * cosW0 + sqrtA2alpha;
        c.b0 = (A * ((A + 1.0f) - (A - 1.0f) * cosW0 + sqrtA2alpha)) / a0;
        c.b1 = (2.0f * A * ((A - 1.0f) - (A + 1.0f) * cosW0)) / a0;
        c.b2 = (A * ((A + 1.0f) - (A - 1.0f) * cosW0 - sqrtA2alpha)) / a0;
        c.a1 = (-2.0f * ((A - 1.0f) + (A + 1.0f) * cosW0)) / a0;
        c.a2 = ((A + 1.0f) + (A - 1.0f) * cosW0 - sqrtA2alpha) / a0;
        return c;
    }

    inline BiquadCoeffs calculateHighShelf(float sampleRate, float freq, float gainDb, float S = 1.0f)
    {
        BiquadCoeffs c;
        float A = std::pow(10.0f, gainDb / 40.0f);
        float w0 = TWOPI * freq / sampleRate;
        float cosW0 = std::cos(w0);
        float sinW0 = std::sin(w0);
        float alpha = sinW0 / 2.0f * std::sqrt((A + 1.0f / A) * (1.0f / S - 1.0f) + 2.0f);
        float sqrtA2alpha = 2.0f * std::sqrt(A) * alpha;

        float a0 = (A + 1.0f) - (A - 1.0f) * cosW0 + sqrtA2alpha;
        c.b0 = (A * ((A + 1.0f) + (A - 1.0f) * cosW0 + sqrtA2alpha)) / a0;
        c.b1 = (-2.0f * A * ((A - 1.0f) + (A + 1.0f) * cosW0)) / a0;
        c.b2 = (A * ((A + 1.0f) + (A - 1.0f) * cosW0 - sqrtA2alpha)) / a0;
        c.a1 = (2.0f * ((A - 1.0f) - (A + 1.0f) * cosW0)) / a0;
        c.a2 = ((A + 1.0f) - (A - 1.0f) * cosW0 - sqrtA2alpha) / a0;
        return c;
    }

    // Multi-pole filter coefficient calculation for 6/12/18/24 dB slopes
    inline float calculateButterworthQ(int order, int stage)
    {
        // Q values for cascaded Butterworth sections
        if (order == 1) return 0.7071f;
        if (order == 2) return 0.7071f;
        if (order == 3) {
            if (stage == 0) return 1.0f;
            return 0.5f;
        }
        if (order == 4) {
            if (stage == 0) return 0.5412f;
            return 1.3065f;
        }
        return 0.7071f;
    }
}
