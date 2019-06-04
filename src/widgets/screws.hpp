#include "rack0.hpp"

using namespace rack;

extern Plugin *pluginInstance;

struct ZZC_Screw : SVGScrew {
  ZZC_Screw() {
    setSvg(APP->window->loadSvg(asset::plugin(pluginInstance, "res/screws/ZZC-Screw.svg")));
    box.size = sw->box.size;
  }
};
