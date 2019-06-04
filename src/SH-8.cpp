#include "ZZC.hpp"

const int NUM_CHANNELS = 8;

struct SH8 : Module {
  enum ParamIds {
    NUM_PARAMS
  };
  enum InputIds {
    NOISE_INPUT,
    ENUMS(TRIG_INPUT, NUM_CHANNELS),
    NUM_INPUTS
  };
  enum OutputIds {
    ENUMS(HOLD_OUTPUT, NUM_CHANNELS),
    NUM_OUTPUTS
  };
  enum LightIds {
    NUM_LIGHTS
  };

  float last_trig_inputs[NUM_CHANNELS];
  dsp::SchmittTrigger triggers[NUM_CHANNELS];

  SH8() {
    config(NUM_PARAMS, NUM_INPUTS, NUM_OUTPUTS, NUM_LIGHTS);
  }
  void process(const ProcessArgs &args) override;
};


void SH8::process(const ProcessArgs &args) {
  bool previousIsTriggered = false;
  for (int i = 0; i < NUM_CHANNELS; i++) {
    if (inputs[TRIG_INPUT + i].isConnected()) {
      if (triggers[i].process(inputs[TRIG_INPUT + i].getVoltage())) {
        outputs[HOLD_OUTPUT + i].setVoltage(inputs[NOISE_INPUT].isConnected() ? inputs[NOISE_INPUT].getVoltage() : random::normal() * 2.0);
        previousIsTriggered = true;
      } else {
        previousIsTriggered = false;
      }
    } else {
      if (i > 0 && previousIsTriggered) {
        outputs[HOLD_OUTPUT + i].setVoltage(inputs[NOISE_INPUT].isConnected() ? inputs[NOISE_INPUT].getVoltage() : random::normal() * 2.0);
      }
    }
  }
}


struct SH8Widget : ModuleWidget {
  SH8Widget(SH8 *module) {
    setModule(module);
    setPanel(APP->window->loadSvg(asset::plugin(pluginInstance, "res/panels/SH-8.svg")));

    addInput(createInput<ZZC_PJ_Port>(Vec(25, 53), module, SH8::NOISE_INPUT));

    for (int i = 0; i < NUM_CHANNELS; i++) {
      addInput(createInput<ZZC_PJ_Port>(Vec(7.25f, 109 + 30 * i), module, SH8::TRIG_INPUT + i));
    }

    for (int i = 0; i < NUM_CHANNELS; i++) {
      addOutput(createOutput<ZZC_PJ_Port>(Vec(42.25f, 109 + 30 * i), module, SH8::HOLD_OUTPUT + i));
    }

    addChild(createWidget<ZZC_Screw>(Vec(RACK_GRID_WIDTH, 0)));
    addChild(createWidget<ZZC_Screw>(Vec(box.size.x - 2 * RACK_GRID_WIDTH, 0)));
    addChild(createWidget<ZZC_Screw>(Vec(RACK_GRID_WIDTH, RACK_GRID_HEIGHT - RACK_GRID_WIDTH)));
    addChild(createWidget<ZZC_Screw>(Vec(box.size.x - 2 * RACK_GRID_WIDTH, RACK_GRID_HEIGHT - RACK_GRID_WIDTH)));
  }
};


Model *modelSH8 = createModel<SH8, SH8Widget>("SH-8");
