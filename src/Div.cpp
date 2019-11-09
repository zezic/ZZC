#include "ZZC.hpp"
#include "DivCore.hpp"

struct Div : Module {
  enum ParamIds {
    FRACTION_PARAM,
    NUM_PARAMS
  };
  enum InputIds {
    CV_INPUT,
    PHASE_INPUT,
    RESET_INPUT,
    NUM_INPUTS
  };
  enum OutputIds {
    CLOCK_OUTPUT,
    PHASE_OUTPUT,
    NUM_OUTPUTS
  };
  enum LightIds {
    NUM_LIGHTS
  };

  DivCore<float_4> divCore[4];

  int fractionDisplay = 1;

  Div() {
    config(NUM_PARAMS, NUM_INPUTS, NUM_OUTPUTS, NUM_LIGHTS);
    configParam(FRACTION_PARAM, -199.0f, 199.0f, 0.0f, "Fraction");
  }

  void process(const ProcessArgs &args) override;
};

void Div::process(const ProcessArgs &args) {
  fractionDisplay = std::max((int) abs(params[FRACTION_PARAM].getValue()), 1);

  simd::float_4 phaseInValue[4];
  if (inputs[PHASE_INPUT].isConnected()) {
    if (inputs[PHASE_INPUT].isPolyphonic()) {
      int channels = inputs[PHASE_INPUT].getChannels();
      outputs[PHASE_OUTPUT].setChannels(channels);
      for (int c = 0; c < channels; c += 4) {
        phaseInValue[c / 4] = simd::float_4::load(inputs[PHASE_INPUT].getVoltages(c)) / 10.f;
        divCore[c / 4].process(phaseInValue[c / 4]);
        divCore[c / 4].phase10.store(outputs[PHASE_OUTPUT].getVoltages(c));
      }
    }
  }
}

struct DivWidget : ModuleWidget {
  DivWidget(Div *module);
};

DivWidget::DivWidget(Div *module) {
  setModule(module);
  setPanel(APP->window->loadSvg(asset::plugin(pluginInstance, "res/panels/Div.svg")));

  addParam(createParam<ZZC_Knob27Blind>(Vec(9, 58), module, Div::FRACTION_PARAM));

  IntDisplayWidget *display = new IntDisplayWidget();

  display->box.pos = Vec(6, 94);
  display->box.size = Vec(33, 21);
  display->textGhost = "188";
  if (module) {
    display->value = &module->fractionDisplay;
  }
  addChild(display);

  addInput(createInput<ZZC_PJ_Port>(Vec(10.5, 124), module, Div::CV_INPUT));
  addInput(createInput<ZZC_PJ_Port>(Vec(10.5, 181), module, Div::PHASE_INPUT));
  addInput(createInput<ZZC_PJ_Port>(Vec(10.5, 224), module, Div::RESET_INPUT));

  addOutput(createOutput<ZZC_PJ_Port>(Vec(10.5, 272), module, Div::CLOCK_OUTPUT));
  addOutput(createOutput<ZZC_PJ_Port>(Vec(10.5, 320), module, Div::PHASE_OUTPUT));

  addChild(createWidget<ZZC_Screw>(Vec(RACK_GRID_WIDTH, 0)));
  addChild(createWidget<ZZC_Screw>(Vec(RACK_GRID_WIDTH, RACK_GRID_HEIGHT - RACK_GRID_WIDTH)));
}

Model *modelDiv = createModel<Div, DivWidget>("Div");
