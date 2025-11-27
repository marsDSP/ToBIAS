#pragma once

#include "Globals.h"
#include "Converters.h"

namespace MarsDSP {

    class Parameters {

    public:

        explicit Parameters(juce::AudioProcessorValueTreeState& vts)
        {
            castParameter(vts, inputParamID, input);
            castParameter(vts, outputParamID, output);
        }

        static juce::AudioProcessorValueTreeState::ParameterLayout createParameterLayout()
        {
            juce::AudioProcessorValueTreeState::ParameterLayout layout;

            layout.add(std::make_unique<juce::AudioParameterFloat>(inputParamID, inputParamIDName,
                juce::NormalisableRange<float> { -12.0f, 12.0f }, 0.0f,
                juce::AudioParameterFloatAttributes().withStringFromValueFunction(Converter::stringFromDecibels)));

            layout.add(std::make_unique<juce::AudioParameterFloat>(outputParamID, outputParamIDName,
                juce::NormalisableRange<float> { -12.0f, 12.0f }, 0.0f,
                juce::AudioParameterFloatAttributes().withStringFromValueFunction(Converter::stringFromDecibels)));

            return layout;
        }

        ~Parameters() = default;

        juce::AudioParameterFloat* input {nullptr};
        juce::AudioParameterFloat* output {nullptr};

    private:

        template<typename T>
        static void castParameter(juce::AudioProcessorValueTreeState& vts,
                        const juce::ParameterID& id, T& destination)
        {
            destination = dynamic_cast<T>(vts.getParameter(id.getParamID()));
            jassert(destination);
        }

    };
}