#include "ZZC.hpp"

struct Clock : Module {
  enum ParamIds {
    BPM_PARAM,
    SWING_8THS_PARAM,
    SWING_16THS_PARAM,
    RUN_SWITCH_PARAM,
    RESET_SWITCH_PARAM,
    REVERSE_SWITCH_PARAM,
    NUM_PARAMS
  };
  enum InputIds {
    VBPS_INPUT,
    EXT_RUN_INPUT,
    EXT_RESET_INPUT,
    CLOCK_INPUT,
    PHASE_INPUT,
    SWING_8THS_INPUT,
    SWING_16THS_INPUT,
    NUM_INPUTS
  };
  enum OutputIds {
    CLOCK_OUTPUT,
    PHASE_OUTPUT,
    CLOCK_8THS_OUTPUT,
    CLOCK_16THS_OUTPUT,
    VBPS_OUTPUT,
    VSPB_OUTPUT,
    RUN_OUTPUT,
    RESET_OUTPUT,
    NUM_OUTPUTS
  };
  enum LightIds {
    CLOCK_LED,
    RUN_LED,
    RESET_LED,
    REVERSE_LED,
    INTERNAL_MODE_LED,
    EXT_VBPS_MODE_LED,
    EXT_CLOCK_MODE_LED,
    EXT_PHASE_MODE_LED,
    NUM_LIGHTS
  };
  enum Modes {
    INTERNAL_MODE,
    EXT_VBPS_MODE,
    EXT_CLOCK_MODE,
    EXT_PHASE_MODE,
    EXT_CLOCK_AND_PHASE_MODE,
    NUM_MODES
  };

  ClockTracker clockTracker;
  LowFrequencyOscillator oscillator;

  enum Modes mode;
  enum Modes lastMode;

  float lastExtPhase = 0.0f;

  bool running = true;
  bool reverse = false;
  float bpm = 120.0f;

  float swing8thsFinal = 50.0f;
  float swing16thsFinal = 50.0f;

  float swinged8thsPhase = 0.5f;
  float swinged16thsFirstPhase = 0.25f;
  float swinged16thsSecondPhase = 0.75f;

  dsp::PulseGenerator clockPulseGenerator;
  dsp::PulseGenerator clock8thsPulseGenerator;
  dsp::PulseGenerator clock16thsPulseGenerator;
  dsp::PulseGenerator runPulseGenerator;
  dsp::PulseGenerator resetPulseGenerator;
  bool clockPulse = false;
  bool clock8thsPulse = false;
  bool clock16thsPulse = false;
  bool runPulse = false;
  bool resetPulse = false;
  bool resetWasHit = false;

  float clockLight = 0.0f;
  float resetLight = 0.0f;
  float reverseLight = 0.0f;

  dsp::SchmittTrigger runButtonTrigger;
  dsp::SchmittTrigger externalRunTrigger;
  dsp::SchmittTrigger resetButtonTrigger;
  dsp::SchmittTrigger externalResetTrigger;
  dsp::SchmittTrigger reverseButtonTrigger;
  dsp::SchmittTrigger externalClockTrigger;

  /* Settings */
  bool resetOnStart = false;
  bool resetOnStop = false;

  void toggle() {
    running = !running;
    if (running) {
      if (resetOnStart) {
        resetWasHit = true;
      }
    } else {
      if (resetOnStop) {
        resetWasHit = true;
      }
    }
    runPulseGenerator.trigger(1e-3f);
  }

  inline void processButtons() {
    if (runButtonTrigger.process(params[RUN_SWITCH_PARAM].getValue()) || (inputs[EXT_RUN_INPUT].isConnected() && externalRunTrigger.process(inputs[EXT_RUN_INPUT].getVoltage()))) {
      toggle();
    }

    if (resetButtonTrigger.process(params[RESET_SWITCH_PARAM].getValue()) || (inputs[EXT_RESET_INPUT].isConnected() && externalResetTrigger.process(inputs[EXT_RESET_INPUT].getVoltage()))) {
      resetWasHit = true;
      resetPulseGenerator.trigger(1e-3f);
    }

    if (reverseButtonTrigger.process(params[REVERSE_SWITCH_PARAM].getValue())) {
      reverse ^= true;
    }
  }

  inline void processSwingInputs() {
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

  inline void triggerThsByPhase(float phase, float lastPhase) {
    float trigger8thsAtPhase = swing8thsFinal / 100.0f;
    if ((lastPhase < trigger8thsAtPhase && trigger8thsAtPhase <= phase) ||
        (lastPhase > trigger8thsAtPhase && trigger8thsAtPhase >= phase)) {
      clock8thsPulseGenerator.trigger(1e-3f);
      clock16thsPulseGenerator.trigger(1e-3f);
    }
    float triggerSecond16thAtPhase = trigger8thsAtPhase * swing16thsFinal / 100.0f;
    float triggerThird16thAtPhase = trigger8thsAtPhase + (1.0f - trigger8thsAtPhase) * swing16thsFinal / 100.0f;
    if ((lastPhase < triggerSecond16thAtPhase && triggerSecond16thAtPhase <= phase) ||
        (lastPhase > triggerSecond16thAtPhase && triggerSecond16thAtPhase >= phase) ||
        (lastPhase < triggerThird16thAtPhase && triggerThird16thAtPhase <= phase) ||
        (lastPhase > triggerThird16thAtPhase && triggerThird16thAtPhase >= phase)) {
      clock16thsPulseGenerator.trigger(1e-3f);
    }
  }

  inline enum Modes detectMode() {
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

  Clock() {
    config(NUM_PARAMS, NUM_INPUTS, NUM_OUTPUTS, NUM_LIGHTS);
    configParam(REVERSE_SWITCH_PARAM, 0.0f, 1.0f, 0.0f, "Reverse");
    configParam(BPM_PARAM, 0.0f, 240.0f, 120.0f, "BPM");
    configParam(SWING_8THS_PARAM, 1.0f, 99.0f, 50.0f, "x2 Swing");
    configParam(SWING_16THS_PARAM, 1.0f, 99.0f, 50.0f, "x4 Swing");
    configParam(RUN_SWITCH_PARAM, 0.0f, 1.0f, 0.0f, "Run");
    configParam(RESET_SWITCH_PARAM, 0.0f, 1.0f, 0.0f, "Reset");
    clockTracker.init();
  }
  void process(const ProcessArgs &args) override;

  json_t *dataToJson() override {
    json_t *rootJ = json_object();
    json_object_set_new(rootJ, "running", json_integer((int) running));
    json_object_set_new(rootJ, "reverse", json_integer((int) reverse));
    json_object_set_new(rootJ, "resetOnStart", json_boolean(resetOnStart));
    json_object_set_new(rootJ, "resetOnStop", json_boolean(resetOnStop));
    return rootJ;
  }

  void dataFromJson(json_t *rootJ) override {
    json_t *runningJ = json_object_get(rootJ, "running");
    json_t *reverseJ = json_object_get(rootJ, "reverse");
    json_t *resetOnStartJ = json_object_get(rootJ, "resetOnStart");
    json_t *resetOnStopJ = json_object_get(rootJ, "resetOnStop");
    if (runningJ) {
      running = json_integer_value(runningJ);
    }
    if (reverseJ) {
      reverse = json_integer_value(reverseJ);
    }
    if (resetOnStartJ) { resetOnStart = json_boolean_value(resetOnStartJ); }
    if (resetOnStopJ) { resetOnStop = json_boolean_value(resetOnStopJ); }
  }
};


void Clock::process(const ProcessArgs &args) {
  lastMode = mode;
  mode = detectMode();

  processButtons();
  processSwingInputs();

  if (mode == INTERNAL_MODE || mode == EXT_VBPS_MODE || mode == EXT_CLOCK_MODE) {

    if (mode == EXT_CLOCK_MODE) {
      if (lastMode != EXT_CLOCK_MODE || resetWasHit) {
        clockTracker.init();
        clockTracker.freq = fabsf(bpm / 60.0f);
      }
      clockTracker.process(args.sampleTime, inputs[CLOCK_INPUT].getVoltage());
      if (clockTracker.freqDetected) {
        bpm = clockTracker.freq * 60.0f;
      }
    } else if (mode == EXT_VBPS_MODE) {
      bpm = params[BPM_PARAM].getValue() + inputs[VBPS_INPUT].getVoltage() * 60.0f;
    } else {
      bpm = params[BPM_PARAM].getValue();
    }

    if (reverse) {
      bpm = bpm * -1.0f;
    }

    if (oscillator.freqCorrectionSuggestion == 0.0f) {
      oscillator.setPitch(bpm / 60.0f);
    }

    if (running) {

      if (resetWasHit) {
        oscillator.reset((reverse ? 1.0f : 0.0f));
      }

      bool phaseFlipped = oscillator.step(args.sampleTime);
      if (mode == EXT_CLOCK_MODE) {
        oscillator.adjustPhase(inputs[CLOCK_INPUT].getVoltage());
      }

      if (phaseFlipped || resetWasHit) {
        clockPulseGenerator.trigger(1e-3f);
        clock8thsPulseGenerator.trigger(1e-3f);
        clock16thsPulseGenerator.trigger(1e-3f);
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
      triggered = externalClockTrigger.process(inputs[CLOCK_INPUT].getVoltage());
    }

    if (running) {
      if (triggered) {
        clockPulseGenerator.trigger(1e-3f);
        clock8thsPulseGenerator.trigger(1e-3f);
        clock16thsPulseGenerator.trigger(1e-3f);
      } else {
        if (lastMode == EXT_PHASE_MODE || lastMode == EXT_CLOCK_AND_PHASE_MODE) {
          triggerThsByPhase(inputs[PHASE_INPUT].getVoltage() / 10.0f, lastExtPhase  / 10.0f);
        }
      }
    }
    lastExtPhase = inputs[PHASE_INPUT].getVoltage();
  }

  // Generate Pulse
  clockPulse = clockPulseGenerator.process(args.sampleTime);
  clock8thsPulse = clock8thsPulseGenerator.process(args.sampleTime);
  clock16thsPulse = clock16thsPulseGenerator.process(args.sampleTime);
  runPulse = runPulseGenerator.process(args.sampleTime);
  resetPulse = resetPulseGenerator.process(args.sampleTime);

  outputs[CLOCK_OUTPUT].setVoltage(clockPulse ? 10.0f : 0.0f);
  outputs[CLOCK_8THS_OUTPUT].setVoltage(clock8thsPulse ? 10.0f : 0.0f);
  outputs[CLOCK_16THS_OUTPUT].setVoltage(clock16thsPulse ? 10.0f : 0.0f);
  outputs[RUN_OUTPUT].setVoltage(runPulse ? 10.0f : 0.0f);
  outputs[RESET_OUTPUT].setVoltage(resetPulse ? 10.0f : 0.0f);

  // Output Voltages
  if (mode == EXT_PHASE_MODE || mode == EXT_CLOCK_AND_PHASE_MODE) {
    outputs[PHASE_OUTPUT].setVoltage(inputs[PHASE_INPUT].getVoltage());
  } else {
    outputs[PHASE_OUTPUT].setVoltage(oscillator.phase * 10.0f);
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
}


struct ClockWidget : ModuleWidget {
  ClockWidget(Clock *module);
  void appendContextMenu(Menu *menu) override;
  void onHoverKey(const event::HoverKey& e) override;
};

ClockWidget::ClockWidget(Clock *module) {
  setModule(module);
  setPanel(APP->window->loadSvg(asset::plugin(pluginInstance, "res/panels/Clock.svg")));

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

  addChild(createLight<SmallLight<ZZC_YellowLight>>(Vec(71.75, 66.5), module, Clock::CLOCK_LED));

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

void ClockWidget::appendContextMenu(Menu *menu) {
  menu->addChild(new MenuSeparator());

  Clock *clock = dynamic_cast<Clock*>(module);
  assert(clock);

  ClockResetOnStartItem *resetOnStartItem = createMenuItem<ClockResetOnStartItem>("Reset on Start");
  resetOnStartItem->clock = clock;
  menu->addChild(resetOnStartItem);

  ClockResetOnStopItem *resetOnStopItem = createMenuItem<ClockResetOnStopItem>("Reset on Stop");
  resetOnStopItem->clock = clock;
  menu->addChild(resetOnStopItem);
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
