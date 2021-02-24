#include "ZZC.hpp"
#include "Phasor.hpp"

Phasor::Phasor() {
  config(NUM_PARAMS, NUM_INPUTS, NUM_OUTPUTS, NUM_LIGHTS);
}

json_t *Phasor::dataToJson() {
  json_t *rootJ = json_object();
  return rootJ;
}

void Phasor::dataFromJson(json_t *rootJ) {
}

void Phasor::process(const ProcessArgs &args) {
}

struct PhasorWidget : ModuleWidget {
  PhasorWidget(Phasor *module);
  void appendContextMenu(Menu *menu) override;
};

PhasorWidget::PhasorWidget(Phasor *module) {
  setModule(module);
  setPanel(APP->window->loadSvg(asset::plugin(pluginInstance, "res/panels/Phasor.svg")));

  addParam(createParam<ZZC_CrossKnob45>(Vec(30.5f, 54.5f), module, Phasor::FREQ_CRSE_PARAM));
  addParam(createParam<ZZC_Switch2Vertical>(Vec(12.f, 71.f), module, Phasor::REVERSE_PARAM));
  addParam(createParam<ZZC_Switch2Vertical>(Vec(93.f, 71.f), module, Phasor::LFO_PARAM));

  addParam(createParam<ZZC_SelectKnob>(Vec(10.f, 138.828f), module, Phasor::NUMERATOR_PARAM));
  addParam(createParam<ZZC_SelectKnob>(Vec(83.f, 138.828f), module, Phasor::DENOMINATOR_PARAM));

  addInput(createInput<ZZC_PJ_Port>(Vec(12.164f, 196.07f), module, Phasor::SYNC_INPUT));

  addParam(createParam<ZZC_LEDBezelDark>(Vec(84.586f, 197.32f), module, Phasor::HARDSYNC_PARAM));
  addChild(createLight<LedLight<ZZC_YellowLight>>(Vec(86.386f, 199.12f), module, Phasor::HARDSYNC_LED));

  addParam(createParam<ZZC_CrossKnob29>(Vec(39.5f, 209.173f), module, Phasor::FREQ_FINE_PARAM));

  addChild(createWidget<ZZC_Screw>(Vec(RACK_GRID_WIDTH, 0)));
  addChild(createWidget<ZZC_Screw>(Vec(box.size.x - 2 * RACK_GRID_WIDTH, 0)));
  addChild(createWidget<ZZC_Screw>(Vec(RACK_GRID_WIDTH, RACK_GRID_HEIGHT - RACK_GRID_WIDTH)));
  addChild(createWidget<ZZC_Screw>(Vec(box.size.x - 2 * RACK_GRID_WIDTH, RACK_GRID_HEIGHT - RACK_GRID_WIDTH)));
}

void PhasorWidget::appendContextMenu(Menu *menu) {
}

Model *modelPhasor = createModel<Phasor, PhasorWidget>("Phasor");