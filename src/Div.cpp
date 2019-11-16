#include "ZZC.hpp"
#include "Clock.hpp"
#include "DivCore.hpp"

struct DivModuleBase : Module {
  enum OutputIds {
    CLOCK_OUTPUT,
    PHASE_OUTPUT,
    NUM_OUTPUTS
  };

  DivBase divBase;

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

struct DivModuleBaseWidget : ModuleWidget {
  void appendContextMenu(Menu *menu) override;
};

struct Div : DivModuleBase {
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
  enum LightIds {
    NUM_LIGHTS
  };

  SchmittTrigger schmittTrigger;

  Div() {
    config(NUM_PARAMS, NUM_INPUTS, NUM_OUTPUTS, NUM_LIGHTS);
    configParam(FRACTION_PARAM, -199.0f, 199.0f, 0.0f, "Fraction");
  }

  void process(const ProcessArgs &args) override;
};

void Div::process(const ProcessArgs &args) {
  divBase.handleFractionParam(params[FRACTION_PARAM].getValue());
  divBase.handleCV(inputs[CV_INPUT].getVoltage());
  divBase.combineMultipliers();

  if (inputs[RESET_INPUT].isConnected()) {
    if (schmittTrigger.process(inputs[RESET_INPUT].getVoltage())) {
      divBase.reset();
    }
  }

  if (inputs[PHASE_INPUT].isConnected()) {
    divBase.process(inputs[PHASE_INPUT].getVoltage(), args.sampleTime);
  }

  outputs[PHASE_OUTPUT].setVoltage(divBase.phaseOutput);
  outputs[CLOCK_OUTPUT].setVoltage(divBase.clockOutput);
}

struct DivWidget : DivModuleBaseWidget {
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

struct DivExp : DivModuleBase {
  enum ParamIds {
    FRACTION_PARAM,
    SYNC_SWITCH_PARAM,
    DIR_PARAM,
    NUM_PARAMS
  };
  enum InputIds {
    CV_INPUT,
    PHASE_INPUT,
    RESET_INPUT,
    NUM_INPUTS
  };
  enum LightIds {
    SYNC_LED,
    DIR_LEFT_LED,
    DIR_RIGHT_LED,
    NUM_LIGHTS
  };

  SchmittTrigger syncButtonTriger;

  DivExp() {
    config(NUM_PARAMS, NUM_INPUTS, NUM_OUTPUTS, NUM_LIGHTS);
    configParam(FRACTION_PARAM, -199.0f, 199.0f, 0.0f, "Fraction");
    configParam(SYNC_SWITCH_PARAM, 0.f, 1.f, 0.f, "Sync Fraction Changes");
    configParam(DIR_PARAM, 0.f, 1.f, 0.f, "Phase Source Search Direction");
  }

  void process(const ProcessArgs &args) override;
};

void DivExp::process(const ProcessArgs &args) {
  // if (leftExpander.module && leftExpander.module->model == modelClock) {
  //   Clock *clock = reinterpret_cast<Clock*>(leftExpander.module);
  // }

  if (syncButtonTriger.process(params[SYNC_SWITCH_PARAM].getValue())) {
    this->divBase.sync ^= true;
  }
  if (this->divBase.sync) {
    lights[SYNC_LED].value = 1.1f;
  }
  divBase.handleFractionParam(params[FRACTION_PARAM].getValue());
  divBase.handleCV(inputs[CV_INPUT].getVoltage());
  divBase.combineMultipliers();

  outputs[PHASE_OUTPUT].setVoltage(divBase.phaseOutput);
  outputs[CLOCK_OUTPUT].setVoltage(divBase.clockOutput);
}

struct DivExpWidget : DivModuleBaseWidget {
  DivExpWidget(DivExp *module);
};

DivExpWidget::DivExpWidget(DivExp *module) {
  setModule(module);
  setPanel(APP->window->loadSvg(asset::plugin(pluginInstance, "res/panels/DivExp.svg")));

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

  addInput(createInput<ZZC_PJ_Port>(Vec(10.5, 124), module, DivExp::CV_INPUT));

  addParam(createParam<ZZC_LEDBezelDark>(Vec(11.3f, 172.0f), module, DivExp::SYNC_SWITCH_PARAM));
  addChild(createLight<LedLight<ZZC_YellowLight>>(Vec(13.1f, 173.7f), module, DivExp::SYNC_LED));

  addParam(createParam<ZZC_Switch2>(Vec(12.f, 204.f), module, DivExp::DIR_PARAM));

  addChild(createLight<SmallLight<ZZC_YellowLight>>(Vec(12.25f, 236.5f), module, DivExp::DIR_LEFT_LED));
  addChild(createLight<SmallLight<ZZC_YellowLight>>(Vec(26.5f, 236.5f), module, DivExp::DIR_RIGHT_LED));

  addOutput(createOutput<ZZC_PJ_Port>(Vec(10.5, 272), module, DivExp::CLOCK_OUTPUT));
  addOutput(createOutput<ZZC_PJ_Port>(Vec(10.5, 320), module, DivExp::PHASE_OUTPUT));

  addChild(createWidget<ZZC_Screw>(Vec(RACK_GRID_WIDTH, 0)));
  addChild(createWidget<ZZC_Screw>(Vec(RACK_GRID_WIDTH, RACK_GRID_HEIGHT - RACK_GRID_WIDTH)));
}

struct DivGateModeItem : MenuItem {
  DivModuleBase *div;
  void onAction(const event::Action &e) override {
    div->divBase.gateMode ^= true;
  }
  void step() override {
    rightText = CHECKMARK(div->divBase.gateMode);
  }
};

void DivModuleBaseWidget::appendContextMenu(Menu *menu) {
  menu->addChild(new MenuSeparator());

  DivModuleBase *div = dynamic_cast<DivModuleBase*>(module);
  assert(div);

  DivGateModeItem *gateModeItem = createMenuItem<DivGateModeItem>("Gate Mode");
  gateModeItem->div = div;
  menu->addChild(gateModeItem);
}

Model *modelDiv = createModel<Div, DivWidget>("Div");
Model *modelDivExp = createModel<DivExp, DivExpWidget>("DivExp");
