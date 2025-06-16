#pragma once

#include <JuceHeader.h>

#define DEFAULT_DRY_WET 0.5

class DryWet
{
public:
    DryWet(double defaultDW = DEFAULT_DRY_WET)
    {
        dwRatio = defaultDW;
    }

    ~DryWet() {}

    void prepareToPlay(double sr, int maxBlockSize)
    {
        drySignal.setSize(2, maxBlockSize);
        drySignal.clear();

        dryLevel.reset(sr, 0.01);
        wetLevel.reset(sr, 0.01);

        updateState();
    }

    void releaseResources()
    {
        drySignal.setSize(0, 0);
    }

    void copyDrySignal(AudioBuffer<float>& sourceBuffer)
    {
        auto numSamples = sourceBuffer.getNumSamples();

        for (int ch = 0; ch < 2; ++ch)
        {
            drySignal.copyFrom(ch, 0, sourceBuffer, ch, 0, numSamples);
        }
    }

    void mixDrySignal(AudioBuffer<float>& destinationBuffer)
    {
        auto numSamples = destinationBuffer.getNumSamples();

        dryLevel.applyGain(drySignal, numSamples);
        wetLevel.applyGain(destinationBuffer, numSamples);

        for (int ch = 0; ch < 2; ++ch)
        {
            destinationBuffer.addFrom(ch, 0, drySignal, ch, 0, numSamples);
        }
    }

    void setDWRatio(float newValue)
    {
        dwRatio = newValue;
        updateState();
    }


private:

    void updateState()
    {
        dryLevel.setTargetValue(sqrt(1.0 - dwRatio));
        wetLevel.setTargetValue(sqrt(dwRatio));
    }

    AudioBuffer<float> drySignal;

    float dwRatio = DEFAULT_DRY_WET;

    SmoothedValue<float, ValueSmoothingTypes::Linear> dryLevel;
    SmoothedValue<float, ValueSmoothingTypes::Linear> wetLevel;

    JUCE_DECLARE_NON_COPYABLE_WITH_LEAK_DETECTOR(DryWet)
};