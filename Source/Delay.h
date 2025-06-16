#pragma once
#include <JuceHeader.h>

#define MAX_DELAY_TIME 5.0

#define DEFAULT_DELAY_TIME 1.0
#define DEFAULT_FB 0.0f

#define TIME_SMOOTHING 0.3
#define AMP_SMOOTHING 0.02

class ModDelay
{
public:
	ModDelay(double defaultMaxDelayTime = MAX_DELAY_TIME, float defaultFeedback = DEFAULT_FB)
	{
		maxDelayTime = defaultMaxDelayTime;
		feedback.setCurrentAndTargetValue(defaultFeedback);
	}

	~ModDelay() {}

	void prepareToPlay(double newSampleRate, int maxBlockSize)
	{
		sampleRate = newSampleRate;
		memorySize = roundToInt(maxDelayTime * sampleRate) + maxBlockSize;
		delayMemory.setSize(2, memorySize);
		delayMemory.clear();

		feedback.reset(sampleRate, AMP_SMOOTHING);
	}

	void releaseResources()
	{
		delayMemory.setSize(0, 0);
		memorySize = 0;
	}

	void processBlock(AudioBuffer<float>& buffer, AudioBuffer<double>& modulation)
	{
		const auto numSamples = buffer.getNumSamples();

		auto bufferData = buffer.getArrayOfWritePointers();
		auto delayData = delayMemory.getArrayOfWritePointers();
		auto modData = modulation.getArrayOfWritePointers();
		auto numModCh = modulation.getNumChannels();


		for (int smp = 0; smp < numSamples; ++smp)
		{
			auto fb = feedback.getNextValue();

			for (int ch = 0; ch < 2; ++ch)
			{
				auto dt = modData[jmin(ch, numModCh - 1)][smp];

				auto readIndex = writeIndex - (dt * sampleRate);

				auto integerPart = static_cast<int>(readIndex);
				auto fractionalPart = readIndex - integerPart;

				auto A = (integerPart + memorySize) % memorySize;
				auto B = (A + 1) % memorySize;
				auto alpha = fractionalPart / (2.0 - fractionalPart);

				// Input data in delay line
				delayData[ch][writeIndex] = bufferData[ch][smp];

				// Read from delay line and write to buffer
				// Allpass interpolation
				auto sampleValue = alpha * (delayData[ch][B] - oldSample[ch]) + delayData[ch][A];
				oldSample[ch] = sampleValue;

				bufferData[ch][smp] = static_cast<float>(sampleValue);

				// Feedback (add delayed signal to delay line)
				delayData[ch][writeIndex] += bufferData[ch][smp] * fb;
			}

			++writeIndex %= memorySize;
		}
	}

	void setFeedback(float newValue)
	{
		feedback.setTargetValue(newValue);
	}

private:

	AudioBuffer<float> delayMemory;

	SmoothedValue<float, ValueSmoothingTypes::Linear> feedback;

	double sampleRate = 1.0;
	double maxDelayTime;
	int memorySize = 0;
	int writeIndex = 0;

	float oldSample[2] = { 0.0f, 0.0f };

	JUCE_DECLARE_NON_COPYABLE_WITH_LEAK_DETECTOR(ModDelay)
};