#pragma once

#include <Includes.h>
#include <array>
#include <cmath>

namespace MarsDSP::DSP {

    // ==============================================================================
    // HELPER CLASSES
    // ==============================================================================

    // 1. XORShift Random Generator
    struct RandomGenerator
    {
        uint32_t state = 0xDEADBEEF;

        void seed(uint32_t s) { state = s; }
        
        // Returns random double 0.0 to 1.0 (approx)
        double nextDouble()
        {
            state ^= state << 13;
            state ^= state >> 17;
            state ^= state << 5;
            return static_cast<double>(state) / static_cast<double>(UINT32_MAX);
        }
    };

    // 2. Biquad Filter
    struct Biquad
    {
        double a0 = 0, a1 = 0, a2 = 0, b1 = 0, b2 = 0;
        double sL1 = 0, sL2 = 0, sR1 = 0, sR2 = 0;

        void setCoefficients(double freq, double reso, double sampleRate)
        {
            double K = std::tan(M_PI * (freq / sampleRate)); 
            double norm = 1.0 / (1.0 + K / reso + K * K);
            a0 = K / reso * norm;
            a1 = 0.0;
            a2 = -a0;
            b1 = 2.0 * (K * K - 1.0) * norm;
            b2 = (1.0 - K / reso + K * K) * norm;
        }

        void processL(double& sample)
        {
            double out = (sample * a0) + sL1;
            sL1 = (sample * a1) - (out * b1) + sL2;
            sL2 = (sample * a2) - (out * b2);
            sample = out;
        }

        void processR(double& sample)
        {
            double out = (sample * a0) + sR1;
            sR1 = (sample * a1) - (out * b1) + sR2;
            sR2 = (sample * a2) - (out * b2);
            sample = out;
        }
    };

    // 3. Hysteresis / Slew Limiter
    class HysteresisProcessor
    {
        static constexpr int STAGES = 9;
        struct Stage
        {
            double valL = 0.0;
            double valR = 0.0;
            double threshold = 0.0;
        };
        std::array<Stage, STAGES> stages;

    public:

        void updateThresholds(double bias, double sampleRate)
        {
            double overallscale = sampleRate / 44100.0;
            double formattedBias = (bias * 2.0) - 1.0;

            double overBias = std::pow(1.0 - (formattedBias > 0.0 ? formattedBias * 0.75 : formattedBias), 3) / overallscale;
            if (formattedBias < 0.0)
                overBias = 1.0 / overallscale;

            for (int i = STAGES - 1; i >= 0; --i)
            {
                stages[i].threshold = overBias;
                overBias *= 1.61803398875;
            }
        }

        void process(double& L, double& R, double biasParameter, double sampleRate)
        {
            double overallscale = sampleRate / 44100.0;
            double formattedBias = (biasParameter * 2.0) - 1.0;
            
            // Calculate underBias threshold
            double underBias = (std::pow(formattedBias, 4) * 0.25) / overallscale;
            if (formattedBias > 0.0) underBias = 0.0;

            // Only process if bias is significant
            if (std::abs(formattedBias) <= 0.001)
                return;

            for (auto& stage : stages)
            {
                // Apply Underbias
                if (underBias > 0.0)
                {
                    double stuckL = std::abs(L - (stage.valL / 0.975)) / underBias;
                    if (stuckL < 1.0)
                        L = (L * stuckL) + ((stage.valL / 0.975) * (1.0 - stuckL));
                    
                    double stuckR = std::abs(R - (stage.valR / 0.975)) / underBias;
                    if (stuckR < 1.0)
                        R = (R * stuckR) + ((stage.valR / 0.975) * (1.0 - stuckR));
                }

                // Apply Overbias (Slew Limiting)
                double diffL = L - stage.valL;
                if (diffL > stage.threshold)
                    L = stage.valL + stage.threshold;
                else if (-diffL > stage.threshold)
                    L = stage.valL - stage.threshold;
                stage.valL = L * 0.975;

                double diffR = R - stage.valR;
                if (diffR > stage.threshold)
                    R = stage.valR + stage.threshold;
                else if (-diffR > stage.threshold)
                    R = stage.valR - stage.threshold;
                stage.valR = R * 0.975;
            }
        }
    };

    // 4. Compander
    struct CompanderBand {

        double iirFilter = 0.0;
        double compGain = 1.0;
        double avgLevel = 0.0;

        void process(double& sample, double amount, double freq, bool isDecode)
        {
            // Low pass filter state update
            iirFilter = (iirFilter * (1.0 - freq)) + (sample * freq);
            
            // Extract high-frequency content
            double factor = isDecode ? 2.628 : 2.848;
            double avgFactor = isDecode ? 1.372 : 1.152;
            
            double highPart = (sample - iirFilter) * factor;
            
            // Rolling average of high-frequency content
            highPart += avgLevel;
            avgLevel = (sample - iirFilter) * avgFactor;
            
            // Hard clip the detection signal
            if (highPart > 1.0)
                highPart = 1.0;

            if (highPart < -1.0)
                highPart = -1.0;
            
            double absHigh = std::abs(highPart);
            
            if (absHigh > 0.0)
            {
                // Non-linear companding curve
                double adjust = std::log(1.0 + (255.0 * absHigh)) / 2.40823996531;
                if (adjust > 0.0)
                    absHigh /= adjust;
                
                // Smooth the gain reduction/expansion
                compGain = (compGain * (1.0 - freq)) + (absHigh * freq);
                
                // Apply to input
                sample += ((highPart * compGain) * amount);
            }
        }
    };

    // ==============================================================================
    // MAIN CLASS
    // ==============================================================================

    class TapeDSP {
    public:
        TapeDSP()
        {
            uint32_t sL = static_cast<uint32_t>(rand());
            if (sL == 0)
                sL = 0xDEADBEEF;
            rngL.seed(sL);
            
            uint32_t sR = static_cast<uint32_t>(rand());
            if (sR == 0)
                sR = 0xCAFEBABE;
            rngR.seed(sR);
        }

        void prepare(const juce::dsp::ProcessSpec& spec)
        {
            sampleRate = spec.sampleRate;
            
            // Reset Delay Lines
            std::fill(std::begin(delayL), std::end(delayL), 0.0);
            std::fill(std::begin(delayR), std::end(delayR), 0.0);
            
            // Reset Helper Classes
            hysteresis = HysteresisProcessor();
            compEncodeL = CompanderBand();
            compEncodeR = CompanderBand();
            compDecodeL = CompanderBand();
            compDecodeR = CompanderBand();
        }

        template <typename SmootherType>
        void processTape(const float* inL, const float* inR, float* outL, float* outR, int numSamples, SmootherType &smoother)
        {
            // 1. Update Parameters once per block
            double inputGain = std::pow(smoother.getInput() * 0.5 * 2.0, 2.0);
            double outputGain = smoother.getOutput(); 
            
            double tiltParam = smoother.getTilt();
            double dublyEncodeAmount = tiltParam * 2.0;
            double dublyDecodeAmount = (1.0 - tiltParam) * -2.0;

            if (dublyDecodeAmount < -1.0)
                dublyDecodeAmount = -1.0;

            double shapeParam = smoother.getShape();
            double overallscale = sampleRate / 44100.0;
            
            double iirEncFreq = (1.0 - shapeParam) / overallscale;
            double iirDecFreq = shapeParam / overallscale;
            double iirMidFreq = ((shapeParam * 0.618) + 0.382) / overallscale;

            // Flutter Setup
            double flutterDepth = std::pow(smoother.getFlutter(), 6) * overallscale * 50.0;

            if (flutterDepth > 498.0)
                flutterDepth = 498.0;

            double flutterSpeed = (0.02 * std::pow(smoother.getFlutterSpeed(), 3)) / overallscale;

            // Head Bump Setup
            double headBumpMix = smoother.getBumpHead() * 0.5;
            double headBumpDrive = (smoother.getBumpHead() * 0.1) / overallscale;
            double headBumpFreqParam = smoother.getBumpHz();

            if (headBumpFreqParam < 1.0)
                headBumpFreqParam = 1.0;
            
            double subCurve = std::sin(smoother.getBumpHead() * 3.14159265358979323846);
            double iirSubFreq = (subCurve * 0.008) / overallscale;
            
            // Update Filter Coefficients
            if (headBumpMix > 0.0)
            {
                bumpFilterA.setCoefficients(headBumpFreqParam, 0.618033988, sampleRate);
                bumpFilterB.setCoefficients(headBumpFreqParam * 0.9375, 0.618033988, sampleRate); 
            }

            // Update Hysteresis Thresholds
            hysteresis.updateThresholds(smoother.getBias(), sampleRate);

            // Advance Smoother
            if (numSamples > 1)
            {
                smoother.setSmoother(numSamples - 1, SmootherType::SmootherUpdateMode::liveInRealTime);
            }

            // 2. Process Loop
            for (int i = 0; i < numSamples; ++i)
            {
                double L = inL[i];
                double R = inR[i];

                // Denormal check
                if (std::abs(L) < 1.18e-23) L = rngL.nextDouble() * 1.18e-17;
                if (std::abs(R) < 1.18e-23) R = rngR.nextDouble() * 1.18e-17;

                // Input Gain
                if (inputGain != 1.0)
                {
                    L *= inputGain; R *= inputGain;
                }

                // A. Encode (Pre-emphasis)
                compEncodeL.process(L, dublyEncodeAmount, iirEncFreq, false);
                compEncodeR.process(R, dublyEncodeAmount, iirEncFreq, false);

                // B. Tape Transport (Flutter)
                if (flutterDepth > 0.0)
                {
                    processFlutter(L, R, flutterDepth, flutterSpeed);
                }

                // C. Hysteresis (Bias & Slew Limiting)
                hysteresis.process(L, R, smoother.getBias(), sampleRate);

                // D. Tape Saturation Core (Split Band Saturation)
                processSaturation(L, iirMidRollerL, iirLowCutoffL, iirMidFreq, iirSubFreq, headBumpMix, headBumpDrive, true);
                processSaturation(R, iirMidRollerR, iirLowCutoffR, iirMidFreq, iirSubFreq, headBumpMix, headBumpDrive, false);

                // E. Decode (De-emphasis)
                compDecodeL.process(L, dublyDecodeAmount, iirDecFreq, true);
                compDecodeR.process(R, dublyDecodeAmount, iirDecFreq, true);

                // Output Gain
                if (outputGain != 1.0)
                {
                    L *= outputGain; R *= outputGain;
                }

                // F. Soft Clipper
                processSoftClip(L, lastSampleL, wasPosClipL, wasNegClipL);
                processSoftClip(R, lastSampleR, wasPosClipR, wasNegClipR);

                outL[i] = static_cast<float>(L);
                outR[i] = static_cast<float>(R);
            }
        }

    private:

        double sampleRate = 44100.0;
        RandomGenerator rngL, rngR;

        // Transport State
        double delayL[1002] = {0};
        double delayR[1002] = {0};
        int writeIndex = 0;
        double sweepL = 3.14159, sweepR = 3.14159;
        double nextMaxL = 0.5, nextMaxR = 0.5;

        // Saturation State
        double iirMidRollerL = 0, iirMidRollerR = 0;

        // Used for sub-bass cut if needed
        double iirLowCutoffL = 0, iirLowCutoffR = 0;

        // Head Bump State
        double headBumpAccL = 0, headBumpAccR = 0;
        Biquad bumpFilterA, bumpFilterB;

        // Helper Classes instances
        HysteresisProcessor hysteresis;
        CompanderBand compEncodeL, compEncodeR, compDecodeL, compDecodeR;

        // Clipper State
        double lastSampleL = 0, lastSampleR = 0;
        bool wasPosClipL = false, wasNegClipL = false, wasPosClipR = false, wasNegClipR = false;

        // Lagrange 5th Interpolation for flutter
        double getLagrangeSample(double* buffer, int baseIndex, double frac)
        {
             double d_2 = frac + 2.0;
             double d_1 = frac + 1.0;
             double d0  = frac;
             double d1  = frac - 1.0;
             double d2  = frac - 2.0;
             double d3  = frac - 3.0;

             double c_2 = (d_1 * d0 * d1 * d2 * d3) * -0.00833333333333333; // 1 / -120
             double c_1 = (d_2 * d0 * d1 * d2 * d3) * 0.04166666666666667;  // 1 / 24
             double c0  = (d_2 * d_1 * d1 * d2 * d3) * -0.08333333333333333; // 1 / -12
             double c1  = (d_2 * d_1 * d0 * d2 * d3) * 0.08333333333333333;  // 1 / 12
             double c2  = (d_2 * d_1 * d0 * d1 * d3) * -0.04166666666666667; // 1 / -24
             double c3  = (d_2 * d_1 * d0 * d1 * d2) * 0.00833333333333333;  // 1 / 120

             auto get = [&](int offset)
             {
                 return buffer[(baseIndex + offset + 10000) % 1000];
             };

             return (get(-2) * c_2) +
                    (get(-1) * c_1) +
                    (get(0)  * c0) +
                    (get(1)  * c1) +
                    (get(2)  * c2) +
                    (get(3)  * c3);
        }

        void processFlutter(double& L, double& R, double depth, double speed)
        {
            if (writeIndex < 0 || writeIndex > 999)
                writeIndex = 0;

            delayL[writeIndex] = L;
            delayR[writeIndex] = R;
            
            // Calculate Read Position L
            double offsetL = depth + (depth * std::sin(sweepL));
            sweepL += nextMaxL * speed;
            if (sweepL > 6.2831853)
            {
                sweepL -= 6.2831853;
                double flutA = 0.24 + (rngL.nextDouble() * 0.74);
                double flutB = 0.24 + (rngL.nextDouble() * 0.74);

                // Scrape flutter logic
                nextMaxL = (std::abs(flutA - std::sin(sweepR + nextMaxR)) < std::abs(flutB - std::sin(sweepR + nextMaxR))) ? flutA : flutB;
            }
            
            // Interpolation L
            int count = writeIndex + static_cast<int>(std::floor(offsetL));
            double frac = offsetL - std::floor(offsetL);
            L = getLagrangeSample(delayL, count, frac);

            // Calculate Read Position R
            double offsetR = depth + (depth * std::sin(sweepR));
            sweepR += nextMaxR * speed;
            if (sweepR > 6.2831853)
            {
                sweepR -= 6.2831853;
                double flutA = 0.24 + (rngR.nextDouble() * 0.74);
                double flutB = 0.24 + (rngR.nextDouble() * 0.74);

                // Scrape flutter logic
                nextMaxR = (std::abs(flutA - std::sin(sweepL + nextMaxL)) < std::abs(flutB - std::sin(sweepL + nextMaxL))) ? flutA : flutB;
            }

            // Interpolation R
            count = writeIndex + static_cast<int>(std::floor(offsetR));
            frac = offsetR - std::floor(offsetR);
            R = getLagrangeSample(delayR, count, frac);
            
            writeIndex++; // Increment global buffer index
        }

        void processSaturation(double& sample, double& midRoller, double& lowCutoff, double midFreq, double subFreq, double bumpMix, double bumpDrive, bool isLeft)
        {
            // Crossover
            midRoller = (midRoller * (1.0 - midFreq)) + (sample * midFreq);
            double highs = sample - midRoller;
            double lows = midRoller;

            if (subFreq > 0.0)
            {
                lowCutoff = (lowCutoff * (1.0 - subFreq)) + (lows * subFreq);
                lows -= lowCutoff;
            }

            // Saturation Curves
            // Lows: Sine saturation (analog warmth)
            if (lows > 1.570796)
                lows = 1.570796;

            if (lows < -1.570796)
                lows = -1.570796;

            lows = std::sin(lows);

            // Highs: Cosine saturation (tape compression)
            double thinned = std::abs(highs) * 1.570796;

            if (thinned > 1.570796)
                thinned = 1.570796;

            thinned = 1.0 - std::cos(thinned);

            if (highs < 0)
                thinned = -thinned;
            highs -= thinned;

            // Head Bump Application
            if (bumpMix > 0.0)
            {
                double& hbAcc = isLeft ? headBumpAccL : headBumpAccR;

                // Cubic distortion for bump
                hbAcc += (lows * bumpDrive);
                hbAcc -= (std::pow(hbAcc, 3) * (0.0618 / std::sqrt(sampleRate / 44100.0)));
                
                // Filter
                double processedBump = hbAcc;
                if (isLeft)
                {
                    bumpFilterA.processL(processedBump);
                    bumpFilterB.processL(processedBump);
                }

                else
                {
                    bumpFilterA.processR(processedBump);
                    bumpFilterB.processR(processedBump);
                }
                
                sample = lows + highs + (processedBump * bumpMix);
            }

            else
            {
                sample = lows + highs;
            }
        }
        
        void processSoftClip(double& sample, double& lastSample, bool& wasPos, bool& wasNeg)
        {
             if (sample > 4.0)
                 sample = 4.0;

             if (sample < -4.0)
                 sample = -4.0;
             
             // Soft clipper
             if (wasPos)
             {
                 if (sample < lastSample) lastSample = 0.7058208 + (sample * 0.2609148);
                 else lastSample = 0.2491717 + (lastSample * 0.7390851);
             }
             wasPos = false;

             if (sample > 0.9549925859)
             {
                 wasPos = true;
                 sample = 0.7058208 + (lastSample * 0.2609148);
             }
             
             if (wasNeg)
             {
                 if (sample > lastSample) lastSample = -0.7058208 + (sample * 0.2609148);
                 else lastSample = -0.2491717 + (lastSample * 0.7390851);
             }
             wasNeg = false;

             if (sample < -0.9549925859)
             {
                 wasNeg = true;
                 sample = -0.7058208 + (lastSample * 0.2609148);
             }
             
             double temp = sample;
             sample = lastSample;
             lastSample = temp;
        }
    };
}