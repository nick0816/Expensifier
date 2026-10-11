#include "PluginEditor.h"
#include "BinaryData.h"

namespace
{
    // Geometry of the artwork, in pixels of the 1254 x 1254 source image.
    constexpr float artSize = 1254.0f;
    constexpr float knobCentreX = 621.0f, knobCentreY = 634.0f;
    constexpr float knobFaceRadius = 146.0f;
    constexpr float knobGrabRadius = 175.0f;
    constexpr float glowX = 440.0f, glowY = 0.0f; // where the glow layers sit

    // The sweep runs from the "Cheap" end of the arc to the "Expensive" end.
    constexpr float sweep = juce::degreesToRadians (112.0f);

    juce::Image loadPng (const void* data, int size)
    {
        return juce::ImageCache::getFromMemory (data, size);
    }

    juce::Image scaleImage (const juce::Image& img, float scale)
    {
        return img.rescaled (juce::jmax (1, juce::roundToInt ((float) img.getWidth() * scale)),
                             juce::jmax (1, juce::roundToInt ((float) img.getHeight() * scale)),
                             juce::Graphics::highResamplingQuality);
    }
}

void KnobIndicatorLookAndFeel::drawRotarySlider (juce::Graphics& g, int x, int y, int width, int height,
                                                 float sliderPos, float startAngle, float endAngle, juce::Slider&)
{
    const auto centre = juce::Rectangle<int> (x, y, width, height).toFloat().getCentre();
    const auto angle = startAngle + sliderPos * (endAngle - startAngle);
    const auto lineFrom = centre.getPointOnCircumference (faceRadius * 0.61f, angle);
    const auto lineTo   = centre.getPointOnCircumference (faceRadius * 0.95f, angle);
    const auto thickness = juce::jmax (2.0f, faceRadius * 0.034f);

    juce::Line<float> line (lineFrom, lineTo);

    // Engraved shadow, soft glow, then the cream-gold indicator itself.
    g.setColour (juce::Colour (0x80301800));
    const juce::Point<float> shadowOffset (thickness * 0.35f, thickness * 0.5f);
    g.drawLine ({ lineFrom + shadowOffset, lineTo + shadowOffset }, thickness * 1.2f);
    g.setColour (juce::Colour (0x40fff0c0));
    g.drawLine (line, thickness * 2.4f);
    g.setColour (juce::Colour (0xfffff2cd));
    g.drawLine (line, thickness);
}

TheDollarEditor::TheDollarEditor (TheDollarProcessor& p)
    : AudioProcessorEditor (p),
      attachment (p.state, TheDollarProcessor::expenseId, knob)
{
    background = loadPng (BinaryData::background_png, BinaryData::background_pngSize);
    glowDim    = loadPng (BinaryData::glow_dim_png,   BinaryData::glow_dim_pngSize);
    glowCrisp  = loadPng (BinaryData::glow_crisp_png, BinaryData::glow_crisp_pngSize);
    glowHalo   = loadPng (BinaryData::glow_halo_png,  BinaryData::glow_halo_pngSize);

    knob.setLookAndFeel (&knobLook);
    knob.setSliderStyle (juce::Slider::RotaryHorizontalVerticalDrag);
    knob.setTextBoxStyle (juce::Slider::NoTextBox, false, 0, 0);
    knob.setRotaryParameters (juce::MathConstants<float>::twoPi - sweep,
                              juce::MathConstants<float>::twoPi + sweep, true);
    knob.setMouseDragSensitivity (220);
    knob.setDoubleClickReturnValue (true, 0.0);
    knob.setMouseCursor (juce::MouseCursor::PointingHandCursor);
    addAndMakeVisible (knob);

    shownValue = (float) knob.getValue();

    setResizable (true, true);
    setResizeLimits (400, 400, (int) artSize, (int) artSize);
    getConstrainer()->setFixedAspectRatio (1.0);
    setSize (640, 640);

    startTimerHz (30);
}

TheDollarEditor::~TheDollarEditor()
{
    knob.setLookAndFeel (nullptr);
}

juce::Rectangle<int> TheDollarEditor::glowArea() const
{
    const auto s = (float) getWidth() / artSize;
    return juce::Rectangle<float> (glowX * s, glowY * s,
                                   (float) glowHalo.getWidth() * s, (float) glowHalo.getHeight() * s)
        .getSmallestIntegerContainer();
}

void TheDollarEditor::resized()
{
    const auto s = (float) getWidth() / artSize;

    scaledBackground = scaleImage (background, s);
    scaledDim   = scaleImage (glowDim, s);
    scaledCrisp = scaleImage (glowCrisp, s);
    scaledHalo  = scaleImage (glowHalo, s);

    knobLook.faceRadius = knobFaceRadius * s;
    const auto grab = knobGrabRadius * s;
    knob.setBounds (juce::Rectangle<float> (grab * 2.0f, grab * 2.0f)
                        .withCentre ({ knobCentreX * s, knobCentreY * s })
                        .toNearestInt());
}

void TheDollarEditor::paint (juce::Graphics& g)
{
    g.drawImageAt (scaledBackground, 0, 0);

    const auto origin = glowArea().getPosition();
    const auto v = shownValue;
    const auto pulse = 1.0f + 0.14f * v * std::sin (pulsePhase);

    // Cheap: the gold looks tarnished. Expensive: it blazes.
    if (const auto dim = 0.55f * (1.0f - v) * (1.0f - v); dim > 0.002f)
    {
        g.setOpacity (dim);
        g.drawImageAt (scaledDim, origin.x, origin.y);
    }

    if (const auto halo = juce::jlimit (0.0f, 1.0f, std::pow (v, 1.3f) * pulse); halo > 0.002f)
    {
        g.setOpacity (halo);
        g.drawImageAt (scaledHalo, origin.x, origin.y);
    }

    if (v > 0.002f)
    {
        g.setOpacity (juce::jlimit (0.0f, 1.0f, v * (0.94f + 0.06f * pulse)));
        g.drawImageAt (scaledCrisp, origin.x, origin.y);
    }

    g.setOpacity (1.0f);
}

void TheDollarEditor::timerCallback()
{
    const auto target = (float) knob.getValue();
    const auto before = shownValue;

    shownValue += (target - shownValue) * 0.3f;
    if (std::abs (target - shownValue) < 0.001f)
        shownValue = target;

    pulsePhase = std::fmod (pulsePhase + 0.11f, juce::MathConstants<float>::twoPi);

    // Only the title area changes, and only while it is glowing or easing.
    if (shownValue > 0.002f || before != shownValue)
        repaint (glowArea());
}
