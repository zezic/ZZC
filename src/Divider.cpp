#include "ZZC.hpp"
#include "Divider.hpp"
#include <cmath>

inline void Divider::processRatioInputs() {
  if (inputs[IN_RATIO_INPUT].isConnected()) {
    from = std::roundf(clamp(inputs[IN_RATIO_INPUT].getVoltage(), 0.0f, 10.0f) / 10.0f * (params[IN_RATIO_PARAM].getValue() - 1) + 1);
  } else {
    from = params[IN_RATIO_PARAM].getValue();
  }
  if (inputs[OUT_RATIO_INPUT].isConnected()) {
    to = std::roundf(clamp(inputs[OUT_RATIO_INPUT].getVoltage(), 0.0f, 10.0f) / 10.0f * (params[OUT_RATIO_PARAM].getValue() - 1) + 1);
  } else {
    to = params[OUT_RATIO_PARAM].getValue();
  }
  ratio = to / from;
}

inline void Divider::processSwingInput() {
  if (inputs[SWING_INPUT].isConnected()) {
    float swingParam = params[SWING_PARAM].getValue();
    float swingInput = clamp(inputs[SWING_INPUT].getVoltage() / 5.0f, -1.0f, 1.0f);
    if (swingInput < 0.0f) {
      swing = swingParam + (swingParam - 1.0f) * swingInput;
    } else if (swingInput >= 0.0f) {
      swing = swingParam + (99.0f - swingParam) * swingInput;
    }
  } else {
    swing = params[SWING_PARAM].getValue();
  }
}

Divider::Divider() {
  config(NUM_PARAMS, NUM_INPUTS, NUM_OUTPUTS, NUM_LIGHTS);
  configParam(IN_RATIO_PARAM, 1.0f, 99.0f, 1.0f, "Ratio Numerator");
  configParam(OUT_RATIO_PARAM, 1.0f, 99.0f, 1.0f, "Ratio Denominator");
  configParam(SWING_PARAM, 1.0f, 99.0f, 50.0f, "Swing");
  rightExpander.producerMessage = &rightMessages[0];
  rightExpander.consumerMessage = &rightMessages[1];
  leftExpander.producerMessage = &leftMessages[0];
  leftExpander.consumerMessage = &leftMessages[1];
  cleanMessage.hasDivider = true;
}

json_t *Divider::dataToJson() {
  json_t *rootJ = json_object();
  json_object_set_new(rootJ, "gateMode", json_boolean(gateMode));
  json_object_set_new(rootJ, "tickOnStart", json_boolean(tickOnStart));
  return rootJ;
}

void Divider::dataFromJson(json_t *rootJ) {
  json_t *gateModeJ = json_object_get(rootJ, "gateMode");
  json_t *tickOnStartJ = json_object_get(rootJ, "tickOnStart");
  if (gateModeJ) { gateMode = json_boolean_value(gateModeJ); }
  if (tickOnStartJ) { tickOnStart = json_boolean_value(tickOnStartJ); }
}


void Divider::process(const ProcessArgs &args) {
  processRatioInputs();
  processSwingInput();

  bool resetWasHitForMessage = false;

  if (resetTrigger.process(inputs[RESET_INPUT].getVoltage())) {
    resetWasHitForMessage = true;
    phaseOut = 0.0f;
    halfPhaseOut = 0.0;
    lastHalfPhaseOut = 0.0f;
    clockPulseGenerator.trigger(gateMode ? 1e-4f : 1e-3f);
  } else if (inputs[PHASE_INPUT].isConnected()) {
    if (lastPhaseInState) {
      phaseIn = std::fmod(inputs[PHASE_INPUT].getVoltage(), 10.0f);
      float phaseInDelta = phaseIn - lastPhaseIn;
      if (fabsf(phaseInDelta) > 0.1f && (sgn(phaseInDelta) != sgn(lastPhaseInDelta))) {
        phaseInDelta = lastPhaseInDelta;
      }
      lastPhaseInDelta = phaseInDelta;
      halfPhaseOut += phaseInDelta * ratio * 0.5f;
    }
  }

  // halfPhaseOut = eucmod(halfPhaseOut, 10.0f);
  while (halfPhaseOut >= 10.0) {
    halfPhaseOut = halfPhaseOut - 10.0;
  }
  while (halfPhaseOut < 0.0) {
    halfPhaseOut = halfPhaseOut + 10.0;
  }

  // Swing resulting phase
  float swingTresh = swing / 10.0f;
  if (halfPhaseOut < swingTresh) {
    phaseOut = halfPhaseOut / swingTresh * 10.0f;
  } else {
    float swingRem = 10.0f - swingTresh;
    float phaseGoes = halfPhaseOut - swingTresh;
    phaseOut = phaseGoes / swingRem * 10.0f;
  }

  // Trigger swinged beat
  if (!gateMode) {
    if ((lastHalfPhaseOut < swingTresh && swingTresh <= halfPhaseOut) ||
        (lastHalfPhaseOut > swingTresh && swingTresh >= halfPhaseOut) ||
        (tickOnStart && lastHalfPhaseOut == 0.0f && halfPhaseOut != 0.0f)) {
      clockPulseGenerator.trigger(gateMode ? 1e-4f : 1e-3f);
    }
  }

  lastHalfPhaseOut = halfPhaseOut;

  lastPhaseIn = std::fmod(inputs[PHASE_INPUT].getVoltage(), 10.0f);
  lastPhaseInState = inputs[PHASE_INPUT].isConnected();


  outputs[PHASE_OUTPUT].setVoltage(phaseOut);

  clockPulse = clockPulseGenerator.process(args.sampleTime);
  if (gateMode) {
    outputs[CLOCK_OUTPUT].setVoltage(phaseOut < 5.0f && !clockPulse ? 10.0f : 0.0f);
  } else {
    outputs[CLOCK_OUTPUT].setVoltage(clockPulse ? 10.0f : 0.0f);
  }

  lights[EXT_PHASE_MODE_LED].value = inputs[PHASE_INPUT].isConnected() ? 0.5f : 0.0f;

  if (rightExpander.module &&
      (rightExpander.module->model == modelDivider ||
       rightExpander.module->model == modelDiv ||
       rightExpander.module->model == modelDivExp)) {
    ZZC_TransportMessage *message = (ZZC_TransportMessage*) rightExpander.module->leftExpander.producerMessage;
    std::memcpy(message, leftExpander.consumerMessage, sizeof(ZZC_TransportMessage));
    message->hasDivider = true;
    message->dividerPhase = outputs[PHASE_OUTPUT].getVoltage();
    message->dividerReset = resetWasHitForMessage;
    rightExpander.module->leftExpander.messageFlipRequested = true;
  }

  if (leftExpander.module &&
      (leftExpander.module->model == modelDivider ||
       leftExpander.module->model == modelDiv ||
       leftExpander.module->model == modelDivExp)) {
    ZZC_TransportMessage *message = (ZZC_TransportMessage*) leftExpander.module->rightExpander.producerMessage;
    std::memcpy(message, rightExpander.consumerMessage, sizeof(ZZC_TransportMessage));
    message->hasDivider = true;
    message->dividerPhase = outputs[PHASE_OUTPUT].getVoltage();
    message->dividerReset = resetWasHitForMessage;
    leftExpander.module->rightExpander.messageFlipRequested = true;
  }

}


struct DividerWidget : ModuleWidget {
  DividerWidget(Divider *module);
  void appendContextMenu(Menu *menu) override;
};

DividerWidget::DividerWidget(Divider *module) {
  setModule(module);
  setPanel(APP->window->loadSvg(asset::plugin(pluginInstance, "res/panels/Divider.svg")));

  RatioDisplayWidget *ratioDisplay = new RatioDisplayWidget();
  ratioDisplay->box.pos = Vec(9.0f, 94.0f);
  ratioDisplay->box.size = Vec(57.0f, 21.0f);
  if (module) {
    ratioDisplay->from = &module->from;
    ratioDisplay->to = &module->to;
  }
  addChild(ratioDisplay);

  addParam(createParam<ZZC_CrossKnobSnappy>(Vec(12.5, 39.5), module, Divider::IN_RATIO_PARAM));
  addParam(createParam<ZZC_CrossKnobSnappy>(Vec(12.5, 123.5), module, Divider::OUT_RATIO_PARAM));

  addInput(createInput<ZZC_PJ_Port>(Vec(8, 191), module, Divider::SWING_INPUT));
  addParam(createParam<ZZC_Knob25>(Vec(42.5, 191.0), module, Divider::SWING_PARAM));

  addInput(createInput<ZZC_PJ_Port>(Vec(8, 233), module, Divider::IN_RATIO_INPUT));
  addInput(createInput<ZZC_PJ_Port>(Vec(42.5, 233), module, Divider::OUT_RATIO_INPUT));

  addInput(createInput<ZZC_PJ_Port>(Vec(8, 275), module, Divider::PHASE_INPUT));
  addChild(createLight<TinyLight<GreenLight>>(Vec(30, 275), module, Divider::EXT_PHASE_MODE_LED));
  addInput(createInput<ZZC_PJ_Port>(Vec(42.5, 275), module, Divider::RESET_INPUT));
  addOutput(createOutput<ZZC_PJ_Port>(Vec(8, 320), module, Divider::CLOCK_OUTPUT));
  addOutput(createOutput<ZZC_PJ_Port>(Vec(42.5, 320), module, Divider::PHASE_OUTPUT));

  addChild(createWidget<ZZC_Screw>(Vec(RACK_GRID_WIDTH, 0)));
  addChild(createWidget<ZZC_Screw>(Vec(box.size.x - 2 * RACK_GRID_WIDTH, 0)));
  addChild(createWidget<ZZC_Screw>(Vec(RACK_GRID_WIDTH, RACK_GRID_HEIGHT - RACK_GRID_WIDTH)));
  addChild(createWidget<ZZC_Screw>(Vec(box.size.x - 2 * RACK_GRID_WIDTH, RACK_GRID_HEIGHT - RACK_GRID_WIDTH)));
}


struct DividerGateModeItem : MenuItem {
  Divider *divider;
  void onAction(const event::Action &e) override {
    divider->gateMode ^= true;
  }
  void step() override {
    rightText = CHECKMARK(divider->gateMode);
  }
};

struct DividerTickOnStartItem : MenuItem {
  Divider *divider;
  void onAction(const event::Action &e) override {
    divider->tickOnStart ^= true;
  }
  void step() override {
    rightText = CHECKMARK(divider->tickOnStart);
  }
};

void DividerWidget::appendContextMenu(Menu *menu) {
  menu->addChild(new MenuSeparator());

  Divider *divider = dynamic_cast<Divider*>(module);
  assert(divider);

  DividerGateModeItem *gateModeItem = createMenuItem<DividerGateModeItem>("Gate Mode");
  gateModeItem->divider = divider;
  menu->addChild(gateModeItem);

  DividerTickOnStartItem *tickOnStartItem = createMenuItem<DividerTickOnStartItem>("Tick on Start");
  tickOnStartItem->divider = divider;
  menu->addChild(tickOnStartItem);
}


Model *modelDivider = createModel<Divider, DividerWidget>("Divider");
