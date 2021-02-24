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
  addChild(createWidget<ZZC_Screw>(Vec(RACK_GRID_WIDTH, 0)));
  addChild(createWidget<ZZC_Screw>(Vec(box.size.x - 2 * RACK_GRID_WIDTH, 0)));
  addChild(createWidget<ZZC_Screw>(Vec(RACK_GRID_WIDTH, RACK_GRID_HEIGHT - RACK_GRID_WIDTH)));
  addChild(createWidget<ZZC_Screw>(Vec(box.size.x - 2 * RACK_GRID_WIDTH, RACK_GRID_HEIGHT - RACK_GRID_WIDTH)));
}

void PhasorWidget::appendContextMenu(Menu *menu) {
}

Model *modelPhasor = createModel<Phasor, PhasorWidget>("Phasor");