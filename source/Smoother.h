#pragma once

#include <Includes.h>

namespace MarsDSP
{
    template <typename ParametersType>
    class Smoother
    {
    public:
        explicit Smoother(const ParametersType& p) : params(p) {}

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
            for (auto& smoother : inputSmoother)
                smoother.setCurrentAndTargetValue(params.input->get());

            tilt = 0.0f;
            for (auto& smoother : tiltSmoother)
                smoother.setCurrentAndTargetValue(params.tilt->get());

            shape = 0.0f;
            for (auto& smoother : shapeSmoother)
                smoother.setCurrentAndTargetValue(params.shape->get());

            bias = 1.0f;
            for (auto& smoother : biasSmoother)
                smoother.setCurrentAndTargetValue(params.bias->get());

            flutter = 0.0f;
            for (auto& smoother : flutterSmoother)
                smoother.setCurrentAndTargetValue(params.flutter->get());

            speed = 0.0f;
            for (auto& smoother : speedSmoother)
                smoother.setCurrentAndTargetValue(params.speed->get());

            bumpHead = 0.0f;
            for (auto& smoother : bumpHeadSmoother)
                smoother.setCurrentAndTargetValue(params.bumpHead->get());

            bumpHz = 0.0f;
            for (auto& smoother : bumpHzSmoother)
                smoother.setCurrentAndTargetValue(params.bumpHz->get());

            output = 0.0f;
            for (auto& smoother : outputSmoother)
                smoother.setCurrentAndTargetValue(params.output->get());

        }

        void update() noexcept
        {
            const float inGain = params.input->get();
            for (auto& smoother : inputSmoother)
                smoother.setTargetValue(inGain);

            const float newTilt = params.tilt->get();
            for (auto& smoother : tiltSmoother)
                smoother.setTargetValue(newTilt);

            const float newShape = params.shape->get();
            for (auto& smoother : shapeSmoother)
                smoother.setTargetValue(newShape);

            const float newBias = params.bias->get();
            for (auto& smoother : biasSmoother)
                smoother.setTargetValue(newBias);

            const float newFlutter = params.flutter->get();
            for (auto& smoother : flutterSmoother)
                smoother.setTargetValue(newFlutter);

            const float newFlutterSpeed = params.speed->get();
            for (auto& smoother : speedSmoother)
                smoother.setTargetValue(newFlutterSpeed);

            const float newBumpHead = params.bumpHead->get();
            for (auto& smoother : bumpHeadSmoother)
                smoother.setTargetValue(newBumpHead);

            const float newBumpHz = params.bumpHz->get();
            for (auto& smoother : bumpHzSmoother)
                smoother.setTargetValue(newBumpHz);

            const float outGain = params.output->get();
            for (auto& smoother : outputSmoother)
                smoother.setTargetValue(outGain);

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

        // A
        float getInput(size_t channel = 0) noexcept
        { return inputSmoother[channel].getNextValue(); }

        // B
        float getTilt(size_t channel = 0) noexcept
        { return tiltSmoother[channel].getNextValue(); }

        // C
        float getShape(size_t channel = 0) noexcept
        { return shapeSmoother[channel].getNextValue(); }

        // D
        float getBias(size_t channel = 0) noexcept
        { return biasSmoother[channel].getNextValue(); }

        // E
        float getFlutter(size_t channel = 0) noexcept
        { return flutterSmoother[channel].getNextValue(); }

        // F
        float getFlutterSpeed(size_t channel = 0) noexcept
        { return speedSmoother[channel].getNextValue(); }

        // G
        float getBumpHead(size_t channel = 0) noexcept
        { return bumpHeadSmoother[channel].getNextValue(); }

        // H
        float getBumpHz(size_t channel = 0) noexcept
        { return bumpHzSmoother[channel].getNextValue(); }

        // I
        float getOutput(size_t channel = 0) noexcept
        { return outputSmoother[channel].getNextValue(); }

        bool getBypass() const noexcept { return isBypassed; }

    private:

        const ParametersType& params;

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
