#include "rack0.hpp"

using namespace rack;

extern Plugin *pluginInstance;

struct ZZC_Switch2 : SVGSwitch, ToggleSwitch {
  ZZC_Switch2() {
    addFrame(SVG::load(assetPlugin(pluginInstance,"res/switches/ZZC-Switch-2_0.svg")));
    addFrame(SVG::load(assetPlugin(pluginInstance,"res/switches/ZZC-Switch-2_1.svg")));
  }
};

struct ZZC_Switch3 : SVGSwitch, ToggleSwitch {
  ZZC_Switch3() {
    addFrame(SVG::load(assetPlugin(pluginInstance,"res/switches/ZZC-Switch-3_0.svg")));
    addFrame(SVG::load(assetPlugin(pluginInstance,"res/switches/ZZC-Switch-3_1.svg")));
    addFrame(SVG::load(assetPlugin(pluginInstance,"res/switches/ZZC-Switch-3_2.svg")));
  }
};

struct ZZC_FN3WaveSwitch : SVGSwitch, ToggleSwitch {
  ZZC_FN3WaveSwitch() {
    addFrame(SVG::load(assetPlugin(pluginInstance,"res/switches/ZZC-FN-3-Wave-Switch_0.svg")));
    addFrame(SVG::load(assetPlugin(pluginInstance,"res/switches/ZZC-FN-3-Wave-Switch_1.svg")));
    addFrame(SVG::load(assetPlugin(pluginInstance,"res/switches/ZZC-FN-3-Wave-Switch_2.svg")));
  }
};

struct ZZC_FN3UniBiSwitch : SVGSwitch, ToggleSwitch {
  ZZC_FN3UniBiSwitch() {
    addFrame(SVG::load(assetPlugin(pluginInstance,"res/switches/ZZC-FN-3-Uni-Bi-Switch_0.svg")));
    addFrame(SVG::load(assetPlugin(pluginInstance,"res/switches/ZZC-FN-3-Uni-Bi-Switch_1.svg")));
  }
};

struct ZZC_LEDBezelDark : SVGSwitch, MomentarySwitch {
  ZZC_LEDBezelDark() {
    addFrame(SVG::load(assetPlugin(pluginInstance, "res/switches/ZZC-LED-Bezel-Dark.svg")));
  }
};
