#include "PluginProcessor.h"
#include "PluginEditor.h"
#include "Parameters.h"



//==============================================================================
StereoFlangerAudioProcessor::StereoFlangerAudioProcessor()
    : parameters(*this, nullptr, "StereoFlanger", Parameters::createParameterLayout()),
    delay(Parameters::maxDelayTime, Parameters::defaultFeedback),
    drywetter(Parameters::defaultDryWet),
    lfo(Parameters::defaultFreq, Parameters::defaultWaveform, Parameters::defaultPhase),
    timeModulation(Parameters::defaultDelayTime, Parameters::defaultAmount)
{
    Parameters::addListenerToAllParameters(parameters, this);
}

StereoFlangerAudioProcessor::~StereoFlangerAudioProcessor()
{
}

void StereoFlangerAudioProcessor::prepareToPlay(double sampleRate, int samplesPerBlock)
{
    drywetter.prepareToPlay(sampleRate, samplesPerBlock);
    delay.prepareToPlay(sampleRate, samplesPerBlock);
    lfo.prepareToPlay(sampleRate);
    modulation.setSize(2, samplesPerBlock);
    timeModulation.prepareToPlay(sampleRate);
}

void StereoFlangerAudioProcessor::releaseResources()
{
    drywetter.releaseResources();
    delay.releaseResources();
    modulation.setSize(0, 0);
}

void StereoFlangerAudioProcessor::processBlock(juce::AudioBuffer<float>& buffer, juce::MidiBuffer& midiMessages)
{
    juce::ScopedNoDenormals noDenormals;

    const auto numSamples = buffer.getNumSamples();
    const auto numCh = buffer.getNumChannels();

    // Generazione della modulazione
    lfo.getNextAudioBlock(modulation, numSamples);

    timeModulation.processBlock(modulation, numSamples);

   
    

    for (int ch = 0; ch < numCh; ++ch)
        FloatVectorOperations::min(modulation.getWritePointer(ch), modulation.getWritePointer(ch), Parameters::maxDelayTime, numSamples);
    
     //Processing del suono
    drywetter.copyDrySignal(buffer);

    delay.processBlock(buffer, modulation);

    drywetter.mixDrySignal(buffer);
}

//==============================================================================
bool StereoFlangerAudioProcessor::hasEditor() const
{
    return false;
}

juce::AudioProcessorEditor* StereoFlangerAudioProcessor::createEditor()
{
    return new StereoFlangerAudioProcessorEditor(*this);
}

//==============================================================================
void StereoFlangerAudioProcessor::getStateInformation(juce::MemoryBlock& destData)
{
    auto state = parameters.copyState();
    std::unique_ptr<XmlElement> xml(state.createXml());
    copyXmlToBinary(*xml, destData);
}

void StereoFlangerAudioProcessor::setStateInformation(const void* data, int sizeInBytes)
{
    std::unique_ptr<XmlElement> xmlState(getXmlFromBinary(data, sizeInBytes));
    if (xmlState.get() != nullptr)
        if (xmlState->hasTagName(parameters.state.getType()))
            parameters.replaceState(ValueTree::fromXml(*xmlState));
}

void StereoFlangerAudioProcessor::parameterChanged(const String& paramID, float newValue)
{
    if (paramID == Parameters::nameDryWet)
    {
        drywetter.setDWRatio(newValue);
    }

    if (paramID == Parameters::nameDelayTime)
    {
        //delay.setTime(newValue * 0.001);
        timeModulation.setParameter(newValue * 0.001);
    }

    if (paramID == Parameters::nameFeedback)
    {
        delay.setFeedback(newValue);
    }

    if (paramID == Parameters::nameFreq)
    {
        lfo.setFrequency(newValue);
    }

    if (paramID == Parameters::nameWaveform)
    {
        lfo.setWaveform(roundToInt(newValue));
    }

    if (paramID == Parameters::namePhase)
    {
        lfo.setPhaseShift(newValue * 0.001);
    }

    if (paramID == Parameters::nameAmount)
    {
        timeModulation.setModAmount(newValue * 0.001);
    }

    
}

//==============================================================================
// This creates new instances of the plugin..
juce::AudioProcessor* JUCE_CALLTYPE createPluginFilter()
{
    return new StereoFlangerAudioProcessor();
}
