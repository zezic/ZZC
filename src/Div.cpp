#include "ZZC.hpp"
#include "DivCore.hpp"

struct DivBase {
  float clockOutput = 0.f;
  float phaseOutput = 0.f;

  float combinedMultiplier = 1.f;
  bool combinedMultiplierDirty = false;
  float roundedMultiplier = 1.f;

  float paramMultiplier = 1.f;
  float cvMultiplier = 1.f;

  float lastParamMultiplier = 1.f;
  float lastCVMultiplier = 1.f;

  float lastFractionParam = 1.f;
  float lastCVVoltage = 0.f;

  MonoDivCore monoDivCore;
  PulseGenerator pulseGenerator;

  int fractionDisplay = 1;
  int fractionDisplayPolarity = 0;

  /* Settings */
  bool gateMode = false;

  void reset() {
    this->monoDivCore.reset();
  }

  void process(float phaseIn, float sampleTime) {
    this->combineMultipliers();
    monoDivCore.ratio = roundedMultiplier;

    bool flipped = monoDivCore.process(phaseIn);

    if (this->gateMode) {
      clockOutput = monoDivCore.phase < 5.0 ? 10.f : 0.f;
    } else {
      if (flipped) {
        pulseGenerator.trigger(1e-3f);
      }
      clockOutput = pulseGenerator.process(sampleTime) ? 10.f : 0.f;
    }
    phaseOutput = monoDivCore.phase;
  }

  void handleFractionParam(float value) {
    if (value == lastFractionParam) { return; }
    float fractionParam = trunc(value);
    float fractionAbs = std::max(1.f, abs(fractionParam));
    this->paramMultiplier = fractionParam >= 0.f ? fractionAbs : 1.f / fractionAbs;
    this->lastFractionParam = value;
    this->combinedMultiplierDirty = true;
  }

  void handleCV(float cvVoltage) {
    if (cvVoltage == lastCVVoltage) { return; }
    this->cvMultiplier = dsp::approxExp2_taylor5(cvVoltage + 20) / 1048576;
    this->lastCVVoltage = cvVoltage;
    this->combinedMultiplierDirty = true;
  }

  void combineMultipliers() {
    if (!this->combinedMultiplierDirty) { return; }
    this->combinedMultiplier = paramMultiplier * cvMultiplier;

    float combinedMultiplierLo = 1.f / roundf(1.f / this->combinedMultiplier);
    float combinedMultiplierHi = roundf(this->combinedMultiplier);

    this->roundedMultiplier = clamp(this->combinedMultiplier < 1.f ? combinedMultiplierLo : combinedMultiplierHi, 0.f, 199.f);
    this->combinedMultiplierDirty = false;

    // Manage fraction display
    if (this->roundedMultiplier == 1.f) {
      this->fractionDisplay = 1;
      this->fractionDisplayPolarity = 0;
    } else if (this->roundedMultiplier > 1.f) {
      this->fractionDisplay = roundf(this->roundedMultiplier);
      this->fractionDisplayPolarity = 1;
    } else {
      this->fractionDisplay = roundf(clamp(1.f / this->roundedMultiplier, 1.f, 199.f));
      this->fractionDisplayPolarity = -1;
    }
  }
};

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

  DivBase divBase;

  SchmittTrigger schmittTrigger;

  Div() {
    config(NUM_PARAMS, NUM_INPUTS, NUM_OUTPUTS, NUM_LIGHTS);
    configParam(FRACTION_PARAM, -199.0f, 199.0f, 0.0f, "Fraction");
  }

  void process(const ProcessArgs &args) override;

  json_t *dataToJson() override {
    json_t *rootJ = json_object();
    json_object_set_new(rootJ, "gateMode", json_boolean(divBase.gateMode));
    return rootJ;
  }

  void dataFromJson(json_t *rootJ) override {
    json_t *gateModeJ = json_object_get(rootJ, "gateMode");
    if (gateModeJ) { divBase.gateMode = json_boolean_value(gateModeJ); }
  }
};

void Div::process(const ProcessArgs &args) {
  divBase.handleFractionParam(params[FRACTION_PARAM].getValue());

  if (inputs[RESET_INPUT].isConnected()) {
    if (schmittTrigger.process(inputs[RESET_INPUT].getVoltage())) {
      divBase.reset();
    }
  }

  if (inputs[CV_INPUT].isConnected()) {
    divBase.handleCV(inputs[CV_INPUT].getVoltage());
  }

  if (inputs[PHASE_INPUT].isConnected()) {
    divBase.process(inputs[PHASE_INPUT].getVoltage(), args.sampleTime);
  }

  outputs[PHASE_OUTPUT].setVoltage(divBase.phaseOutput);
  outputs[CLOCK_OUTPUT].setVoltage(divBase.clockOutput);
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
    display->value = &module->divBase.fractionDisplay;
    display->polarity = &module->divBase.fractionDisplayPolarity;
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
    div->divBase.gateMode ^= true;
  }
  void step() override {
    rightText = CHECKMARK(div->divBase.gateMode);
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
