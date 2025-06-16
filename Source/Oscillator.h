#pragma once
#include <JuceHeader.h>

class NaiveOscillator
{
public:
	NaiveOscillator(double defaultFrequency = 440.0, int defaultWaveform = 0, double defaultPhase = 0)
	{
		waveform = defaultWaveform;
		frequency.setTargetValue(defaultFrequency);
		phaseShift.setCurrentAndTargetValue(defaultPhase);
	}

	~NaiveOscillator() {}

	void prepareToPlay(double sampleRate)
	{
		frequency.reset(sampleRate, 0.02);
		samplePeriod = 1.0 / sampleRate;
	}

	void setFrequency(double newValue)
	{
		jassert(newValue > 0);
		frequency.setTargetValue(newValue);
	}

	void setWaveform(int newValue)
	{
		waveform = newValue;
	}

	void setPhaseShift(double newValue)
	{
		phaseShift.setTargetValue(newValue);
	}

	void getNextAudioBlock(AudioBuffer<double>& buffer, const int numsamples)
	{
		const int numCh = buffer.getNumChannels();

		auto data = buffer.getArrayOfWritePointers();

		double shift = 0.0;
		double sampleValue[2];

		for (int smp = 0; smp < numsamples; ++smp)
		{
			if (phaseShift.isSmoothing())
				shift = phaseShift.getNextValue();
			else
				shift = phaseShift.getCurrentValue();

			getNextAudioSample(sampleValue, shift);

			for (int ch = 0; ch < numCh; ++ch)
			{
				data[ch][smp] = sampleValue[ch];
			}
		}
	}

	void getNextAudioSample(double* sampleValue, double shift = 0.0)
	{
		auto phaseAus = currentPhase; //creo una copia della fase e la modifico ulteriormente per far sfasare i due canali
		phaseAus += shift;
		phaseAus -= static_cast<int>(phaseAus);

		switch (waveform) 
		{
		case 0: // Sinusoidale
			sampleValue[0] = sin(MathConstants<double>::twoPi * phaseAus);
			sampleValue[1] = sin(MathConstants<double>::twoPi * currentPhase);

			break;
		case 1: // Triangular
			sampleValue[0] = 4.0 * abs(phaseAus - 0.5) - 1.0;
			sampleValue[1] = 4.0 * abs(currentPhase - 0.5) - 1.0;
			break;
		case 2: // Saw UP
			sampleValue[0] = 2.0 * phaseAus - 1.0;
			sampleValue[1] = 2.0 * currentPhase - 1.0;
			break;
		case 3: // Saw down
			sampleValue[0] = -2.0 * phaseAus + 1.0;
			sampleValue[1] = -2.0 * currentPhase + 1.0;
			break;
		case 4: // Square
			sampleValue[0] = (phaseAus > 0.5) - (phaseAus < 0.5);
			sampleValue[1] = (currentPhase > 0.5) - (currentPhase < 0.5);
			break;
		default:
			// WTF MAN!
			jassertfalse;
			break;
		}

		phaseIncrement = frequency.getNextValue() * samplePeriod;
		currentPhase += phaseIncrement;
		currentPhase -= static_cast<int>(currentPhase);
	}

private:
	int waveform;
	SmoothedValue<double, ValueSmoothingTypes::Multiplicative> frequency;
	SmoothedValue<double, ValueSmoothingTypes::Linear> phaseShift;

	double currentPhase = 0;
	double phaseIncrement = 0;
	double samplePeriod = 1.0;

	JUCE_DECLARE_NON_COPYABLE_WITH_LEAK_DETECTOR(NaiveOscillator)
};


class ParameterModulation
{
public:

	ParameterModulation(const double defaultParameter = 0.0, const double defaultModAmount = 0.0)
	{
		parameter.setCurrentAndTargetValue(defaultParameter);
		modAmount.setCurrentAndTargetValue(defaultModAmount);
	}

	~ParameterModulation() {}

	void prepareToPlay(double sampleRate)
	{
		parameter.reset(sampleRate, 0.02);
		modAmount.reset(sampleRate, 0.02);
	}

	void setModAmount(const double newValue)
	{
		modAmount.setTargetValue(newValue);
	}

	void setParameter(const double newValue)
	{
		parameter.setTargetValue(newValue);
	}

	void processBlock(AudioBuffer<double>& buffer, const int numSamples, const int channel = 0)
	{
		auto data = buffer.getArrayOfWritePointers();
		const auto numCh = buffer.getNumChannels();

		// Scalo la modulazione tra 0 e 1
		for (int ch = 0; ch < numCh; ++ch)
		{
			FloatVectorOperations::add(data[ch], 1.0, numSamples);
			FloatVectorOperations::multiply(data[ch], 0.5, numSamples);
		}

		// Scalo la modulazione in accordo con mod amount
		modAmount.applyGain(buffer, numSamples);

		// Sommo modulazione e parametro
		if (parameter.isSmoothing())
		{
			for (int smp = 0; smp < numSamples; ++smp)
				for (int ch = 0; ch < numCh; ++ch)
					data[ch][smp] += ch ? parameter.getCurrentValue() : parameter.getNextValue();
		}
		else
		{
			for (int ch = 0; ch < numCh; ++ch)
				FloatVectorOperations::add(data[ch], parameter.getCurrentValue(), numSamples);
		}
	}

private:

	SmoothedValue<double, ValueSmoothingTypes::Linear> parameter;
	SmoothedValue<double, ValueSmoothingTypes::Linear> modAmount;

	JUCE_DECLARE_NON_COPYABLE_WITH_LEAK_DETECTOR(ParameterModulation)
};
