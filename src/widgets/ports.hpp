#include "rack0.hpp"

using namespace rack;

extern Plugin *pluginInstance;

struct ZZC_PJ_In_Port : SVGPort {
  ZZC_PJ_In_Port() {
    setSVG(APP->window->loadSvg(asset::plugin(pluginInstance, "res/sockets/ZZC-PJ-In.svg")));
  }
};

struct ZZC_PJ_Out_Port : SVGPort {
  ZZC_PJ_Out_Port() {
    setSVG(APP->window->loadSvg(asset::plugin(pluginInstance, "res/sockets/ZZC-PJ-Out.svg")));
  }
};

struct ZZC_PJ_Port : SVGPort {
  ZZC_PJ_Port() {
    setSVG(APP->window->loadSvg(asset::plugin(pluginInstance, "res/sockets/ZZC-PJ.svg")));
    shadow->box.size = Vec(29, 29);
    shadow->box.pos = Vec(-2, 0);
    shadow->blurRadius = 15.0f;
    shadow->opacity = 1.0f;
  }
};
