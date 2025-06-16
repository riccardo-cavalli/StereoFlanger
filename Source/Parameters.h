#pragma once
#include <JuceHeader.h>


namespace Parameters
{
	// CONSTANTS
	static const double maxDelayTime = 5.1;

	// PARAM IDs
	static const String nameDelayTime = "DT";
	static const String nameDryWet = "DW";
	static const String nameFeedback = "FB";
	static const String nameFreq = "MF";
	static const String nameWaveform = "MW";
	static const String nameAmount = "MA";
	static const String namePhase = "NP";

	// PARAM DEFAULTS
	static const float defaultDelayTime = 1000.0f;
	static const float defaultDryWet = 0.5f;
	static const float defaultFeedback = 0.0f;
	static const float defaultFreq = 1.0f;
	static const int defaultWaveform = 0;
	static const float defaultAmount = 0.0f;
	static const float defaultPhase = 0.1f;

	static AudioProcessorValueTreeState::ParameterLayout createParameterLayout()
	{
		std::vector<std::unique_ptr<RangedAudioParameter>> parameters;

		parameters.push_back(std::make_unique<AudioParameterFloat>(nameDelayTime, "Delay Time (ms)", NormalisableRange<float>(0.0f, maxDelayTime * 1000.0 - 100.0, 0.1, 0.3), defaultDelayTime));
		parameters.push_back(std::make_unique<AudioParameterFloat>(nameDryWet, "Dry/Wet (%)", 0, 1, defaultDryWet));
		parameters.push_back(std::make_unique<AudioParameterFloat>(nameFeedback, "Feedback (%)", NormalisableRange<float>(0.0f, 1.0f, 0.001, 1.5f), defaultFeedback));
		parameters.push_back(std::make_unique<AudioParameterFloat>(nameFreq, "LFO Frequency (Hz)", NormalisableRange<float>(0.1f, 500.0f, 0.1, 0.3f), defaultFreq));
		parameters.push_back(std::make_unique<AudioParameterChoice>(nameWaveform, "LFO Waveform", StringArray{ "Sinusoid","Triangular","Saw Up","Saw Down","Square" }, defaultWaveform));
		parameters.push_back(std::make_unique<AudioParameterFloat>(nameAmount, "Mod amount (ms)", NormalisableRange<float>(0.0f, 100.0f, 0.1, 0.5f), defaultAmount));
		parameters.push_back(std::make_unique<AudioParameterFloat>(namePhase, "Channel Stereo", NormalisableRange<float>(0.0f, 1.0f, 0.1, 0.1f), defaultPhase));

		return { parameters.begin(), parameters.end() };
	}

	static void addListenerToAllParameters(AudioProcessorValueTreeState& valueTreeState, AudioProcessorValueTreeState::Listener* listener)
	{
		std::unique_ptr<XmlElement> xml(valueTreeState.copyState().createXml());

		for (auto* element : xml->getChildWithTagNameIterator("PARAM"))
		{
			const String& id = element->getStringAttribute("id");
			valueTreeState.addParameterListener(id, listener);
		}
	}
}