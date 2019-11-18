#include "ZZC.hpp"
#include "Clock.hpp"
#include "DivCore.hpp"

enum TransportSources {
  TS_CLOCK,
  TS_DIVIDER,
  TS_DIV,
  TS_DIVEXP,
  NUM_TRANSPORT_SOURCES
};

struct DivModuleBase : Module {
  enum OutputIds {
    CLOCK_OUTPUT,
    PHASE_OUTPUT,
    NUM_OUTPUTS
  };

  DivBase divBase;

  ZZC_TransportMessage leftMessages[2];
  ZZC_TransportMessage rightMessages[2];
  ZZC_TransportMessage cleanMessage;

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
    rightExpander.producerMessage = &rightMessages[0];
    rightExpander.consumerMessage = &rightMessages[1];
    leftExpander.producerMessage = &leftMessages[0];
    leftExpander.consumerMessage = &leftMessages[1];
    cleanMessage.hasDiv = true;
  }

  void process(const ProcessArgs &args) override;
};

void Div::process(const ProcessArgs &args) {
  bool resetWasHitForMessage = false;

  divBase.handleFractionParam(params[FRACTION_PARAM].getValue());
  divBase.handleCV(inputs[CV_INPUT].getVoltage());
  divBase.combineMultipliers();

  if (inputs[RESET_INPUT].isConnected()) {
    if (schmittTrigger.process(inputs[RESET_INPUT].getVoltage())) {
      divBase.reset();
      resetWasHitForMessage = true;
    }
  }

  if (inputs[PHASE_INPUT].isConnected()) {
    divBase.process(inputs[PHASE_INPUT].getVoltage(), args.sampleTime);
  }

  outputs[PHASE_OUTPUT].setVoltage(divBase.phaseOutput);
  outputs[CLOCK_OUTPUT].setVoltage(divBase.clockOutput);

  if (rightExpander.module &&
      (rightExpander.module->model == modelDivider ||
       rightExpander.module->model == modelDiv ||
       rightExpander.module->model == modelDivExp)) {
    ZZC_TransportMessage *message = (ZZC_TransportMessage*) rightExpander.module->leftExpander.producerMessage;
    std::memcpy(message, leftExpander.consumerMessage, sizeof(ZZC_TransportMessage));
    message->hasDiv = true;
    message->divPhase = outputs[PHASE_OUTPUT].getVoltage();
    message->divReset = resetWasHitForMessage;
    rightExpander.module->leftExpander.messageFlipRequested = true;
  }

  if (leftExpander.module &&
      (leftExpander.module->model == modelDivider ||
       leftExpander.module->model == modelDiv ||
       leftExpander.module->model == modelDivExp)) {
    ZZC_TransportMessage *message = (ZZC_TransportMessage*) leftExpander.module->rightExpander.producerMessage;
    std::memcpy(message, rightExpander.consumerMessage, sizeof(ZZC_TransportMessage));
    message->hasDiv = true;
    message->divPhase = outputs[PHASE_OUTPUT].getVoltage();
    message->divReset = resetWasHitForMessage;
    leftExpander.module->rightExpander.messageFlipRequested = true;
  }
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
  bool resetWasHitForMessage = false;
  TransportSources transportSource = TS_CLOCK;
  bool syncEnabled = true;

  DivExp() {
    config(NUM_PARAMS, NUM_INPUTS, NUM_OUTPUTS, NUM_LIGHTS);
    configParam(FRACTION_PARAM, -199.0f, 199.0f, 0.0f, "Fraction");
    configParam(SYNC_SWITCH_PARAM, 0.f, 1.f, 0.f, "Sync Fraction Changes");
    configParam(DIR_PARAM, 0.f, 1.f, 0.f, "Phase Source Search Direction");
    rightExpander.producerMessage = &rightMessages[0];
    rightExpander.consumerMessage = &rightMessages[1];
    leftExpander.producerMessage = &leftMessages[0];
    leftExpander.consumerMessage = &leftMessages[1];
    cleanMessage.hasDivExp = true;
  }

  json_t *dataToJson() override {
    json_t *rootJ = DivModuleBase::dataToJson();
    json_object_set_new(rootJ, "transportSource", json_integer(transportSource));
    json_object_set_new(rootJ, "sync", json_boolean(divBase.sync));
    return rootJ;
  }

  void dataFromJson(json_t *rootJ) override {
    DivModuleBase::dataFromJson(rootJ);
    json_t *transportSourceJ = json_object_get(rootJ, "transportSource");
    json_t *syncJ = json_object_get(rootJ, "sync");
    if (transportSourceJ) { transportSource = TransportSources(json_integer_value(transportSourceJ)); }
    if (syncJ) { divBase.sync = json_boolean_value(syncJ); }
  }

  void process(const ProcessArgs &args) override;
  void processMessageData(ZZC_TransportMessage *message , float sampleTime) {
    float phase = 0.f;
    bool reset = false;
    if (transportSource == TS_CLOCK) {
      if (!message->hasClock) { return; }
      phase = message->clockPhase;
      reset = message->clockReset;
    } else if (transportSource == TS_DIVIDER) {
      if (!message->hasDivider) { return; }
      phase = message->dividerPhase;
      reset = message->dividerReset;
    } else if (transportSource == TS_DIV) {
      if (!message->hasDiv) { return; }
      phase = message->divPhase;
      reset = message->divReset;
    } else {
      if (!message->hasDivExp) { return; }
      phase = message->divExpPhase;
      reset = message->divExpReset;
    }
    if (reset) {
      divBase.reset();
      resetWasHitForMessage = true;
    }
    divBase.process(phase, sampleTime);
    lights[DIR_LEFT_LED + params[DIR_PARAM].getValue()].value = 1.1f;
  }
};

void DivExp::process(const ProcessArgs &args) {
  if (syncButtonTriger.process(params[SYNC_SWITCH_PARAM].getValue())) {
    this->divBase.sync ^= true;
  }
  if (this->divBase.sync) {
    lights[SYNC_LED].value = 1.1f;
  }
  divBase.handleFractionParam(params[FRACTION_PARAM].getValue());
  divBase.handleCV(inputs[CV_INPUT].getVoltage());
  divBase.combineMultipliers();

  if (leftExpander.module &&
      (leftExpander.module->model == modelClock ||
       leftExpander.module->model == modelDivider ||
       leftExpander.module->model == modelDiv ||
       leftExpander.module->model == modelDivExp)) {
    ZZC_TransportMessage *message = (ZZC_TransportMessage*) leftExpander.consumerMessage;
    if (params[DIR_PARAM].getValue() == 0.f) {
      processMessageData(message, args.sampleTime);
    }
    if (rightExpander.module &&
        (rightExpander.module->model == modelDivider ||
         rightExpander.module->model == modelDiv ||
         rightExpander.module->model == modelDivExp)) {
      message = (ZZC_TransportMessage*) rightExpander.module->leftExpander.producerMessage;
      std::memcpy(message, leftExpander.consumerMessage, sizeof(ZZC_TransportMessage));
      message->hasDivExp = true;
      message->divExpPhase = divBase.phaseOutput;
      message->divExpReset = resetWasHitForMessage;
      rightExpander.module->leftExpander.messageFlipRequested = true;
    }
  }

  if (rightExpander.module &&
      (rightExpander.module->model == modelClock ||
       rightExpander.module->model == modelDivider ||
       rightExpander.module->model == modelDiv ||
       rightExpander.module->model == modelDivExp)) {
    ZZC_TransportMessage *message = (ZZC_TransportMessage*) rightExpander.consumerMessage;
    if (params[DIR_PARAM].getValue() == 1.f) {
      processMessageData(message, args.sampleTime);
    }
    if (leftExpander.module &&
        (leftExpander.module->model == modelDivider ||
         leftExpander.module->model == modelDiv ||
         leftExpander.module->model == modelDivExp)) {
      ZZC_TransportMessage *message = (ZZC_TransportMessage*) leftExpander.module->rightExpander.producerMessage;
      std::memcpy(message, rightExpander.consumerMessage, sizeof(ZZC_TransportMessage));
      message->hasDivExp = true;
      message->divExpPhase = divBase.phaseOutput;
      message->divExpReset = resetWasHitForMessage;
      leftExpander.module->rightExpander.messageFlipRequested = true;
    }
  }

  outputs[PHASE_OUTPUT].setVoltage(divBase.phaseOutput);
  outputs[CLOCK_OUTPUT].setVoltage(divBase.clockOutput);
  resetWasHitForMessage = false;
}

struct DivExpWidget : DivModuleBaseWidget {
  DivExpWidget(DivExp *module);
  void appendContextMenu(Menu *menu) override;
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
    display->blinking = &module->divBase.monoDivCore.ratioIsRequested;
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

struct TransportSourceOptionItem : MenuItem {
  DivExp *module;
  TransportSources transportSource;
  void onAction(const event::Action &e) override {
    module->transportSource = this->transportSource;
  }
};

void DivExpWidget::appendContextMenu(Menu *menu) {
  DivModuleBaseWidget::appendContextMenu(menu);
  menu->addChild(new MenuSeparator());

  DivExp *divExp = dynamic_cast<DivExp*>(module);
  assert(divExp);

  std::vector<std::string> transportSourceNames = {
    "Clock",
    "Divider",
    "Div",
    "Div (Expander)"
  };

  menu->addChild(construct<MenuLabel>(&MenuLabel::text, "Target transport source"));
  for (int i = 0; i < NUM_TRANSPORT_SOURCES; i++) {
    TransportSourceOptionItem * item = new TransportSourceOptionItem;
    item->text = transportSourceNames.at(i);
    item->transportSource = TransportSources(i);
    item->rightText = CHECKMARK(divExp->transportSource == item->transportSource);
    item->module = divExp;
    menu->addChild(item);
  }
}

Model *modelDiv = createModel<Div, DivWidget>("Div");
Model *modelDivExp = createModel<DivExp, DivExpWidget>("DivExp");
