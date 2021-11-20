#include "ZZC.hpp"
#include "Clock.hpp"
#include "PhaseDivider.hpp"

enum TransportSources {
  TS_CLOCK,
  TS_DIVIDER,
  TS_DIV,
  TS_DIVEXP,
  TS_NONE,
  NUM_TRANSPORT_SOURCES
};

struct DivBase : Module {
  enum OutputIds {
    CLOCK_OUTPUT,
    PHASE_OUTPUT,
    NUM_OUTPUTS
  };

  PhaseDivider phaseDivider;
  dsp::PulseGenerator pulseGenerator;

  float combinedMultiplier = 1.f;
  bool combinedMultiplierDirty = false;
  float roundedMultiplier = 1.f;

  float paramMultiplier = 1.f;
  float cvMultiplier = 1.f;

  float lastParamMultiplier = 1.f;
  float lastCVMultiplier = 1.f;

  float lastFractionParam = 1.f;
  float lastCVVoltage = 0.f;

  int fractionDisplay = 1;
  int fractionDisplayPolarity = 0;

  ZZC_TransportMessage leftMessages[2];
  ZZC_TransportMessage rightMessages[2];
  ZZC_TransportMessage cleanMessage;

  /* Settings */
  bool gateMode = true;

  json_t *dataToJson() override {
    json_t *rootJ = json_object();
    json_object_set_new(rootJ, "gateMode", json_boolean(this->gateMode));
    return rootJ;
  }

  void dataFromJson(json_t *rootJ) override {
    json_t *gateModeJ = json_object_get(rootJ, "gateMode");
    if (gateModeJ) { this->gateMode = json_boolean_value(gateModeJ); }
  }

  void handleFractionParam(float value);
  void handleCV(float cvVoltage);
  void combineMultipliers();
};

void DivBase::handleFractionParam(float value) {
  if (value == lastFractionParam) { return; }
  float fractionParam = trunc(value);
  float fractionAbs = std::max(1.f, (float) abs(fractionParam));
  this->paramMultiplier = fractionParam >= 0.f ? fractionAbs : 1.f / fractionAbs;
  this->lastFractionParam = value;
  this->combinedMultiplierDirty = true;
}

void DivBase::handleCV(float cvVoltage) {
  if (cvVoltage == lastCVVoltage) { return; }
  this->cvMultiplier = dsp::approxExp2_taylor5(cvVoltage + 20) / 1048576;
  this->lastCVVoltage = cvVoltage;
  this->combinedMultiplierDirty = true;
}

void DivBase::combineMultipliers() {
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

struct DivModuleBaseWidget : ModuleWidget {
  void appendContextMenu(Menu *menu) override;
};

struct Div : DivBase {
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

  dsp::SchmittTrigger schmittTrigger;

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

  this->handleFractionParam(params[FRACTION_PARAM].getValue());
  this->handleCV(inputs[CV_INPUT].getVoltage());
  this->combineMultipliers();
  this->phaseDivider.setRatio(this->roundedMultiplier);

  if (inputs[RESET_INPUT].isConnected()) {
    if (schmittTrigger.process(inputs[RESET_INPUT].getVoltage())) {
      this->phaseDivider.reset();
      resetWasHitForMessage = true;
    }
  }

  if (inputs[PHASE_INPUT].isConnected()) {
    bool flipped = this->phaseDivider.process(inputs[PHASE_INPUT].getVoltage());
    if (flipped && !this->gateMode) {
      this->pulseGenerator.trigger(1e-3f);
    }
  }

  outputs[PHASE_OUTPUT].setVoltage(this->phaseDivider.phase);
  outputs[CLOCK_OUTPUT].setVoltage(
    (this->gateMode ? this->phaseDivider.phase < 5.0 : this->pulseGenerator.process(args.sampleTime)) ? 10.f : 0.f
  );

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

struct DivExp : DivBase {
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

  dsp::SchmittTrigger syncButtonTriger;
  bool resetWasHitForMessage = false;
  TransportSources transportSource = TS_CLOCK;
  TransportSources lastTimeDrivenBy = TS_NONE;
  bool syncing = false;
  bool firstCycle = true;
  bool lastSyncState = false;

  /* Settings */
  bool sync = false;

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
    json_t *rootJ = DivBase::dataToJson();
    json_object_set_new(rootJ, "transportSource", json_integer(this->transportSource));
    json_object_set_new(rootJ, "sync", json_boolean(this->sync));
    return rootJ;
  }

  void dataFromJson(json_t *rootJ) override {
    DivBase::dataFromJson(rootJ);
    json_t *transportSourceJ = json_object_get(rootJ, "transportSource");
    json_t *syncJ = json_object_get(rootJ, "sync");
    if (transportSourceJ) { this->transportSource = TransportSources(json_integer_value(transportSourceJ)); }
    if (syncJ) {
      this->sync = json_boolean_value(syncJ);
      this->lastSyncState = this->sync;
    }
  }

  void process(const ProcessArgs &args) override;
  bool processMessageData(ZZC_TransportMessage *message , float sampleTime);
};

bool DivExp::processMessageData(ZZC_TransportMessage *message , float sampleTime) {
  float phase = 0.f;
  bool reset = false;
  if (transportSource == TS_CLOCK) {
    if (!message->hasClock) { return false; }
    phase = message->clockPhase;
    reset = message->clockReset;
  } else if (transportSource == TS_DIVIDER) {
    if (!message->hasDivider) { return false; }
    phase = message->dividerPhase;
    reset = message->dividerReset;
  } else if (transportSource == TS_DIV) {
    if (!message->hasDiv) { return false; }
    phase = message->divPhase;
    reset = message->divReset;
  } else {
    if (!message->hasDivExp) { return false; }
    phase = message->divExpPhase;
    reset = message->divExpReset;
  }
  if (reset) {
    this->phaseDivider.reset();
    resetWasHitForMessage = true;
  }
  if ((this->transportSource != this->lastTimeDrivenBy) ||
      (this->sync && !this->lastSyncState)){
    this->phaseDivider.requestHardSync();
  }
  bool flipped = this->phaseDivider.process(phase);
  if (flipped && !this->gateMode) {
    this->pulseGenerator.trigger(1e-3f);
  }
  lights[DIR_LEFT_LED + params[DIR_PARAM].getValue()].value = 1.1f;
  return true;
}

void DivExp::process(const ProcessArgs &args) {
  if (syncButtonTriger.process(params[SYNC_SWITCH_PARAM].getValue())) {
    this->sync ^= true;
  }
  if (this->sync) {
    lights[SYNC_LED].value = 1.1f;
  }
  this->handleFractionParam(params[FRACTION_PARAM].getValue());
  this->handleCV(inputs[CV_INPUT].getVoltage());
  this->combineMultipliers();
  if (this->sync && !this->firstCycle) {
    this->phaseDivider.requestRatio(this->roundedMultiplier);
  } else {
    this->phaseDivider.setRatio(this->roundedMultiplier);
  }

  bool wasDrivenByMessage = false;

  if (leftExpander.module &&
      (leftExpander.module->model == modelClock ||
       leftExpander.module->model == modelDivider ||
       leftExpander.module->model == modelDiv ||
       leftExpander.module->model == modelDivExp)) {
    ZZC_TransportMessage *message = (ZZC_TransportMessage*) leftExpander.consumerMessage;
    if (params[DIR_PARAM].getValue() == 0.f) {
      if (processMessageData(message, args.sampleTime)) {
        wasDrivenByMessage = true;
      }
    }
    if (rightExpander.module &&
        (rightExpander.module->model == modelDivider ||
         rightExpander.module->model == modelDiv ||
         rightExpander.module->model == modelDivExp)) {
      message = (ZZC_TransportMessage*) rightExpander.module->leftExpander.producerMessage;
      std::memcpy(message, leftExpander.consumerMessage, sizeof(ZZC_TransportMessage));
      message->hasDivExp = true;
      message->divExpPhase = this->phaseDivider.phase;
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
      if (processMessageData(message, args.sampleTime)) {
        wasDrivenByMessage = true;
      }
    }
    if (leftExpander.module &&
        (leftExpander.module->model == modelDivider ||
         leftExpander.module->model == modelDiv ||
         leftExpander.module->model == modelDivExp)) {
      ZZC_TransportMessage *message = (ZZC_TransportMessage*) leftExpander.module->rightExpander.producerMessage;
      std::memcpy(message, rightExpander.consumerMessage, sizeof(ZZC_TransportMessage));
      message->hasDivExp = true;
      message->divExpPhase = this->phaseDivider.phase;
      message->divExpReset = resetWasHitForMessage;
      leftExpander.module->rightExpander.messageFlipRequested = true;
    }
  }

  if (wasDrivenByMessage) {
    this->lastTimeDrivenBy = this->transportSource;
  } else {
    this->lastTimeDrivenBy = TS_NONE;
  }

  this->syncing = this->phaseDivider.ratioIsRequested || this->phaseDivider.hardSyncIsRequested;

  outputs[PHASE_OUTPUT].setVoltage(this->phaseDivider.phase);
  outputs[CLOCK_OUTPUT].setVoltage(
    (this->gateMode ? this->phaseDivider.phase < 5.0 : this->pulseGenerator.process(args.sampleTime)) ? 10.f : 0.f
  );
  resetWasHitForMessage = false;
  this->firstCycle = false;
  this->lastSyncState = this->sync;
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
    display->value = &module->fractionDisplay;
    display->polarity = &module->fractionDisplayPolarity;
    display->blinking = &module->syncing;
  }
  addChild(display);

  addInput(createInput<ZZC_PJ_Port>(Vec(10.5, 124), module, DivExp::CV_INPUT));

  addParam(createParam<ZZC_LEDBezelDark>(Vec(11.3f, 172.0f), module, DivExp::SYNC_SWITCH_PARAM));
  addChild(createLight<LedLight<ZZC_YellowLight>>(Vec(13.1f, 173.7f), module, DivExp::SYNC_LED));

  addParam(createParam<ZZC_Switch2>(Vec(12.f, 204.f), module, DivExp::DIR_PARAM));

  addChild(createLight<SmallLight<ZZC_YellowLight>>(Vec(12.35f, 236.6f), module, DivExp::DIR_LEFT_LED));
  addChild(createLight<SmallLight<ZZC_YellowLight>>(Vec(26.6f, 236.6f), module, DivExp::DIR_RIGHT_LED));

  addOutput(createOutput<ZZC_PJ_Port>(Vec(10.5, 272), module, DivExp::CLOCK_OUTPUT));
  addOutput(createOutput<ZZC_PJ_Port>(Vec(10.5, 320), module, DivExp::PHASE_OUTPUT));

  addChild(createWidget<ZZC_Screw>(Vec(RACK_GRID_WIDTH, 0)));
  addChild(createWidget<ZZC_Screw>(Vec(RACK_GRID_WIDTH, RACK_GRID_HEIGHT - RACK_GRID_WIDTH)));
}

struct DivGateModeItem : MenuItem {
  DivBase *div;
  void onAction(const event::Action &e) override {
    div->gateMode ^= true;
  }
  void step() override {
    rightText = CHECKMARK(div->gateMode);
  }
};

void DivModuleBaseWidget::appendContextMenu(Menu *menu) {
  menu->addChild(new MenuSeparator());

  DivBase *div = dynamic_cast<DivBase*>(module);
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
  for (int i = 0; i < NUM_TRANSPORT_SOURCES - 1; i++) {
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
