#include "PluginProcessor.h"
#include "PluginEditor.h"

static juce::AudioProcessorValueTreeState::ParameterLayout createLayout()
{
    return { std::make_unique<juce::AudioParameterFloat> (
        juce::ParameterID { TheDollarProcessor::expenseId, 1 },
        "Expense",
        juce::NormalisableRange<float> (0.0f, 1.0f),
        0.0f,
        juce::AudioParameterFloatAttributes()
            .withStringFromValueFunction ([] (float v, int) { return juce::String (juce::roundToInt (v * 100.0f)) + " % rich"; })) };
}

TheDollarProcessor::TheDollarProcessor()
    : AudioProcessor (BusesProperties()
                          .withInput ("Input", juce::AudioChannelSet::stereo(), true)
                          .withOutput ("Output", juce::AudioChannelSet::stereo(), true)),
      state (*this, nullptr, "SimonsExpensifier", createLayout())
{
}

bool TheDollarProcessor::isBusesLayoutSupported (const BusesLayout& layouts) const
{
    const auto out = layouts.getMainOutputChannelSet();
    if (out != juce::AudioChannelSet::mono() && out != juce::AudioChannelSet::stereo())
        return false;

    return out == layouts.getMainInputChannelSet();
}

void TheDollarProcessor::processBlock (juce::AudioBuffer<float>&, juce::MidiBuffer&)
{
    // Deliberately empty: input passes through unchanged. Luxury is a state of mind.
}

juce::AudioProcessorEditor* TheDollarProcessor::createEditor()
{
    return new TheDollarEditor (*this);
}

void TheDollarProcessor::getStateInformation (juce::MemoryBlock& destData)
{
    if (auto xml = state.copyState().createXml())
        copyXmlToBinary (*xml, destData);
}

void TheDollarProcessor::setStateInformation (const void* data, int sizeInBytes)
{
    if (auto xml = getXmlFromBinary (data, sizeInBytes))
        if (xml->hasTagName (state.state.getType()))
            state.replaceState (juce::ValueTree::fromXml (*xml));
}

juce::AudioProcessor* JUCE_CALLTYPE createPluginFilter()
{
    return new TheDollarProcessor();
}
