#pragma once

#include "Globals.h"
#include "Converters.h"

namespace MarsDSP {

    class Parameters {

    public:

        explicit Parameters(juce::AudioProcessorValueTreeState& vts)
        {
            castParameter(vts,  inputParamID,   input);
            castParameter(vts,  tiltParamID,    tilt);
            castParameter(vts,  shapeParamID,   shape);
            castParameter(vts,  biasParamID,    bias);
            castParameter(vts,  flutterParamID, flutter);
            castParameter(vts,  fSpeedParamID,  speed);
            castParameter(vts,  bumpParamID,    bumpHead);
            castParameter(vts,  bumpHzParamID,  bumpHz);
            castParameter(vts,  outputParamID,  output);
            castParameter(vts,  bypassParamID,  bypass);
        }

        static juce::AudioProcessorValueTreeState::ParameterLayout createParameterLayout()
        {
            juce::AudioProcessorValueTreeState::ParameterLayout layout;

            // Input
            layout.add(std::make_unique<juce::AudioParameterFloat>
                (inputParamID, inputParamIDName, juce::NormalisableRange<float>
                    { -12.0f, 12.0f }, 0.0f,
                juce::AudioParameterFloatAttributes()
                .withStringFromValueFunction(Converter::stringFromDecibels)));

            // Tilt
            layout.add(std::make_unique<juce::AudioParameterFloat>
                (tiltParamID, tiltParamIDName, juce::NormalisableRange<float>
                    {0.0f, 100.0f, 1.0f}, 0.0f,
                juce::AudioParameterFloatAttributes()
                .withStringFromValueFunction(Converter::stringFromPercent)));

            // Shape
            layout.add(std::make_unique<juce::AudioParameterFloat>
                (shapeParamID, shapeParamIDName, juce::NormalisableRange<float>
                    {0.0f, 100.0f, 1.0f}, 0.0f,
                juce::AudioParameterFloatAttributes()
                .withStringFromValueFunction(Converter::stringFromPercent)));

            // Bias
            layout.add(std::make_unique<juce::AudioParameterFloat>
                (biasParamID, biasParamIDName, juce::NormalisableRange<float>
                    {0.0f, 100.0f, 1.0f}, 50.0f,
                juce::AudioParameterFloatAttributes()
                .withStringFromValueFunction(Converter::stringFromPercent)));

            // Flutter
            layout.add(std::make_unique<juce::AudioParameterFloat>
                (flutterParamID, flutterParamIDName, juce::NormalisableRange<float>
                    {0.0f, 100.0f, 1.0f}, 50.0f,
                juce::AudioParameterFloatAttributes()
                .withStringFromValueFunction(Converter::stringFromPercent)));

            // Flutter Speed
            layout.add(std::make_unique<juce::AudioParameterFloat>
                (fSpeedParamID, fSpeedParamIDName, juce::NormalisableRange<float>
                    {0.0f, 100.0f, 1.0f}, 50.0f,
                juce::AudioParameterFloatAttributes()
                .withStringFromValueFunction(Converter::stringFromPercent)));

            // Bump Head
            layout.add(std::make_unique<juce::AudioParameterFloat>
                (bumpParamID, bumpParamIDName, juce::NormalisableRange<float>
                    {0.0f, 100.0f, 1.0f}, 50.0f,
                juce::AudioParameterFloatAttributes()
                .withStringFromValueFunction(Converter::stringFromPercent)));

            // BumpHz
            layout.add(std::make_unique<juce::AudioParameterFloat>
                (bumpHzParamID, bumpHzParamIDName, juce::NormalisableRange<float>
                    {0.0f, 100.0f, 1.0f}, 50.0f,
                juce::AudioParameterFloatAttributes()
                .withStringFromValueFunction(Converter::stringFromPercent)));

            // Output
            layout.add(std::make_unique<juce::AudioParameterFloat>
                (outputParamID, outputParamIDName, juce::NormalisableRange<float>
                    { -12.0f, 12.0f }, 0.0f,
                juce::AudioParameterFloatAttributes()
                .withStringFromValueFunction(Converter::stringFromDecibels)));

            // Bypass
            layout.add(std::make_unique<juce::AudioParameterBool>
                (bypassParamID, bypassParamIDName, false));

            return layout;
        }

        ~Parameters() = default;

        /*
         * 9
         * ==========GAIN============
         * Input = 0->1 default 0.5
         * Output = 0->1 default 0.5
         * ==========TAPE============
         * Tilt = 0->1 default 0.5
         * Shape = 0->1 default 0.5
         * Bias = 0->1 default 0.5
         * ==========FLUTTER=========
         * Flutter = 0->1 default 0.5
         * FSpeed = 0->1 default 0.5
         * ==========HEAD============
         * Bump = 0->1 default 0.5
         * Freq = 0->150 default 75.0
         */

        juce::AudioParameterFloat* input    { nullptr };
        juce::AudioParameterFloat* tilt     { nullptr };
        juce::AudioParameterFloat* shape    { nullptr };
        juce::AudioParameterFloat* bias     { nullptr };
        juce::AudioParameterFloat* flutter  { nullptr };
        juce::AudioParameterFloat* speed    { nullptr };
        juce::AudioParameterFloat* bumpHead { nullptr };
        juce::AudioParameterFloat* bumpHz   { nullptr };
        juce::AudioParameterFloat* output   { nullptr };

        juce::AudioParameterBool* bypass {nullptr};

    private:

        template<typename T>
        static void castParameter(juce::AudioProcessorValueTreeState& vts,
                        const juce::ParameterID& id, T& destination)
        {
            destination = dynamic_cast<T>(vts.getParameter(id.getParamID()));
            jassert(destination);
        }

        JUCE_DECLARE_NON_COPYABLE_WITH_LEAK_DETECTOR(Parameters)
    };
}