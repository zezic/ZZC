#include "rack0.hpp"

using namespace rack;

extern Plugin *pluginInstance;

struct ZZC_Switch2 : SvgSwitch {
  ZZC_Switch2() {
    addFrame(APP->window->loadSvg(asset::plugin(pluginInstance,"res/switches/ZZC-Switch-2_0.svg")));
    addFrame(APP->window->loadSvg(asset::plugin(pluginInstance,"res/switches/ZZC-Switch-2_1.svg")));
  }
};

struct ZZC_Switch3 : SvgSwitch {
  ZZC_Switch3() {
    addFrame(APP->window->loadSvg(asset::plugin(pluginInstance,"res/switches/ZZC-Switch-3_0.svg")));
    addFrame(APP->window->loadSvg(asset::plugin(pluginInstance,"res/switches/ZZC-Switch-3_1.svg")));
    addFrame(APP->window->loadSvg(asset::plugin(pluginInstance,"res/switches/ZZC-Switch-3_2.svg")));
  }
};

struct ZZC_FN3WaveSwitch : SvgSwitch {
  ZZC_FN3WaveSwitch() {
    addFrame(APP->window->loadSvg(asset::plugin(pluginInstance,"res/switches/ZZC-FN-3-Wave-Switch_0.svg")));
    addFrame(APP->window->loadSvg(asset::plugin(pluginInstance,"res/switches/ZZC-FN-3-Wave-Switch_1.svg")));
    addFrame(APP->window->loadSvg(asset::plugin(pluginInstance,"res/switches/ZZC-FN-3-Wave-Switch_2.svg")));
    shadow->opacity = 0.f;
  }
};

struct ZZC_FN3UniBiSwitch : SvgSwitch {
  ZZC_FN3UniBiSwitch() {
    addFrame(APP->window->loadSvg(asset::plugin(pluginInstance,"res/switches/ZZC-FN-3-Uni-Bi-Switch_0.svg")));
    addFrame(APP->window->loadSvg(asset::plugin(pluginInstance,"res/switches/ZZC-FN-3-Uni-Bi-Switch_1.svg")));
    shadow->opacity = 0.f;
  }
};

struct ZZC_LEDBezelDark : SvgSwitch {
  ZZC_LEDBezelDark() {
    momentary = true;
    addFrame(APP->window->loadSvg(asset::plugin(pluginInstance, "res/switches/ZZC-LED-Bezel-Dark.svg")));
    shadow->opacity = 0.0f;
  }
};
