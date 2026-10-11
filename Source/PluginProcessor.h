#pragma once

#include <juce_audio_processors/juce_audio_processors.h>

// An audio effect that leaves the audio untouched. Its single parameter,
// "Expense", only changes how much the gold in the editor glows.
class TheDollarProcessor : public juce::AudioProcessor
{
public:
    static constexpr const char* expenseId = "expense";

    TheDollarProcessor();

    void prepareToPlay (double, int) override {}
    void releaseResources() override {}
    bool isBusesLayoutSupported (const BusesLayout&) const override;
    void processBlock (juce::AudioBuffer<float>&, juce::MidiBuffer&) override;

    juce::AudioProcessorEditor* createEditor() override;
    bool hasEditor() const override { return true; }

    const juce::String getName() const override { return JucePlugin_Name; }
    bool acceptsMidi() const override { return false; }
    bool producesMidi() const override { return false; }
    bool isMidiEffect() const override { return false; }
    double getTailLengthSeconds() const override { return 0.0; }

    int getNumPrograms() override { return 1; }
    int getCurrentProgram() override { return 0; }
    void setCurrentProgram (int) override {}
    const juce::String getProgramName (int) override { return {}; }
    void changeProgramName (int, const juce::String&) override {}

    void getStateInformation (juce::MemoryBlock&) override;
    void setStateInformation (const void*, int) override;

    juce::AudioProcessorValueTreeState state;

private:
    JUCE_DECLARE_NON_COPYABLE_WITH_LEAK_DETECTOR (TheDollarProcessor)
};
