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

  MonoDiv monoDiv;
  SchmittTrigger schmittTrigger;
  PulseGenerator pulseGenerator;

  int fractionDisplay = 1;
  int fractionDisplayPolarity = 0;

  /* Settings */
  bool gateMode = false;

  Div() {
    config(NUM_PARAMS, NUM_INPUTS, NUM_OUTPUTS, NUM_LIGHTS);
    configParam(FRACTION_PARAM, -199.0f, 199.0f, 0.0f, "Fraction");
  }

  void process(const ProcessArgs &args) override;

  json_t *dataToJson() override {
    json_t *rootJ = json_object();
    json_object_set_new(rootJ, "gateMode", json_boolean(gateMode));
    return rootJ;
  }

  void dataFromJson(json_t *rootJ) override {
    json_t *gateModeJ = json_object_get(rootJ, "gateMode");
    if (gateModeJ) { gateMode = json_boolean_value(gateModeJ); }
  }
};

void Div::process(const ProcessArgs &args) {
  float fractionParam = std::trunc(params[FRACTION_PARAM].getValue());
  float fractionAbs = std::max(1.f, std::abs(fractionParam));


  float paramMultiplier = fractionParam >= 0.f ? fractionAbs : 1.f / fractionAbs;


  if (inputs[RESET_INPUT].isConnected()) {
    if (schmittTrigger.process(inputs[RESET_INPUT].getVoltage())) {
      monoDiv.reset();
    }
  }

  if (inputs[PHASE_INPUT].isConnected()) {
    float combinedMultiplier = paramMultiplier;
    if (inputs[CV_INPUT].isConnected()) {
      float cvVoltage = inputs[CV_INPUT].getVoltage();

      // Not sure how to make it output 2 when cvVoltage is 1
      float cvMultiplier = dsp::approxExp2_taylor5(cvVoltage + 20) / 1048576;
      // float cvMultiplier = std::pow(2.f, cvVoltage);
      combinedMultiplier *= cvMultiplier;
    }
    float combinedMultiplierLo = 1.f / std::round(1.f / combinedMultiplier);
    float combinedMultiplierHi = std::round(combinedMultiplier);

    float roundedMultiplier = combinedMultiplier < 1.f ? combinedMultiplierLo : combinedMultiplierHi;
    // core->multiplier = clamp(roundedMultiplier, 0.f, 199.f);
    monoDiv.ratio = clamp(roundedMultiplier, 0.f, 199.f);

    bool flipped = monoDiv.process(inputs[PHASE_INPUT].getVoltage());

    if (gateMode) {
      outputs[CLOCK_OUTPUT].setVoltage(monoDiv.phase < 5.0 ? 10.f : 0.f);
    } else {
      if (flipped) {
        pulseGenerator.trigger(1e-3f);
      }
      outputs[CLOCK_OUTPUT].setVoltage(pulseGenerator.process(args.sampleTime) ? 10.f : 0.f);
    }
    outputs[PHASE_OUTPUT].setVoltage(monoDiv.phase);
  }

  // Manage fraction display
  float multiplier = monoDiv.ratio;
  if (multiplier == 1.f) {
    fractionDisplay = 1;
    fractionDisplayPolarity = 0;
  } else if (multiplier > 1.f) {
    fractionDisplay = std::round(multiplier);
    fractionDisplayPolarity = 1;
  } else {
    fractionDisplay = std::round(clamp(1.f / multiplier, 1.f, 199.f));
    fractionDisplayPolarity = -1;
  }

}

struct DivWidget : ModuleWidget {
  DivWidget(Div *module);
  void appendContextMenu(Menu *menu) override;
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
    display->polarity = &module->fractionDisplayPolarity;
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

struct DivGateModeItem : MenuItem {
  Div *div;
  void onAction(const event::Action &e) override {
    div->gateMode ^= true;
  }
  void step() override {
    rightText = CHECKMARK(div->gateMode);
  }
};

void DivWidget::appendContextMenu(Menu *menu) {
  menu->addChild(new MenuSeparator());

  Div *div = dynamic_cast<Div*>(module);
  assert(div);

  DivGateModeItem *gateModeItem = createMenuItem<DivGateModeItem>("Gate Mode");
  gateModeItem->div = div;
  menu->addChild(gateModeItem);
}

Model *modelDiv = createModel<Div, DivWidget>("Div");
