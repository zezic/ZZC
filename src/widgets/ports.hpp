#include "rack.hpp"

using namespace rack;

extern Plugin *plugin;

struct ZZC_PJ_In_Port : SVGPort {
  ZZC_PJ_In_Port() {
    setSVG(SVG::load(assetPlugin(plugin, "res/sockets/ZZC-PJ-In.svg")));
  }
};

struct ZZC_PJ_Out_Port : SVGPort {
  ZZC_PJ_Out_Port() {
    setSVG(SVG::load(assetPlugin(plugin, "res/sockets/ZZC-PJ-Out.svg")));
  }
};
