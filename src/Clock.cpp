#include "ZZC.hpp"
#include "Clock.hpp"

void Clock::toggle() {
  running = !running;
  if (running) {
    if (resetOnStart) {
      resetWasHit = true;
      resetWasHitForMessage = true;
      resetPulseGenerator.trigger(1e-3f);
    }
  } else {
    if (resetOnStop) {
      resetWasHit = true;
      resetWasHitForMessage = true;
      resetPulseGenerator.trigger(1e-3f);
    }
  }
  runPulseGenerator.trigger(1e-3f);
}

inline void Clock::processButtons() {
  if (runInputIsGate && inputs[EXT_RUN_INPUT].isConnected()) {
    if (inputs[EXT_RUN_INPUT].getVoltage() > 1.0f) {
      if (!running) {
        toggle();
      }
    } else {
      if (running) {
        toggle();
      }
    }
  } else {
    if (runButtonTrigger.process(params[RUN_SWITCH_PARAM].getValue()) ||
        (inputs[EXT_RUN_INPUT].isConnected() && externalRunTrigger.process(inputs[EXT_RUN_INPUT].getVoltage()))) {
      toggle();
    }
  }

  if (resetButtonTrigger.process(params[RESET_SWITCH_PARAM].getValue()) ||
      (inputs[EXT_RESET_INPUT].isConnected() && externalResetTrigger.process(inputs[EXT_RESET_INPUT].getVoltage()))) {
    resetWasHit = true;
    resetWasHitForMessage = true;
    resetPulseGenerator.trigger(1e-3f);
  }

  if (reverseButtonTrigger.process(params[REVERSE_SWITCH_PARAM].getValue())) {
    reverse ^= true;
  }

  this->useCompatibleBPMCV = params[USE_COMPATIBLE_BPM_CV_PARAM].getValue() == 1.f;
}

inline void Clock::processSwingInputs() {
  swing8thsFinal = params[SWING_8THS_PARAM].getValue();
  if (inputs[SWING_8THS_INPUT].isConnected()) {
    float swing8thsInput = clamp(inputs[SWING_8THS_INPUT].getVoltage() / 5.0f, -1.0f, 1.0f);
    if (swing8thsInput < 0.0f) {
      swing8thsFinal += (swing8thsFinal - 1.0f) * swing8thsInput;
    } else if (swing8thsInput > 0.0f) {
      swing8thsFinal += (99.0f - swing8thsFinal) * swing8thsInput;
    }
  }
  swing16thsFinal = params[SWING_16THS_PARAM].getValue();
  if (inputs[SWING_16THS_INPUT].isConnected()) {
    float swing16thsInput = clamp(inputs[SWING_16THS_INPUT].getVoltage() / 5.0f, -1.0f, 1.0f);
    if (swing16thsInput < 0.0f) {
      swing16thsFinal += (swing16thsFinal - 1.0f) * swing16thsInput;
    } else if (swing16thsInput > 0.0f) {
      swing16thsFinal += (99.0f - swing16thsFinal) * swing16thsInput;
    }
  }
}

inline void Clock::triggerThsByPhase(float phase, float lastPhase) {
  float trigger8thsAtPhase = swing8thsFinal / 100.0f;
  float triggerSecond16thAtPhase = trigger8thsAtPhase * swing16thsFinal / 100.0f;
  float triggerFourth16thAtPhase = trigger8thsAtPhase + (1.0f - trigger8thsAtPhase) * swing16thsFinal / 100.0f;
  if (!x2ClockGateMode || !x4ClockGateMode) {
    if ((lastPhase < trigger8thsAtPhase && trigger8thsAtPhase <= phase) ||
        (lastPhase > trigger8thsAtPhase && trigger8thsAtPhase >= phase)) {
      clock8thsPulseGenerator.trigger(1e-3f);
      clock16thsPulseGenerator.trigger(1e-3f);
    }
  }
  if (x2ClockGateMode) {
    clock8thsPulse = (
      (phase < triggerSecond16thAtPhase) ||
      (phase >= trigger8thsAtPhase && phase < triggerFourth16thAtPhase)
    );
  }
  if (x4ClockGateMode) {
    clock16thsPulse = (
      (phase < triggerSecond16thAtPhase / 2.0f) ||
      (phase >= triggerSecond16thAtPhase && phase < (trigger8thsAtPhase - (trigger8thsAtPhase - triggerSecond16thAtPhase) / 2.0f)) ||
      (phase >= trigger8thsAtPhase && phase < (triggerFourth16thAtPhase - ((triggerFourth16thAtPhase - trigger8thsAtPhase) / 2.0f))) ||
      (phase >= triggerFourth16thAtPhase && phase < (1.0f - (1.0f - triggerFourth16thAtPhase) / 2.0f))
    );
  } else {
    if ((lastPhase < triggerSecond16thAtPhase && triggerSecond16thAtPhase <= phase) ||
        (lastPhase > triggerSecond16thAtPhase && triggerSecond16thAtPhase >= phase) ||
        (lastPhase < triggerFourth16thAtPhase && triggerFourth16thAtPhase <= phase) ||
        (lastPhase > triggerFourth16thAtPhase && triggerFourth16thAtPhase >= phase)) {
      clock16thsPulseGenerator.trigger(1e-3f);
    }
  }
}

inline enum Clock::Modes Clock::detectMode() {
  if (inputs[CLOCK_INPUT].isConnected() && inputs[PHASE_INPUT].isConnected()) {
    return EXT_CLOCK_AND_PHASE_MODE;
  } else if (inputs[CLOCK_INPUT].isConnected()) {
    return EXT_CLOCK_MODE;
  } else if (inputs[PHASE_INPUT].isConnected()) {
    return EXT_PHASE_MODE;
  } else if (inputs[VBPS_INPUT].isConnected()) {
    return EXT_VBPS_MODE;
  }
  return INTERNAL_MODE;
}

Clock::Clock() {
  config(NUM_PARAMS, NUM_INPUTS, NUM_OUTPUTS, NUM_LIGHTS);
  configParam(REVERSE_SWITCH_PARAM, 0.0f, 1.0f, 0.0f, "Reverse");
  configParam(BPM_PARAM, 0.0f, 240.0f, 120.0f, "BPM");
  configParam(SWING_8THS_PARAM, 1.0f, 99.0f, 50.0f, "x2 Swing");
  configParam(SWING_16THS_PARAM, 1.0f, 99.0f, 50.0f, "x4 Swing");
  configParam(RUN_SWITCH_PARAM, 0.0f, 1.0f, 0.0f, "Run");
  configParam(RESET_SWITCH_PARAM, 0.0f, 1.0f, 0.0f, "Reset");
  configParam(USE_COMPATIBLE_BPM_CV_PARAM, 0.0f, 1.0f, 1.0f, "External CV Mode");
  clockTracker.init();
  rightExpander.producerMessage = &rightMessages[0];
  rightExpander.consumerMessage = &rightMessages[1];
  leftExpander.producerMessage = &leftMessages[0];
  leftExpander.consumerMessage = &leftMessages[1];
  cleanMessage.hasClock = true;
}

json_t *Clock::dataToJson() {
  json_t *rootJ = json_object();
  json_object_set_new(rootJ, "running", json_integer((int) running));
  json_object_set_new(rootJ, "reverse", json_integer((int) reverse));
  json_object_set_new(rootJ, "baseClockGateMode", json_boolean(baseClockGateMode));
  json_object_set_new(rootJ, "x2ClockGateMode", json_boolean(x2ClockGateMode));
  json_object_set_new(rootJ, "x4ClockGateMode", json_boolean(x4ClockGateMode));
  json_object_set_new(rootJ, "resetOnStart", json_boolean(resetOnStart));
  json_object_set_new(rootJ, "resetOnStop", json_boolean(resetOnStop));
  json_object_set_new(rootJ, "runInputIsGate", json_boolean(runInputIsGate));
  json_object_set_new(rootJ, "runOutputIsGate", json_boolean(runOutputIsGate));
  json_object_set_new(rootJ, "phaseOutputOffset", json_real(phaseOutputOffset));
  json_object_set_new(rootJ, "useCompatibleBPMCV", json_boolean(useCompatibleBPMCV));
  json_object_set_new(rootJ, "snapCV", json_boolean(snapCV));
  json_object_set_new(rootJ, "externalClockPPQN", json_integer(externalClockPPQN));
  return rootJ;
}

void Clock::dataFromJson(json_t *rootJ) {
  json_t *runningJ = json_object_get(rootJ, "running");
  json_t *reverseJ = json_object_get(rootJ, "reverse");
  json_t *baseClockGateModeJ = json_object_get(rootJ, "baseClockGateMode");
  json_t *x2ClockGateModeJ = json_object_get(rootJ, "x2ClockGateMode");
  json_t *x4ClockGateModeJ = json_object_get(rootJ, "x4ClockGateMode");
  json_t *resetOnStartJ = json_object_get(rootJ, "resetOnStart");
  json_t *resetOnStopJ = json_object_get(rootJ, "resetOnStop");
  json_t *runInputIsGateJ = json_object_get(rootJ, "runInputIsGate");
  json_t *runOutputIsGateJ = json_object_get(rootJ, "runOutputIsGate");
  json_t *phaseOutputOffsetJ = json_object_get(rootJ, "phaseOutputOffset");
  json_t *useCompatibleBPMCVJ = json_object_get(rootJ, "useCompatibleBPMCV");
  json_t *snapCVJ = json_object_get(rootJ, "snapCV");
  json_t *externalClockPPQNJ = json_object_get(rootJ, "externalClockPPQN");
  if (runningJ) { running = json_integer_value(runningJ); }
  if (reverseJ) { reverse = json_integer_value(reverseJ); }
  if (baseClockGateModeJ) { baseClockGateMode = json_boolean_value(baseClockGateModeJ); }
  if (x2ClockGateModeJ) { x2ClockGateMode = json_boolean_value(x2ClockGateModeJ); }
  if (x4ClockGateModeJ) { x4ClockGateMode = json_boolean_value(x4ClockGateModeJ); }
  if (resetOnStartJ) { resetOnStart = json_boolean_value(resetOnStartJ); }
  if (resetOnStopJ) { resetOnStop = json_boolean_value(resetOnStopJ); }
  if (runInputIsGateJ) { runInputIsGate = json_boolean_value(runInputIsGateJ); }
  if (runOutputIsGateJ) { runOutputIsGate = json_boolean_value(runOutputIsGateJ); }
  if (phaseOutputOffsetJ) { phaseOutputOffset = json_real_value(phaseOutputOffsetJ); }
  if (useCompatibleBPMCVJ) {
    useCompatibleBPMCV = json_boolean_value(useCompatibleBPMCVJ);
  } else {
    useCompatibleBPMCV = false; // Fallback to pre v1.1.3 default behavior
  }
  if (snapCVJ) { snapCV = json_boolean_value(snapCVJ); }
  if (externalClockPPQNJ) { externalClockPPQN = json_integer_value(externalClockPPQNJ); }
}

void Clock::process(const ProcessArgs &args) {
  lastMode = mode;
  mode = detectMode();

  processButtons();
  processSwingInputs();

  oscillator.PPQN = externalClockPPQN;

  if (mode == INTERNAL_MODE || mode == EXT_VBPS_MODE || mode == EXT_CLOCK_MODE) {

    if (mode == EXT_CLOCK_MODE) {
      if (lastMode != EXT_CLOCK_MODE || resetWasHit) {
        clockTracker.init();
        clockTracker.freq = fabsf(bpm / 60.0f);
      }
      clockTracker.process(args.sampleTime, inputs[CLOCK_INPUT].getVoltage());
      if (clockTracker.freqDetected) {
        bpm = clockTracker.freq * 60.0f / externalClockPPQN;
      }
    } else if (mode == EXT_VBPS_MODE) {
      if (this->useCompatibleBPMCV) {
        bpm = params[BPM_PARAM].getValue() * dsp::approxExp2_taylor5(inputs[VBPS_INPUT].getVoltage() + 10.f) / 1024.f;
      } else {
        bpm = params[BPM_PARAM].getValue() + inputs[VBPS_INPUT].getVoltage() * 60.0f;
      }
      if (this->snapCV) {
        bpm = std::round(bpm);
      }
    } else {
      bpm = params[BPM_PARAM].getValue();
    }

    if (reverse) {
      bpm = bpm * -1.0f;
    }

    oscillator.setPitch(bpm / 60.0f);

    if (running) {

      if (resetWasHit) {
        oscillator.reset((reverse ? 1.0f : 0.0f));
      }

      if (mode == EXT_CLOCK_MODE) {
        oscillator.adjustPhase(inputs[CLOCK_INPUT].getVoltage());
      }

      bool phaseFlipped = oscillator.step(args.sampleTime);

      if (phaseFlipped || resetWasHit) {
        if (!baseClockGateMode) { clockPulseGenerator.trigger(1e-3f); }
        if (!x2ClockGateMode) { clock8thsPulseGenerator.trigger(1e-3f); }
        if (!x4ClockGateMode) { clock16thsPulseGenerator.trigger(1e-3f); }
      } else {
        triggerThsByPhase(oscillator.phase, oscillator.lastPhase);
      }

      if (resetWasHit) {
        resetWasHit = false;
      }

    }
    if (resetWasHit) {
      oscillator.reset((reverse ? 1.0f : 0.0f));
    }
  }
  if (mode == EXT_PHASE_MODE || mode == EXT_CLOCK_AND_PHASE_MODE) {
    bool triggered = false;

    // Trust lastExtPhase if previous step was done with PHASE_INPUT plugged in
    if (lastMode == EXT_PHASE_MODE || lastMode == EXT_CLOCK_AND_PHASE_MODE) {
      float delta = inputs[PHASE_INPUT].getVoltage() - lastExtPhase;
      float absDelta = fabsf(delta);
      if (absDelta > 9.5f && absDelta <= 10.0f) {

        // Probably, it was a phase flip, don't twitch our BPM and take it as a beat trigger
        if (mode == EXT_PHASE_MODE) {
          triggered = true;
        }

        // Compensate phase flip
        if (delta < 0.0f) {
          delta = 10.0f + delta;
        } else {
          delta = delta - 10.0f;
        }
      }
      bpm = delta / args.sampleTime * 6.0f;
    }

    if (mode == EXT_CLOCK_AND_PHASE_MODE) {
      triggered = externalClockTrigger.process(inputs[CLOCK_INPUT].getVoltage()) && (externalClockPPQN == 1 || externalClockPPQN == 2);
    }

    if (running) {
      if (triggered) {
        if (!baseClockGateMode) { clockPulseGenerator.trigger(1e-3f); }
        if (!x2ClockGateMode) { clock8thsPulseGenerator.trigger(1e-3f); }
        if (!x4ClockGateMode) { clock16thsPulseGenerator.trigger(1e-3f); }
      } else {
        if (lastMode == EXT_PHASE_MODE || lastMode == EXT_CLOCK_AND_PHASE_MODE) {
          triggerThsByPhase(inputs[PHASE_INPUT].getVoltage() / 10.0f, lastExtPhase  / 10.0f);
        }
      }
    }
    lastExtPhase = inputs[PHASE_INPUT].getVoltage();
  }

  // Generate Pulse
  if (!baseClockGateMode) { clockPulse = clockPulseGenerator.process(args.sampleTime); }
  if (!x2ClockGateMode) { clock8thsPulse = clock8thsPulseGenerator.process(args.sampleTime); }
  if (!x4ClockGateMode) { clock16thsPulse = clock16thsPulseGenerator.process(args.sampleTime); }
  runPulse = runPulseGenerator.process(args.sampleTime);
  resetPulse = resetPulseGenerator.process(args.sampleTime);

  if (running) {
    if (baseClockGateMode) {
      bool pulse = (mode == EXT_PHASE_MODE || mode == EXT_CLOCK_AND_PHASE_MODE) ? inputs[PHASE_INPUT].getVoltage() < 5.f : oscillator.phase < 0.5f;
      outputs[CLOCK_OUTPUT].setVoltage(pulse ? 10.f : 0.f);
    } else {
      outputs[CLOCK_OUTPUT].setVoltage(clockPulse ? 10.f : 0.f);
    }
  } else {
    outputs[CLOCK_OUTPUT].setVoltage(0.f);
  }
  outputs[CLOCK_8THS_OUTPUT].setVoltage(running && clock8thsPulse ? 10.0f : 0.0f);
  outputs[CLOCK_16THS_OUTPUT].setVoltage(running && clock16thsPulse ? 10.0f : 0.0f);
  if (runOutputIsGate) {
    outputs[RUN_OUTPUT].setVoltage(running ? 10.0f : 0.0f);
  } else {
    outputs[RUN_OUTPUT].setVoltage(runPulse ? 10.0f : 0.0f);
  }
  outputs[RESET_OUTPUT].setVoltage(resetPulse ? 10.0f : 0.0f);

  // Output Voltages
  if (mode == EXT_PHASE_MODE || mode == EXT_CLOCK_AND_PHASE_MODE) {
    outputs[PHASE_OUTPUT].setVoltage(inputs[PHASE_INPUT].getVoltage() + this->phaseOutputOffset);
  } else {
    outputs[PHASE_OUTPUT].setVoltage(oscillator.phase * 10.0f + this->phaseOutputOffset);
  }
  outputs[VBPS_OUTPUT].setVoltage(bpm / 60.0f);
  outputs[VSPB_OUTPUT].setVoltage(bpm == 0.0f ? 10.0f : fminf(60.0f / fabsf(bpm), 10.0f));

  // Status Lights
  if (running) {
    lights[RUN_LED].value = 1.1f;
  }
  lights[INTERNAL_MODE_LED].value = (mode == INTERNAL_MODE || mode == EXT_VBPS_MODE) ? 1.0f : 0.0f;
  lights[EXT_VBPS_MODE_LED].value = mode == EXT_VBPS_MODE ? 1.0f : 0.0f;
  lights[EXT_CLOCK_MODE_LED].value = (mode == EXT_CLOCK_MODE || mode == EXT_CLOCK_AND_PHASE_MODE) ? 1.0f : 0.0f;
  lights[EXT_PHASE_MODE_LED].value = (mode == EXT_PHASE_MODE || mode == EXT_CLOCK_AND_PHASE_MODE) ? 1.0f : 0.0f;

  lights[CLOCK_LED].value = (1.0f - oscillator.phase) * (1.0f - oscillator.phase);
  if (resetPulse) {
    lights[RESET_LED].value = 1.1f;
  }
  if (bpm < 0.0f) {
    lights[REVERSE_LED].value = 1.1f;
  }

  if (rightExpander.module &&
      (rightExpander.module->model == modelDivider ||
       rightExpander.module->model == modelDiv ||
       rightExpander.module->model == modelDivExp)) {
    ZZC_TransportMessage *message = (ZZC_TransportMessage*) rightExpander.module->leftExpander.producerMessage;
    std::memcpy(message, &cleanMessage, sizeof(ZZC_TransportMessage));
    message->clockPhase = outputs[PHASE_OUTPUT].getVoltage();
    message->clockReset = resetWasHitForMessage;
    rightExpander.module->leftExpander.messageFlipRequested = true;
  }

  if (leftExpander.module &&
      (leftExpander.module->model == modelDivider ||
       leftExpander.module->model == modelDiv ||
       leftExpander.module->model == modelDivExp)) {
    ZZC_TransportMessage *message = (ZZC_TransportMessage*) leftExpander.module->rightExpander.producerMessage;
    std::memcpy(message, &cleanMessage, sizeof(ZZC_TransportMessage));
    message->clockPhase = outputs[PHASE_OUTPUT].getVoltage();
    message->clockReset = resetWasHitForMessage;
    leftExpander.module->rightExpander.messageFlipRequested = true;
  }

  resetWasHitForMessage = false;
}


struct ClockWidget : ModuleWidget {
  ClockWidget(Clock *module);
  void appendContextMenu(Menu *menu) override;
  void onHoverKey(const event::HoverKey& e) override;
};

ClockWidget::ClockWidget(Clock *module) {
  setModule(module);
  setPanel(APP->window->loadSvg(asset::plugin(pluginInstance, "res/panels/Clock.svg")));

  addParam(createParam<ZZC_VBPSVOCTSwitch>(Vec(10.8f, 41.f), module, Clock::USE_COMPATIBLE_BPM_CV_PARAM));

  addInput(createInput<ZZC_PJ_Port>(Vec(10.8f, 52), module, Clock::VBPS_INPUT));
  addChild(createLight<TinyLight<GreenLight>>(Vec(33, 52), module, Clock::EXT_VBPS_MODE_LED));

  addParam(createParam<ZZC_LEDBezelDark>(Vec(116.3f, 53.0f), module, Clock::REVERSE_SWITCH_PARAM));
  addChild(createLight<LedLight<ZZC_YellowLight>>(Vec(118.1f, 54.7f), module, Clock::REVERSE_LED));

  Display32Widget *bpmDisplay = new Display32Widget();
  bpmDisplay->box.pos = Vec(46.0f, 40.0f);
  bpmDisplay->box.size = Vec(58.0f, 21.0f);
  if (module) {
    bpmDisplay->value = &module->bpm;
  }
  addChild(bpmDisplay);

  addChild(createLight<SmallLight<ZZC_YellowLight>>(Vec(72.0, 66.75), module, Clock::CLOCK_LED));

  addParam(createParam<ZZC_BigKnobSnappy>(Vec(41.5, 82.5), module, Clock::BPM_PARAM));
  addChild(createLight<TinyLight<GreenLight>>(Vec(111.5f, 83), module, Clock::INTERNAL_MODE_LED));
  addParam(createParam<ZZC_Knob27Snappy>(Vec(13.5, 186), module, Clock::SWING_8THS_PARAM));
  addParam(createParam<ZZC_Knob27Snappy>(Vec(109.5, 186), module, Clock::SWING_16THS_PARAM));

  addInput(createInput<ZZC_PJ_Port>(Vec(45.5, 224), module, Clock::CLOCK_INPUT));
  addChild(createLight<TinyLight<GreenLight>>(Vec(67.5, 224), module, Clock::EXT_CLOCK_MODE_LED));
  addInput(createInput<ZZC_PJ_Port>(Vec(80, 224), module, Clock::PHASE_INPUT));
  addChild(createLight<TinyLight<GreenLight>>(Vec(102, 224), module, Clock::EXT_PHASE_MODE_LED));

  addOutput(createOutput<ZZC_PJ_Port>(Vec(45.5, 272), module, Clock::CLOCK_OUTPUT));
  addOutput(createOutput<ZZC_PJ_Port>(Vec(80, 272), module, Clock::PHASE_OUTPUT));

  addOutput(createOutput<ZZC_PJ_Port>(Vec(10.8f, 320), module, Clock::VBPS_OUTPUT));
  addOutput(createOutput<ZZC_PJ_Port>(Vec(114.8f, 320), module, Clock::VSPB_OUTPUT));

  addInput(createInput<ZZC_PJ_Port>(Vec(10.8f, 145), module, Clock::EXT_RUN_INPUT));
  addParam(createParam<ZZC_LEDBezelDark>(Vec(47.3f, 168.0f), module, Clock::RUN_SWITCH_PARAM));
  addChild(createLight<LedLight<ZZC_YellowLight>>(Vec(49.1f, 169.7f), module, Clock::RUN_LED));
  addOutput(createOutput<ZZC_PJ_Port>(Vec(45.5, 320), module, Clock::RUN_OUTPUT));

  addInput(createInput<ZZC_PJ_Port>(Vec(114.8f, 145), module, Clock::EXT_RESET_INPUT));
  addParam(createParam<ZZC_LEDBezelDark>(Vec(81.3f, 168.0f), module, Clock::RESET_SWITCH_PARAM));
  addChild(createLight<LedLight<ZZC_YellowLight>>(Vec(83.1f, 169.7f), module, Clock::RESET_LED));
  addOutput(createOutput<ZZC_PJ_Port>(Vec(80, 320), module, Clock::RESET_OUTPUT));

  addInput(createInput<ZZC_PJ_Port>(Vec(10.8f, 224), module, Clock::SWING_8THS_INPUT));
  addInput(createInput<ZZC_PJ_Port>(Vec(114.8f, 224), module, Clock::SWING_16THS_INPUT));
  addOutput(createOutput<ZZC_PJ_Port>(Vec(10.8f, 272), module, Clock::CLOCK_8THS_OUTPUT));
  addOutput(createOutput<ZZC_PJ_Port>(Vec(114.8f, 272), module, Clock::CLOCK_16THS_OUTPUT));

  addChild(createWidget<ZZC_Screw>(Vec(RACK_GRID_WIDTH, 0)));
  addChild(createWidget<ZZC_Screw>(Vec(box.size.x - 2 * RACK_GRID_WIDTH, 0)));
  addChild(createWidget<ZZC_Screw>(Vec(RACK_GRID_WIDTH, RACK_GRID_HEIGHT - RACK_GRID_WIDTH)));
  addChild(createWidget<ZZC_Screw>(Vec(box.size.x - 2 * RACK_GRID_WIDTH, RACK_GRID_HEIGHT - RACK_GRID_WIDTH)));
}

struct BaseClockGateModeItem : MenuItem {
  Clock *clock;
  void onAction(const event::Action &e) override {
    clock->baseClockGateMode ^= true;
  }
};

struct X2ClockGateModeItem : MenuItem {
  Clock *clock;
  void onAction(const event::Action &e) override {
    clock->x2ClockGateMode ^= true;
  }
};

struct X4ClockGateModeItem : MenuItem {
  Clock *clock;
  void onAction(const event::Action &e) override {
    clock->x4ClockGateMode ^= true;
  }
};

struct UseGatesForItem : MenuItem {
  Clock *module;
  Menu *createChildMenu() override {
    Menu *menu = new Menu;

    BaseClockGateModeItem *baseClockGateModeItem = new BaseClockGateModeItem;
    baseClockGateModeItem->text = "Main Clock Output";
    baseClockGateModeItem->rightText = CHECKMARK(module->baseClockGateMode);
    baseClockGateModeItem->clock = module;
    menu->addChild(baseClockGateModeItem);

    X2ClockGateModeItem *x2ClockGateModeItem = new X2ClockGateModeItem;
    x2ClockGateModeItem->text = "X2 Clock Output";
    x2ClockGateModeItem->rightText = CHECKMARK(module->x2ClockGateMode);
    x2ClockGateModeItem->clock = module;
    menu->addChild(x2ClockGateModeItem);

    X4ClockGateModeItem *x4ClockGateModeItem = new X4ClockGateModeItem;
    x4ClockGateModeItem->text = "X4 Clock Output";
    x4ClockGateModeItem->rightText = CHECKMARK(module->x4ClockGateMode);
    x4ClockGateModeItem->clock = module;
    menu->addChild(x4ClockGateModeItem);

    return menu;
  }
};

struct ClockResetOnStartItem : MenuItem {
  Clock *clock;
  void onAction(const event::Action &e) override {
    clock->resetOnStart ^= true;
  }
  void step() override {
    rightText = CHECKMARK(clock->resetOnStart);
  }
};

struct ClockResetOnStopItem : MenuItem {
  Clock *clock;
  void onAction(const event::Action &e) override {
    clock->resetOnStop ^= true;
  }
  void step() override {
    rightText = CHECKMARK(clock->resetOnStop);
  }
};

struct RunInputTriggerItem : MenuItem {
  Clock *module;
  void onAction(const event::Action &e) override {
    module->runInputIsGate = false;
  }
};

struct RunInputGateItem : MenuItem {
  Clock *module;
  void onAction(const event::Action &e) override {
    module->runInputIsGate = true;
  }
};

struct RunOutputTriggerItem : MenuItem {
  Clock *module;
  void onAction(const event::Action &e) override {
    module->runOutputIsGate = false;
  }
};

struct RunOutputGateItem : MenuItem {
  Clock *module;
  void onAction(const event::Action &e) override {
    module->runOutputIsGate = true;
  }
};

struct PhaseOutputRange010Item : MenuItem {
  Clock *module;
  void onAction(const event::Action &e) override {
    module->phaseOutputOffset = 0.0f;
  }
};

struct PhaseOutputRange55Item : MenuItem {
  Clock *module;
  void onAction(const event::Action &e) override {
    module->phaseOutputOffset = -5.0f;
  }
};

struct RunInputModeItem : MenuItem {
  Clock *module;
  Menu *createChildMenu() override {
    Menu *menu = new Menu;

    RunInputTriggerItem *runInputTriggerItem = new RunInputTriggerItem;
    runInputTriggerItem->text = "Toggle";
    runInputTriggerItem->rightText = CHECKMARK(!module->runInputIsGate);
    runInputTriggerItem->module = module;
    menu->addChild(runInputTriggerItem);

    RunInputGateItem *runInputGateItem = new RunInputGateItem;
    runInputGateItem->text = "Hold";
    runInputGateItem->rightText = CHECKMARK(module->runInputIsGate);
    runInputGateItem->module = module;
    menu->addChild(runInputGateItem);

    return menu;
  }
};

struct RunOutputModeItem : MenuItem {
  Clock *module;
  Menu *createChildMenu() override {
    Menu *menu = new Menu;

    RunOutputTriggerItem *runOutputTriggerItem = new RunOutputTriggerItem;
    runOutputTriggerItem->text = "Trigger";
    runOutputTriggerItem->rightText = CHECKMARK(!module->runOutputIsGate);
    runOutputTriggerItem->module = module;
    menu->addChild(runOutputTriggerItem);

    RunOutputGateItem *runOutputGateItem = new RunOutputGateItem;
    runOutputGateItem->text = "Gate";
    runOutputGateItem->rightText = CHECKMARK(module->runOutputIsGate);
    runOutputGateItem->module = module;
    menu->addChild(runOutputGateItem);

    return menu;
  }
};

struct PhaseOutputRangeItem : MenuItem {
  Clock *module;
  Menu *createChildMenu() override {
    Menu *menu = new Menu;

    PhaseOutputRange55Item *phaseOutputRange55Item = new PhaseOutputRange55Item;
    phaseOutputRange55Item->text = "-5V to +5V";
    phaseOutputRange55Item->rightText = CHECKMARK(module->phaseOutputOffset != 0.0f);
    phaseOutputRange55Item->module = module;
    menu->addChild(phaseOutputRange55Item);

    PhaseOutputRange010Item *phaseOutputRange010Item = new PhaseOutputRange010Item;
    phaseOutputRange010Item->text = "0V to +10V";
    phaseOutputRange010Item->rightText = CHECKMARK(module->phaseOutputOffset == 0.0f);
    phaseOutputRange010Item->module = module;
    menu->addChild(phaseOutputRange010Item);

    return menu;
  }
};

struct ExternalClockPPQNOptionItem : MenuItem {
  Clock *module;
  int targetPPQN;
  void onAction(const event::Action &e) override {
    module->externalClockPPQN = this->targetPPQN;
  }
};

struct ExternalClockPPQNItem : MenuItem {
  Clock *module;
  Menu *createChildMenu() override {
    Menu *menu = new Menu;
    std::vector<int> PPQNModes = {
      1, 2, 4, 8, 12, 16, 24, 32, 48, 72, 96, 120, 144, 168, 192, 384, 768, 960
    };
    for (int PPQN : PPQNModes) {
      ExternalClockPPQNOptionItem *item = new ExternalClockPPQNOptionItem;
      item->text = std::to_string(PPQN);
      item->rightText = CHECKMARK(module->externalClockPPQN == PPQN);
      item->module = module;
      item->targetPPQN = PPQN;
      menu->addChild(item);
    }
    return menu;
  }
};

struct ExternalCVModeCompatibleOptionItem : MenuItem {
  Clock *module;
  void onAction(const event::Action &e) override {
    module->useCompatibleBPMCV = true;
    module->params[Clock::USE_COMPATIBLE_BPM_CV_PARAM].setValue(1.f);
  }
};

struct ExternalCVModeVBPSOptionItem : MenuItem {
  Clock *module;
  void onAction(const event::Action &e) override {
    module->useCompatibleBPMCV = false;
    module->params[Clock::USE_COMPATIBLE_BPM_CV_PARAM].setValue(0.f);
  }
};

struct SnapCVOptionItem : MenuItem {
  Clock *module;
  void onAction(const event::Action &e) override {
    module->snapCV ^= true;
  }
};

struct ExternalCVModeItem : MenuItem {
  Clock *module;
  Menu *createChildMenu() override {
    Menu *menu = new Menu;

    ExternalCVModeCompatibleOptionItem *item1 = new ExternalCVModeCompatibleOptionItem;
    item1->text = "V/OCT";
    item1->rightText = CHECKMARK(module->useCompatibleBPMCV);
    item1->module = module;
    menu->addChild(item1);

    ExternalCVModeVBPSOptionItem *item2 = new ExternalCVModeVBPSOptionItem;
    item2->text = "V/BPS";
    item2->rightText = CHECKMARK(!module->useCompatibleBPMCV);
    item2->module = module;
    menu->addChild(item2);

    menu->addChild(new MenuSeparator());

    SnapCVOptionItem *item3 = new SnapCVOptionItem;
    item3->text = "Snap CV";
    item3->rightText = CHECKMARK(module->snapCV);
    item3->module = module;
    menu->addChild(item3);

    return menu;
  }
};

void ClockWidget::appendContextMenu(Menu *menu) {

  Clock *clock = dynamic_cast<Clock*>(module);
  assert(clock);

  menu->addChild(new MenuSeparator());

  UseGatesForItem *useGatesForItem = new UseGatesForItem;
  useGatesForItem->text = "Use Gates For";
  useGatesForItem->rightText = RIGHT_ARROW;
  useGatesForItem->module = clock;
  menu->addChild(useGatesForItem);

  menu->addChild(new MenuSeparator());

  ClockResetOnStartItem *resetOnStartItem = createMenuItem<ClockResetOnStartItem>("Reset on Start");
  resetOnStartItem->clock = clock;
  menu->addChild(resetOnStartItem);

  ClockResetOnStopItem *resetOnStopItem = createMenuItem<ClockResetOnStopItem>("Reset on Stop");
  resetOnStopItem->clock = clock;
  menu->addChild(resetOnStopItem);

  menu->addChild(new MenuSeparator());

  RunInputModeItem *runInputModeItem = new RunInputModeItem;
  runInputModeItem->text = "Run Input Mode";
  runInputModeItem->rightText = RIGHT_ARROW;
  runInputModeItem->module = clock;
  menu->addChild(runInputModeItem);

  RunOutputModeItem *runOutputModeItem = new RunOutputModeItem;
  runOutputModeItem->text = "Run Output Mode";
  runOutputModeItem->rightText = RIGHT_ARROW;
  runOutputModeItem->module = clock;
  menu->addChild(runOutputModeItem);

  menu->addChild(new MenuSeparator());

  PhaseOutputRangeItem *phaseOutputRangeItem = new PhaseOutputRangeItem;
  phaseOutputRangeItem->text = "Phase Output Range";
  phaseOutputRangeItem->rightText = RIGHT_ARROW;
  phaseOutputRangeItem->module = clock;
  menu->addChild(phaseOutputRangeItem);

  menu->addChild(new MenuSeparator());

  ExternalClockPPQNItem *externalClockPPQNItem = new ExternalClockPPQNItem;
  externalClockPPQNItem->text = "External Clock PPQN";
  externalClockPPQNItem->rightText = RIGHT_ARROW;
  externalClockPPQNItem->module = clock;
  menu->addChild(externalClockPPQNItem);

  ExternalCVModeItem *externalCVModeItem = new ExternalCVModeItem;
  externalCVModeItem->text = "External CV Mode";
  externalCVModeItem->rightText = RIGHT_ARROW;
  externalCVModeItem->module = clock;
  menu->addChild(externalCVModeItem);
}

void ClockWidget::onHoverKey(const event::HoverKey &e) {
  if (e.action == GLFW_PRESS) {
    if ((e.mods & RACK_MOD_MASK) == 0) {
      if (e.key == GLFW_KEY_SPACE) {
        Clock *clock = dynamic_cast<Clock*>(module);
        clock->toggle();
        e.consume(this);
        return;
      }
    }
  }
  ModuleWidget::onHoverKey(e);
}

Model *modelClock = createModel<Clock, ClockWidget>("Clock");
