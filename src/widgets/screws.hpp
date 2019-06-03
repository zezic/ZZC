#include "rack0.hpp"

using namespace rack;

extern Plugin *pluginInstance;

struct ZZC_Screw : SVGScrew {
  ZZC_Screw() {
    sw->setSVG(SVG::load(assetPlugin(pluginInstance, "res/screws/ZZC-Screw.svg")));
    box.size = sw->box.size;
  }
};
