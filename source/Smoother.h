#pragma once

#include "Parameters.h"

namespace MarsDSP
{
    class Smoother
    {
    public:
        explicit Smoother(const Parameters& p) : params(p) {}

        void prepare(const juce::dsp::ProcessSpec& spec) noexcept
        {
            constexpr double duration = 0.02;
            const int steps = static_cast<int>(spec.sampleRate * duration);
            auto resetAll = [steps](auto& smootherArray)
            {
                for (auto& smoother : smootherArray)
                    smoother.reset(steps);
            };

            resetAll(inputSmoother);
            resetAll(tiltSmoother);
            resetAll(shapeSmoother);
            resetAll(biasSmoother);
            resetAll(flutterSmoother);
            resetAll(speedSmoother);
            resetAll(bumpHeadSmoother);
            resetAll(bumpHzSmoother);
            resetAll(outputSmoother);
        }

        void reset() noexcept
        {
            input = 0.0f;
            const auto inGainVal = juce::Decibels::decibelsToGain(params.input->get());
            for (auto& smoother : inputSmoother)
                smoother.setCurrentAndTargetValue(inGainVal);

            tilt = 0.0f;
            for (auto& smoother : tiltSmoother)
                smoother.setCurrentAndTargetValue(params.tilt->get() * 0.01f);

            shape = 0.0f;
            for (auto& smoother : shapeSmoother)
                smoother.setCurrentAndTargetValue(params.shape->get()); // milliseconds

            bias = 1.0f;
            for (auto& smoother : biasSmoother)
                smoother.setCurrentAndTargetValue(params.bias->get() * 0.01f);

            flutter = 0.0f;
            for (auto& smoother : flutterSmoother)
                smoother.setCurrentAndTargetValue(params.flutter->get() * 0.01f);

            speed = 0.0f;
            for (auto& smoother : speedSmoother)
                smoother.setCurrentAndTargetValue(params.speed->get() * 0.01f);

            bumpHead = 0.0f;
            for (auto& smoother : bumpHeadSmoother)
                smoother.setCurrentAndTargetValue(params.bumpHead->get() * 0.01f);

            bumpHz = 0.0f;
            for (auto& smoother : bumpHzSmoother)
                smoother.setCurrentAndTargetValue(params.bumpHz->get() * 0.01f);

            output = 0.0f;
            const auto outGainVal = juce::Decibels::decibelsToGain(params.output->get());
            for (auto& smoother : outputSmoother)
                smoother.setCurrentAndTargetValue(outGainVal);

        }

        void update() noexcept
        {
            const float inGain = params.input->get();
            const float inGainDB = juce::Decibels::decibelsToGain(inGain);
            for (auto& smoother : inputSmoother)
                smoother.setTargetValue(inGainDB);

            const float newTilt = params.tilt->get() * 0.01f;
            for (auto& smoother : tiltSmoother)
                smoother.setTargetValue(newTilt);

            const float newShape = params.shape->get() * 0.01f;
            for (auto& smoother : shapeSmoother)
                smoother.setTargetValue(newShape);

            const float newBias = params.bias->get() * 0.01f;
            for (auto& smoother : biasSmoother)
                smoother.setTargetValue(newBias);

            const float newFlutter = params.flutter->get() * 0.01f;
            for (auto& smoother : flutterSmoother)
                smoother.setTargetValue(newFlutter);

            const float newFlutterSpeed = params.speed->get() * 0.01f;
            for (auto& smoother : speedSmoother)
                smoother.setTargetValue(newFlutterSpeed);

            const float newBumpHead = params.bumpHead->get() * 0.01f;
            for (auto& smoother : bumpHeadSmoother)
                smoother.setTargetValue(newBumpHead);

            const float newBumpHz = params.bumpHz->get() * 0.01f;
            for (auto& smoother : bumpHzSmoother)
                smoother.setTargetValue(newBumpHz);

            const float outGain = params.output->get();
            const float outGainDB = juce::Decibels::decibelsToGain(outGain);
            for (auto& smoother : outputSmoother)
                smoother.setTargetValue(outGainDB);

            isBypassed = params.bypass->get();
        }

        void smoothen() noexcept
        {
            auto smoothen = [](auto& smootherArray)
            {
                for (auto& smoother : smootherArray)
                    smoother.getNextValue();
            };

            smoothen(inputSmoother);
            smoothen(tiltSmoother);
            smoothen(shapeSmoother);
            smoothen(biasSmoother);
            smoothen(flutterSmoother);
            smoothen(speedSmoother);
            smoothen(bumpHeadSmoother);
            smoothen(bumpHzSmoother);
            smoothen(outputSmoother);
        }

        std::vector<std::array<juce::LinearSmoothedValue<float>, 2>*> getSmoother() noexcept
        {
            return { &inputSmoother,
                     &tiltSmoother,
                     &shapeSmoother,
                     &biasSmoother,
                     &flutterSmoother,
                     &speedSmoother,
                     &bumpHeadSmoother,
                     &bumpHzSmoother,
                     &outputSmoother };
        }

        enum class SmootherUpdateMode
        {
            initialize,
            liveInRealTime
        };

        void setSmoother(int numSamplesToSkip, SmootherUpdateMode init) noexcept
        {
            juce::ignoreUnused(init);

            auto skipArray = [numSamplesToSkip](auto& smootherArray)
            {
                for (auto& s : smootherArray)
                    s.skip(numSamplesToSkip);
            };

            skipArray(inputSmoother);
            skipArray(tiltSmoother);
            skipArray(shapeSmoother);
            skipArray(biasSmoother);
            skipArray(flutterSmoother);
            skipArray(speedSmoother);
            skipArray(bumpHeadSmoother);
            skipArray(bumpHzSmoother);
            skipArray(outputSmoother);
        }

        float getInput(size_t channel = 0) noexcept
        { return inputSmoother[channel].getNextValue(); }

        float getTilt(size_t channel = 0) noexcept
        { return tiltSmoother[channel].getNextValue(); }

        float getShape(size_t channel = 0) noexcept
        { return shapeSmoother[channel].getNextValue(); }

        float getBias(size_t channel = 0) noexcept
        { return biasSmoother[channel].getNextValue(); }

        float getFlutter(size_t channel = 0) noexcept
        { return flutterSmoother[channel].getNextValue(); }

        float getFlutterSpeed(size_t channel = 0) noexcept
        { return speedSmoother[channel].getNextValue(); }

        float getBumpHead(size_t channel = 0) noexcept
        { return bumpHeadSmoother[channel].getNextValue(); }

        float getBumpHz(size_t channel = 0) noexcept
        { return bumpHzSmoother[channel].getNextValue(); }

        float getOutput(size_t channel = 0) noexcept
        { return outputSmoother[channel].getNextValue(); }

    private:

        const Parameters& params;

        float input    { 0.0f };
        float tilt     { 0.0f };
        float shape    { 0.0f };
        float bias     { 0.0f };
        float flutter  { 0.0f };
        float speed    { 0.0f };
        float bumpHead { 0.0f };
        float bumpHz   { 0.0f };
        float output   { 0.0f };

        bool isBypassed  { false };

        std::array<juce::LinearSmoothedValue<float>, 2>

        inputSmoother,
        tiltSmoother,
        shapeSmoother,
        biasSmoother,
        flutterSmoother,
        speedSmoother,
        bumpHeadSmoother,
        bumpHzSmoother,
        outputSmoother;
    };
}
