#pragma once

#include <JuceHeader.h>

// Professional dark theme for mastering plugin
class MasterBusLookAndFeel : public juce::LookAndFeel_V4
{
public:
    // Color palette
    struct Colors
    {
        // Background colors
        static inline juce::Colour background      = juce::Colour(0xff1a1a1a);
        static inline juce::Colour panelBackground = juce::Colour(0xff252525);
        static inline juce::Colour sectionBackground = juce::Colour(0xff2a2a2a);

        // Accent colors
        static inline juce::Colour accent          = juce::Colour(0xff4a9eff);
        static inline juce::Colour accentDark      = juce::Colour(0xff3070aa);
        static inline juce::Colour eqAccent        = juce::Colour(0xff00d4aa);
        static inline juce::Colour compAccent      = juce::Colour(0xffff6b4a);

        // Text colors
        static inline juce::Colour textPrimary     = juce::Colour(0xfff0f0f0);
        static inline juce::Colour textSecondary   = juce::Colour(0xff909090);
        static inline juce::Colour textDim         = juce::Colour(0xff606060);

        // Meter colors
        static inline juce::Colour meterGreen      = juce::Colour(0xff4aff6b);
        static inline juce::Colour meterYellow     = juce::Colour(0xffffcc00);
        static inline juce::Colour meterRed        = juce::Colour(0xffff4444);
        static inline juce::Colour meterBackground = juce::Colour(0xff1f1f1f);

        // Knob colors
        static inline juce::Colour knobBody        = juce::Colour(0xff404040);
        static inline juce::Colour knobRing        = juce::Colour(0xff505050);
        static inline juce::Colour knobIndicator   = juce::Colour(0xffffffff);

        // Grid/line colors
        static inline juce::Colour gridLine        = juce::Colour(0xff333333);
        static inline juce::Colour gridLineMajor   = juce::Colour(0xff444444);
    };

    MasterBusLookAndFeel()
    {
        setColour(juce::ResizableWindow::backgroundColourId, Colors::background);
        setColour(juce::Slider::backgroundColourId, Colors::panelBackground);
        setColour(juce::Slider::trackColourId, Colors::accent);
        setColour(juce::Slider::thumbColourId, Colors::knobIndicator);
        setColour(juce::Label::textColourId, Colors::textPrimary);
        setColour(juce::ComboBox::backgroundColourId, Colors::panelBackground);
        setColour(juce::ComboBox::textColourId, Colors::textPrimary);
        setColour(juce::ComboBox::outlineColourId, Colors::gridLine);
        setColour(juce::TextButton::buttonColourId, Colors::panelBackground);
        setColour(juce::TextButton::textColourOffId, Colors::textPrimary);
        setColour(juce::ToggleButton::textColourId, Colors::textPrimary);
        setColour(juce::ToggleButton::tickColourId, Colors::accent);
    }

    void drawRotarySlider(juce::Graphics& g, int x, int y, int width, int height,
                          float sliderPosProportional, float rotaryStartAngle, float rotaryEndAngle,
                          juce::Slider& slider) override
    {
        auto bounds = juce::Rectangle<float>(static_cast<float>(x), static_cast<float>(y),
                                              static_cast<float>(width), static_cast<float>(height)).reduced(4.0f);
        float cx = bounds.getCentreX();
        float cy = bounds.getCentreY();
        float radius = juce::jmin(bounds.getWidth(), bounds.getHeight()) / 2.0f - 2.0f;

        // Outer ring
        g.setColour(Colors::knobRing);
        g.drawEllipse(cx - radius, cy - radius, radius * 2.0f, radius * 2.0f, 2.0f);

        // Background arc (track)
        juce::Path trackArc;
        float arcRadius = radius - 4.0f;
        trackArc.addCentredArc(cx, cy, arcRadius, arcRadius, 0.0f,
                               rotaryStartAngle, rotaryEndAngle, true);
        g.setColour(Colors::gridLine);
        g.strokePath(trackArc, juce::PathStrokeType(3.0f, juce::PathStrokeType::curved,
                                                     juce::PathStrokeType::rounded));

        // Value arc
        float currentAngle = rotaryStartAngle + sliderPosProportional * (rotaryEndAngle - rotaryStartAngle);
        juce::Path valueArc;
        valueArc.addCentredArc(cx, cy, arcRadius, arcRadius, 0.0f,
                               rotaryStartAngle, currentAngle, true);
        g.setColour(Colors::accent);
        g.strokePath(valueArc, juce::PathStrokeType(3.0f, juce::PathStrokeType::curved,
                                                     juce::PathStrokeType::rounded));

        // Knob body
        float innerRadius = radius * 0.65f;
        juce::ColourGradient knobGradient(Colors::knobBody.brighter(0.2f), cx - innerRadius * 0.5f, cy - innerRadius * 0.5f,
                                           Colors::knobBody.darker(0.3f), cx + innerRadius * 0.5f, cy + innerRadius * 0.5f, true);
        g.setGradientFill(knobGradient);
        g.fillEllipse(cx - innerRadius, cy - innerRadius, innerRadius * 2.0f, innerRadius * 2.0f);

        // Indicator line
        float indicatorLength = innerRadius * 0.7f;
        float indicatorStartRadius = innerRadius * 0.2f;
        float ix1 = cx + indicatorStartRadius * std::cos(currentAngle - juce::MathConstants<float>::halfPi);
        float iy1 = cy + indicatorStartRadius * std::sin(currentAngle - juce::MathConstants<float>::halfPi);
        float ix2 = cx + indicatorLength * std::cos(currentAngle - juce::MathConstants<float>::halfPi);
        float iy2 = cy + indicatorLength * std::sin(currentAngle - juce::MathConstants<float>::halfPi);
        g.setColour(Colors::knobIndicator);
        g.drawLine(ix1, iy1, ix2, iy2, 2.5f);

        juce::ignoreUnused(slider);
    }

    void drawLinearSlider(juce::Graphics& g, int x, int y, int width, int height,
                          float sliderPos, float minSliderPos, float maxSliderPos,
                          const juce::Slider::SliderStyle style, juce::Slider& slider) override
    {
        juce::ignoreUnused(minSliderPos, maxSliderPos);

        if (style == juce::Slider::LinearVertical)
        {
            // Background
            auto bounds = juce::Rectangle<int>(x, y, width, height).toFloat().reduced(2.0f);
            g.setColour(Colors::meterBackground);
            g.fillRoundedRectangle(bounds, 2.0f);

            // Fill
            float fillHeight = sliderPos - y;
            auto fillBounds = bounds.withHeight(fillHeight).withBottomY(bounds.getBottom());
            g.setColour(Colors::accent);
            g.fillRoundedRectangle(fillBounds, 2.0f);
        }
        else
        {
            LookAndFeel_V4::drawLinearSlider(g, x, y, width, height, sliderPos, minSliderPos, maxSliderPos, style, slider);
        }
    }

    void drawButtonBackground(juce::Graphics& g, juce::Button& button, const juce::Colour& backgroundColour,
                              bool isMouseOverButton, bool isButtonDown) override
    {
        auto bounds = button.getLocalBounds().toFloat().reduced(1.0f);

        juce::Colour baseColour = backgroundColour;

        if (button.getToggleState())
            baseColour = Colors::accent;
        else if (isButtonDown)
            baseColour = baseColour.brighter(0.2f);
        else if (isMouseOverButton)
            baseColour = baseColour.brighter(0.1f);

        g.setColour(baseColour);
        g.fillRoundedRectangle(bounds, 4.0f);

        g.setColour(Colors::gridLine);
        g.drawRoundedRectangle(bounds, 4.0f, 1.0f);
    }

    void drawToggleButton(juce::Graphics& g, juce::ToggleButton& button,
                          bool shouldDrawButtonAsHighlighted, bool shouldDrawButtonAsDown) override
    {
        auto bounds = button.getLocalBounds().toFloat().reduced(2.0f);

        juce::Colour bgColour = button.getToggleState() ? Colors::accent : Colors::panelBackground;

        if (shouldDrawButtonAsDown)
            bgColour = bgColour.brighter(0.2f);
        else if (shouldDrawButtonAsHighlighted)
            bgColour = bgColour.brighter(0.1f);

        g.setColour(bgColour);
        g.fillRoundedRectangle(bounds, 4.0f);

        g.setColour(Colors::gridLine);
        g.drawRoundedRectangle(bounds, 4.0f, 1.0f);

        g.setColour(button.getToggleState() ? Colors::background : Colors::textPrimary);
        g.setFont(juce::Font(juce::FontOptions(static_cast<float>(bounds.getHeight()) * 0.6f)));
        g.drawText(button.getButtonText(), bounds, juce::Justification::centred);
    }

    juce::Font getLabelFont(juce::Label& label) override
    {
        juce::ignoreUnused(label);
        return juce::Font(juce::FontOptions(12.0f));
    }

    void drawComboBox(juce::Graphics& g, int width, int height, bool isButtonDown,
                      int buttonX, int buttonY, int buttonW, int buttonH,
                      juce::ComboBox& box) override
    {
        juce::ignoreUnused(buttonX, buttonY, buttonW, buttonH, isButtonDown);

        auto bounds = juce::Rectangle<int>(0, 0, width, height).toFloat().reduced(1.0f);

        g.setColour(Colors::panelBackground);
        g.fillRoundedRectangle(bounds, 4.0f);

        g.setColour(Colors::gridLine);
        g.drawRoundedRectangle(bounds, 4.0f, 1.0f);

        // Draw arrow
        float arrowX = width - 15.0f;
        float arrowY = height * 0.5f;
        juce::Path arrow;
        arrow.addTriangle(arrowX - 4.0f, arrowY - 2.0f,
                          arrowX + 4.0f, arrowY - 2.0f,
                          arrowX, arrowY + 3.0f);
        g.setColour(box.isEnabled() ? Colors::textSecondary : Colors::textDim);
        g.fillPath(arrow);
    }
};

// Specialized look and feel for EQ section
class EQLookAndFeel : public MasterBusLookAndFeel
{
public:
    EQLookAndFeel()
    {
        setColour(juce::Slider::trackColourId, Colors::eqAccent);
    }

    void drawRotarySlider(juce::Graphics& g, int x, int y, int width, int height,
                          float sliderPosProportional, float rotaryStartAngle, float rotaryEndAngle,
                          juce::Slider& slider) override
    {
        // Use EQ accent color for the value arc
        auto bounds = juce::Rectangle<float>(static_cast<float>(x), static_cast<float>(y),
                                              static_cast<float>(width), static_cast<float>(height)).reduced(4.0f);
        float cx = bounds.getCentreX();
        float cy = bounds.getCentreY();
        float radius = juce::jmin(bounds.getWidth(), bounds.getHeight()) / 2.0f - 2.0f;

        // Outer ring
        g.setColour(Colors::knobRing);
        g.drawEllipse(cx - radius, cy - radius, radius * 2.0f, radius * 2.0f, 2.0f);

        // Track arc
        float arcRadius = radius - 4.0f;
        juce::Path trackArc;
        trackArc.addCentredArc(cx, cy, arcRadius, arcRadius, 0.0f,
                               rotaryStartAngle, rotaryEndAngle, true);
        g.setColour(Colors::gridLine);
        g.strokePath(trackArc, juce::PathStrokeType(3.0f, juce::PathStrokeType::curved,
                                                     juce::PathStrokeType::rounded));

        // Value arc
        float currentAngle = rotaryStartAngle + sliderPosProportional * (rotaryEndAngle - rotaryStartAngle);
        juce::Path valueArc;
        valueArc.addCentredArc(cx, cy, arcRadius, arcRadius, 0.0f,
                               rotaryStartAngle, currentAngle, true);
        g.setColour(Colors::eqAccent);
        g.strokePath(valueArc, juce::PathStrokeType(3.0f, juce::PathStrokeType::curved,
                                                     juce::PathStrokeType::rounded));

        // Knob body
        float innerRadius = radius * 0.65f;
        juce::ColourGradient knobGradient(Colors::knobBody.brighter(0.2f), cx - innerRadius * 0.5f, cy - innerRadius * 0.5f,
                                           Colors::knobBody.darker(0.3f), cx + innerRadius * 0.5f, cy + innerRadius * 0.5f, true);
        g.setGradientFill(knobGradient);
        g.fillEllipse(cx - innerRadius, cy - innerRadius, innerRadius * 2.0f, innerRadius * 2.0f);

        // Indicator
        float indicatorLength = innerRadius * 0.7f;
        float indicatorStartRadius = innerRadius * 0.2f;
        float ix1 = cx + indicatorStartRadius * std::cos(currentAngle - juce::MathConstants<float>::halfPi);
        float iy1 = cy + indicatorStartRadius * std::sin(currentAngle - juce::MathConstants<float>::halfPi);
        float ix2 = cx + indicatorLength * std::cos(currentAngle - juce::MathConstants<float>::halfPi);
        float iy2 = cy + indicatorLength * std::sin(currentAngle - juce::MathConstants<float>::halfPi);
        g.setColour(Colors::knobIndicator);
        g.drawLine(ix1, iy1, ix2, iy2, 2.5f);

        juce::ignoreUnused(slider);
    }
};

// Specialized look and feel for Compressor section
class CompressorLookAndFeel : public MasterBusLookAndFeel
{
public:
    CompressorLookAndFeel()
    {
        setColour(juce::Slider::trackColourId, Colors::compAccent);
    }

    void drawRotarySlider(juce::Graphics& g, int x, int y, int width, int height,
                          float sliderPosProportional, float rotaryStartAngle, float rotaryEndAngle,
                          juce::Slider& slider) override
    {
        auto bounds = juce::Rectangle<float>(static_cast<float>(x), static_cast<float>(y),
                                              static_cast<float>(width), static_cast<float>(height)).reduced(4.0f);
        float cx = bounds.getCentreX();
        float cy = bounds.getCentreY();
        float radius = juce::jmin(bounds.getWidth(), bounds.getHeight()) / 2.0f - 2.0f;

        g.setColour(Colors::knobRing);
        g.drawEllipse(cx - radius, cy - radius, radius * 2.0f, radius * 2.0f, 2.0f);

        float arcRadius = radius - 4.0f;
        juce::Path trackArc;
        trackArc.addCentredArc(cx, cy, arcRadius, arcRadius, 0.0f,
                               rotaryStartAngle, rotaryEndAngle, true);
        g.setColour(Colors::gridLine);
        g.strokePath(trackArc, juce::PathStrokeType(3.0f, juce::PathStrokeType::curved,
                                                     juce::PathStrokeType::rounded));

        float currentAngle = rotaryStartAngle + sliderPosProportional * (rotaryEndAngle - rotaryStartAngle);
        juce::Path valueArc;
        valueArc.addCentredArc(cx, cy, arcRadius, arcRadius, 0.0f,
                               rotaryStartAngle, currentAngle, true);
        g.setColour(Colors::compAccent);
        g.strokePath(valueArc, juce::PathStrokeType(3.0f, juce::PathStrokeType::curved,
                                                     juce::PathStrokeType::rounded));

        float innerRadius = radius * 0.65f;
        juce::ColourGradient knobGradient(Colors::knobBody.brighter(0.2f), cx - innerRadius * 0.5f, cy - innerRadius * 0.5f,
                                           Colors::knobBody.darker(0.3f), cx + innerRadius * 0.5f, cy + innerRadius * 0.5f, true);
        g.setGradientFill(knobGradient);
        g.fillEllipse(cx - innerRadius, cy - innerRadius, innerRadius * 2.0f, innerRadius * 2.0f);

        float indicatorLength = innerRadius * 0.7f;
        float indicatorStartRadius = innerRadius * 0.2f;
        float ix1 = cx + indicatorStartRadius * std::cos(currentAngle - juce::MathConstants<float>::halfPi);
        float iy1 = cy + indicatorStartRadius * std::sin(currentAngle - juce::MathConstants<float>::halfPi);
        float ix2 = cx + indicatorLength * std::cos(currentAngle - juce::MathConstants<float>::halfPi);
        float iy2 = cy + indicatorLength * std::sin(currentAngle - juce::MathConstants<float>::halfPi);
        g.setColour(Colors::knobIndicator);
        g.drawLine(ix1, iy1, ix2, iy2, 2.5f);

        juce::ignoreUnused(slider);
    }
};
