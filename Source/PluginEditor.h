#pragma once

#include "PluginProcessor.h"

// Draws only the indicator line; the knob itself is part of the background art.
class KnobIndicatorLookAndFeel : public juce::LookAndFeel_V4
{
public:
    float faceRadius = 146.0f;

    void drawRotarySlider (juce::Graphics&, int x, int y, int width, int height,
                           float sliderPos, float startAngle, float endAngle, juce::Slider&) override;
};

class TheDollarEditor : public juce::AudioProcessorEditor, private juce::Timer
{
public:
    explicit TheDollarEditor (TheDollarProcessor&);
    ~TheDollarEditor() override;

    void paint (juce::Graphics&) override;
    void resized() override;

private:
    void timerCallback() override;
    juce::Rectangle<int> glowArea() const;

    KnobIndicatorLookAndFeel knobLook;
    juce::Slider knob;
    juce::AudioProcessorValueTreeState::SliderAttachment attachment;

    juce::Image background, glowDim, glowCrisp, glowHalo;
    juce::Image scaledBackground, scaledDim, scaledCrisp, scaledHalo;

    float shownValue = 0.0f; // eased towards the knob value
    float pulsePhase = 0.0f;

    JUCE_DECLARE_NON_COPYABLE_WITH_LEAK_DETECTOR (TheDollarEditor)
};
