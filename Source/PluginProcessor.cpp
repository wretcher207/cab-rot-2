#include "PluginProcessor.h"
#include "PluginEditor.h"

namespace cabrot
{
CabRotProcessor::CabRotProcessor()
    : juce::AudioProcessor (BusesProperties()
                                .withInput  ("Input",  juce::AudioChannelSet::stereo(), true)
                                .withOutput ("Output", juce::AudioChannelSet::stereo(), true))
{
}

void CabRotProcessor::prepareToPlay (double, int)
{
}

void CabRotProcessor::releaseResources()
{
}

bool CabRotProcessor::isBusesLayoutSupported (const BusesLayout& layouts) const
{
    const auto& mainOut = layouts.getMainOutputChannelSet();
    const auto& mainIn  = layouts.getMainInputChannelSet();

    if (mainOut != juce::AudioChannelSet::mono() && mainOut != juce::AudioChannelSet::stereo())
        return false;

    return mainIn == mainOut;
}

void CabRotProcessor::processBlock (juce::AudioBuffer<float>& buffer, juce::MidiBuffer&)
{
    juce::ScopedNoDenormals noDenormals;

    const auto totalNumInputChannels  = getTotalNumInputChannels();
    const auto totalNumOutputChannels = getTotalNumOutputChannels();

    // Clear any output channels that don't have a matching input - prevents garbage in unused buses.
    for (int channel = totalNumInputChannels; channel < totalNumOutputChannels; ++channel)
        buffer.clear (channel, 0, buffer.getNumSamples());

    // Phase 0: pure passthrough. DSP arrives in Phase 4.
    juce::ignoreUnused (totalNumInputChannels);
}

juce::AudioProcessorEditor* CabRotProcessor::createEditor()
{
    return new CabRotEditor (*this);
}

void CabRotProcessor::getStateInformation (juce::MemoryBlock&)
{
    // Phase 0: no state. APVTS arrives in Phase 3.
}

void CabRotProcessor::setStateInformation (const void*, int)
{
    // Phase 0: no state. APVTS arrives in Phase 3.
}
} // namespace cabrot

// JUCE entry point - must live at global scope.
juce::AudioProcessor* JUCE_CALLTYPE createPluginFilter()
{
    return new cabrot::CabRotProcessor();
}
