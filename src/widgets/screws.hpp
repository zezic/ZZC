using namespace rack;

extern Plugin *pluginInstance;

struct ZZC_Screw : SvgScrew {
  ZZC_Screw() {
    setSvg(APP->window->loadSvg(asset::plugin(pluginInstance, "res/screws/ZZC-Screw.svg")));
    box.size = sw->box.size;
  }
};
