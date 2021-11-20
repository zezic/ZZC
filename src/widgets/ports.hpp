using namespace rack;

extern Plugin *pluginInstance;

struct ZZC_PJ_In_Port : SvgPort {
  ZZC_PJ_In_Port() {
    setSvg(APP->window->loadSvg(asset::plugin(pluginInstance, "res/sockets/ZZC-PJ-In.svg")));
  }
};

struct ZZC_PJ_Out_Port : SvgPort {
  ZZC_PJ_Out_Port() {
    setSvg(APP->window->loadSvg(asset::plugin(pluginInstance, "res/sockets/ZZC-PJ-Out.svg")));
  }
};

struct ZZC_PJ_Port : SvgPort {
  ZZC_PJ_Port() {
    setSvg(APP->window->loadSvg(asset::plugin(pluginInstance, "res/sockets/ZZC-PJ.svg")));
    shadow->box.size = Vec(29, 29);
    shadow->box.pos = Vec(-2, 0);
    shadow->blurRadius = 15.0f;
    shadow->opacity = 1.0f;
  }
};
