#include "rack.hpp"
#include "window.hpp"

#include "widgets/displays.hpp"
#include "widgets/knobs.hpp"
#include "widgets/lights.hpp"
#include "widgets/ports.hpp"
#include "widgets/screws.hpp"
#include "widgets/sliders.hpp"
#include "widgets/switches.hpp"

using namespace rack;

// Forward-declare the Plugin, defined in ZZC.cpp
extern Plugin *plugin;

// Forward-declare each Model, defined in each module source file
extern Model *modelClock;
extern Model *modelDivider;
extern Model *modelFN3;
extern Model *modelSCVCA;
extern Model *modelSH8;
extern Model *modelSRC;
