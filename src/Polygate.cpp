#include "ZZC.hpp"

const int NUM_CHANNELS = 16;

struct Polygate : Module {
  enum ParamIds {
    ENUMS(GATE_PARAM, NUM_CHANNELS),
    NUM_PARAMS
  };
  enum InputIds {
    NUM_INPUTS
  };
  enum OutputIds {
    GATE_OUTPUT,
    NUM_OUTPUTS
  };
  enum LightIds {
    ENUMS(GATE_LED, NUM_CHANNELS),
    NUM_LIGHTS
  };

  dsp::BooleanTrigger triggers[NUM_CHANNELS];
  bool gates[NUM_CHANNELS] = { false };
  bool lastGates[NUM_CHANNELS] = { false };
  float range = 5.f;

  Polygate() {
    config(NUM_PARAMS, NUM_INPUTS, NUM_OUTPUTS, NUM_LIGHTS);
    for (int i = 0; i < NUM_CHANNELS; i++) {
      configParam(GATE_PARAM + i, 0.0f, 1.0f, 0.0f, "Channel " + std::to_string(i + 1));
    }
  }
  void process(const ProcessArgs &args) override;

  json_t *dataToJson() override {
    json_t *rootJ = json_object();
    json_t *gatesArray = json_array();
    for (int i = 0; i < NUM_CHANNELS; i++) {
      json_array_append(gatesArray, json_boolean(gates[i]));
    }
    json_object_set_new(rootJ, "gates", gatesArray);
    json_object_set_new(rootJ, "range", json_real(range));
    return rootJ;
  }

  void dataFromJson(json_t *rootJ) override {
    json_t *gatesArray = json_object_get(rootJ, "gates");
    json_t *rangeJ = json_object_get(rootJ, "range");
    if (gatesArray) {
      size_t size = json_array_size(gatesArray);
      for (size_t i = 0; i < size; i++) {
        gates[i] = json_boolean_value(json_array_get(gatesArray, i));
      }
    }
    if (rangeJ) { range = json_real_value(rangeJ); }
  }
};

void Polygate::process(const ProcessArgs &args) {
  outputs[GATE_OUTPUT].setChannels(NUM_CHANNELS);

  for (int c = 0; c < NUM_CHANNELS; c++) {
    if (triggers[c].process(params[GATE_PARAM + c].getValue())) {
      gates[c] ^= true;
    }
    if (gates[c] != lastGates[c]) {
      outputs[GATE_OUTPUT].setVoltage(gates[c] ? range : 0.f, c);
    }
    if (gates[c]) {
    lights[GATE_LED + c].value = 1.1f;
    }
  }
  std::copy(std::begin(gates), std::end(gates), std::begin(lastGates));
}

struct PolygateWidget : ModuleWidget {
  PolygateWidget(Polygate *module) {
    setModule(module);
    setPanel(APP->window->loadSvg(asset::plugin(pluginInstance, "res/panels/Polygate.svg")));

    float groupX[2] = { 10.f, 42.f };
    float groupY[2] = { 53.f, 181.f };
    float yStep = 27.f;

    for (int c = 0; c < NUM_CHANNELS; c++) {
      float x = groupX[c / 8];
      float y = groupY[c % 8 / 4] + yStep * (c % 4);
      addParam(createParam<ZZC_LEDBezelDark>(Vec(x + 0.3f, y + 0.0f), module, Polygate::GATE_PARAM + c));
      addChild(createLight<LedLight<ZZC_YellowLight>>(Vec(x + 2.1f, y + 1.7f), module, Polygate::GATE_LED + c));
    }

    addOutput(createOutput<ZZC_PJ_Port>(Vec(25, 320), module, Polygate::GATE_OUTPUT));

    addChild(createWidget<ZZC_Screw>(Vec(RACK_GRID_WIDTH, 0)));
    addChild(createWidget<ZZC_Screw>(Vec(box.size.x - 2 * RACK_GRID_WIDTH, 0)));
    addChild(createWidget<ZZC_Screw>(Vec(RACK_GRID_WIDTH, RACK_GRID_HEIGHT - RACK_GRID_WIDTH)));
    addChild(createWidget<ZZC_Screw>(Vec(box.size.x - 2 * RACK_GRID_WIDTH, RACK_GRID_HEIGHT - RACK_GRID_WIDTH)));
  }
};

Model *modelPolygate = createModel<Polygate, PolygateWidget>("Polygate");
