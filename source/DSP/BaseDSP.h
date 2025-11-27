#pragma once

#include <Includes.h>
#include "Smoother.h"

namespace MarsDSP::DSP
{
    template <typename ParametersType>
    class BaseDSP
    {
    public:
        explicit BaseDSP(const ParametersType& params) : smoother(params) {}
        virtual ~BaseDSP() = default;

        virtual void prepare(const juce::dsp::ProcessSpec& spec)
        {
            smoother.prepare(spec);
            smoother.reset();
        }

        virtual void processBlock(juce::dsp::AudioBlock<float>& block, int num_samples)
        {
            smoother.update();

            for (size_t channel {}; channel < block.getNumChannels(); ++channel)
            {
                auto* data = block.getChannelPointer(channel);

                for (size_t sample {}; sample < num_samples; ++sample)
                {
                    const float xn_input = data[sample];
                    const float yn_output = processSample(xn_input, static_cast<int>(channel));
                    data[sample] = yn_output;
                }
            }
        }

        virtual float processSample(float xn_input, int channel) = 0;

        virtual float getInput(size_t channel = 0)        { return smoother.getInput(channel); }
        virtual float getTilt(size_t channel = 0)         { return smoother.getTilt(channel); }
        virtual float getShape(size_t channel = 0)        { return smoother.getShape(channel); }
        virtual float getBias(size_t channel = 0)         { return smoother.getBias(channel); }
        virtual float getFlutter(size_t channel = 0)      { return smoother.getFlutter(channel); }
        virtual float getFlutterSpeed(size_t channel = 0) { return smoother.getFlutterSpeed(channel); }
        virtual float getBumpHead(size_t channel = 0)     { return smoother.getBumpHead(channel); }
        virtual float getBumpHz(size_t channel = 0)       { return smoother.getBumpHz(channel); }
        virtual float getOutput(size_t channel = 0)       { return smoother.getOutput(channel); }

    private:

        Smoother<ParametersType> smoother;
    };
}
