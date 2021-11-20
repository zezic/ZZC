#include <rack.hpp>

#include "shared.hpp"
#include "widgets.hpp"

using namespace rack;

// Forward-declare the Plugin, defined in ZZC.cpp
extern Plugin *pluginInstance;

// Forward-declare each Model, defined in each module source file
extern Model *modelClock;
extern Model *modelDivider;
extern Model *modelFN3;
extern Model *modelSCVCA;
extern Model *modelSH8;
extern Model *modelSRC;
extern Model *modelDiv;
extern Model *modelDivExp;
extern Model *modelPolygate;
// extern Model *modelWavetablePlayer;
// extern Model *modelPhasor;
