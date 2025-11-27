#pragma once

#include <Includes.h>

inline const juce::ParameterID inputParamID { "input", 1 };
static constexpr const char* inputParamIDName = "Input";

inline const juce::ParameterID tiltParamID { "tilt", 1 };
static constexpr const char* tiltParamIDName = "Tilt";

inline const juce::ParameterID shapeParamID { "shape", 1 };
static constexpr const char* shapeParamIDName = "Shape";

inline const juce::ParameterID biasParamID { "bias", 1 };
static constexpr const char* biasParamIDName = "Bias";

inline const juce::ParameterID flutterParamID { "flutter", 1 };
static constexpr const char* flutterParamIDName = "Flutter";

inline const juce::ParameterID fSpeedParamID { "speed", 1 };
static constexpr const char* fSpeedParamIDName = "fSpeed";

inline const juce::ParameterID bumpParamID { "bump", 1 };
static constexpr const char* bumpParamIDName = "BumpHead";

inline const juce::ParameterID bumpHzParamID { "bumpHz", 1 };
static constexpr const char* bumpHzParamIDName = "BumpHz";

inline const juce::ParameterID outputParamID { "output", 1 };
static constexpr const char* outputParamIDName = "Output";

inline const juce::ParameterID bypassParamID { "bypass", 1 };
static constexpr const char* bypassParamIDName = "Bypass";
