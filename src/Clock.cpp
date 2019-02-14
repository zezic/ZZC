#include "ZZC.hpp"
#include "shared.hpp"
#include "widgets.hpp"


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

  enum Modes mode;
  enum Modes lastMode;

  float lastExtPhase = 0.0f;
  LowFrequencyOscillator oscillator;

  bool running = true;
  bool reverse = false;
  float bpm = 120.0f;

  float swing8thsFinal = 50.0f;
  float swing16thsFinal = 50.0f;

  float swinged8thsPhase = 0.5f;
  float swinged16thsFirstPhase = 0.25f;
  float swinged16thsSecondPhase = 0.75f;

  PulseGenerator clockPulseGenerator;
  PulseGenerator clock8thsPulseGenerator;
  PulseGenerator clock16thsPulseGenerator;
  PulseGenerator runPulseGenerator;
  PulseGenerator resetPulseGenerator;
  bool clockPulse = false;
  bool clock8thsPulse = false;
  bool clock16thsPulse = false;
  bool runPulse = false;
  bool resetPulse = false;
  bool resetWasHit = false;

  float clockLight = 0.0f;
  float resetLight = 0.0f;
  float reverseLight = 0.0f;
  
  SchmittTrigger runButtonTrigger;
  SchmittTrigger externalRunTrigger;
  SchmittTrigger resetButtonTrigger;
  SchmittTrigger externalResetTrigger;
  SchmittTrigger reverseButtonTrigger;
  SchmittTrigger externalClockTrigger;

  inline void processButtons() {
    if (runButtonTrigger.process(params[RUN_SWITCH_PARAM].value) || (inputs[EXT_RUN_INPUT].active && externalRunTrigger.process(inputs[EXT_RUN_INPUT].value))) {
      running = !running;
      runPulseGenerator.trigger(1e-3f);
    }

    if (resetButtonTrigger.process(params[RESET_SWITCH_PARAM].value) || (inputs[EXT_RESET_INPUT].active && externalResetTrigger.process(inputs[EXT_RESET_INPUT].value))) {
      resetWasHit = true;
      resetPulseGenerator.trigger(1e-3f);
    }

    if (reverseButtonTrigger.process(params[REVERSE_SWITCH_PARAM].value)) {
      reverse ^= true;
    }
  }

  inline void processSwingInputs() {
    swing8thsFinal = params[SWING_8THS_PARAM].value;
    if (inputs[SWING_8THS_INPUT].active) {
      float swing8thsInput = clamp(inputs[SWING_8THS_INPUT].value / 5.0f, -1.0f, 1.0f);
      if (swing8thsInput < 0.0f) {
        swing8thsFinal += (swing8thsFinal - 1.0f) * swing8thsInput;
      } else if (swing8thsInput > 0.0f) {
        swing8thsFinal += (99.0f - swing8thsFinal) * swing8thsInput;
      }
    }
    swing16thsFinal = params[SWING_16THS_PARAM].value;
    if (inputs[SWING_16THS_INPUT].active) {
      float swing16thsInput = clamp(inputs[SWING_16THS_INPUT].value / 5.0f, -1.0f, 1.0f);
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
    if (inputs[CLOCK_INPUT].active && inputs[PHASE_INPUT].active) {
      return EXT_CLOCK_AND_PHASE_MODE;
    } else if (inputs[CLOCK_INPUT].active) {
      return EXT_CLOCK_MODE;
    } else if (inputs[PHASE_INPUT].active) {
      return EXT_PHASE_MODE;
    } else if (inputs[VBPS_INPUT].active) {
      return EXT_VBPS_MODE;
    }
    return INTERNAL_MODE;
  }

  Clock() : Module(NUM_PARAMS, NUM_INPUTS, NUM_OUTPUTS, NUM_LIGHTS) {
    clockTracker.init();
  }
  void step() override;

  json_t *toJson() override {
    json_t *rootJ = json_object();
    json_object_set_new(rootJ, "running", json_integer((int) running));
    json_object_set_new(rootJ, "reverse", json_integer((int) reverse));
    return rootJ;
  }

  void fromJson(json_t *rootJ) override {
    json_t *runningJ = json_object_get(rootJ, "running");
    json_t *reverseJ = json_object_get(rootJ, "reverse");
    if (runningJ) {
      running = json_integer_value(runningJ);
    }
    if (reverseJ) {
      reverse = json_integer_value(reverseJ);
    }
  }
};


void Clock::step() {
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
      clockTracker.process(engineGetSampleTime(), inputs[CLOCK_INPUT].value);
      if (clockTracker.freqDetected) {
        bpm = clockTracker.freq * 60.0f;
      }
    } else if (mode == EXT_VBPS_MODE) {
      bpm = params[BPM_PARAM].value + inputs[VBPS_INPUT].value * 60.0f;
    } else {
      bpm = params[BPM_PARAM].value;
    }

    if (reverse) {
      bpm = bpm * -1.0f;
    }
    oscillator.setPitch(bpm / 60.0f);

    if (running) {

      if (resetWasHit) {
        oscillator.reset((reverse ? 1.0f : 0.0f));
      }

      bool phaseFlipped = oscillator.step(engineGetSampleTime());
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
      float delta = inputs[PHASE_INPUT].value - lastExtPhase;
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
      bpm = delta / engineGetSampleTime() * 6.0f;
    }

    if (mode == EXT_CLOCK_AND_PHASE_MODE) {
      triggered = externalClockTrigger.process(inputs[CLOCK_INPUT].value);
    }

    if (running) {
      if (triggered) {
        clockPulseGenerator.trigger(1e-3f);
        clock8thsPulseGenerator.trigger(1e-3f);
        clock16thsPulseGenerator.trigger(1e-3f);
      } else {
        if (lastMode == EXT_PHASE_MODE || lastMode == EXT_CLOCK_AND_PHASE_MODE) {
          triggerThsByPhase(inputs[PHASE_INPUT].value / 10.0f, lastExtPhase  / 10.0f);
        }
      }
    }
    lastExtPhase = inputs[PHASE_INPUT].value;
  }

  // Generate Pulse
  clockPulse = clockPulseGenerator.process(engineGetSampleTime());
  clock8thsPulse = clock8thsPulseGenerator.process(engineGetSampleTime());
  clock16thsPulse = clock16thsPulseGenerator.process(engineGetSampleTime());
  runPulse = runPulseGenerator.process(engineGetSampleTime());
  resetPulse = resetPulseGenerator.process(engineGetSampleTime());

  outputs[CLOCK_OUTPUT].value = clockPulse ? 10.0f : 0.0f;
  outputs[CLOCK_8THS_OUTPUT].value = clock8thsPulse ? 10.0f : 0.0f;
  outputs[CLOCK_16THS_OUTPUT].value = clock16thsPulse ? 10.0f : 0.0f;
  outputs[RUN_OUTPUT].value = runPulse ? 10.0f : 0.0f;
  outputs[RESET_OUTPUT].value = resetPulse ? 10.0f : 0.0f;

  // Output Voltages
  if (mode == EXT_PHASE_MODE || mode == EXT_CLOCK_AND_PHASE_MODE) {
    outputs[PHASE_OUTPUT].value = inputs[PHASE_INPUT].value;
  } else {
    outputs[PHASE_OUTPUT].value = oscillator.phase * 10.0f;
  }
  outputs[VBPS_OUTPUT].value = bpm / 60.0f;
  outputs[VSPB_OUTPUT].value = bpm == 0.0f ? 10.0f : fminf(60.0f / fabsf(bpm), 10.0f);

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
  ClockWidget(Clock *module) : ModuleWidget(module) {
    setPanel(SVG::load(assetPlugin(plugin, "res/panels/Clock.svg")));

		addParam(ParamWidget::create<ZZC_PreciseKnobSnappy>(Vec(49, 59), module, Clock::BPM_PARAM, 0.0f, 240.0f, 120.0f));
    addChild(ModuleLightWidget::create<TinyLight<GreenLight>>(Vec(94.5f, 63), module, Clock::INTERNAL_MODE_LED));
		addParam(ParamWidget::create<ZZC_Knob21Snappy>(Vec(12, 276), module, Clock::SWING_8THS_PARAM, 1.0f, 99.0f, 50.0f));
		addParam(ParamWidget::create<ZZC_Knob21Snappy>(Vec(117, 276), module, Clock::SWING_16THS_PARAM, 1.0f, 99.0f, 50.0f));

    addParam(ParamWidget::create<ZZC_LEDBezelDark>(Vec(11.3f, 119.0f), module, Clock::REVERSE_SWITCH_PARAM, 0.0f, 1.0f, 0.0f));
    addChild(ModuleLightWidget::create<LedLight<ZZC_YellowLight>>(Vec(13.1f, 120.7f), module, Clock::REVERSE_LED));

    addInput(Port::create<ZZC_PJ_In_Port>(Vec(10, 72), Port::INPUT, module, Clock::VBPS_INPUT));
    addChild(ModuleLightWidget::create<TinyLight<GreenLight>>(Vec(32, 72), module, Clock::EXT_VBPS_MODE_LED));

    addInput(Port::create<ZZC_PJ_In_Port>(Vec(45, 168), Port::INPUT, module, Clock::CLOCK_INPUT));
    addChild(ModuleLightWidget::create<TinyLight<GreenLight>>(Vec(67, 168), module, Clock::EXT_CLOCK_MODE_LED));
    addInput(Port::create<ZZC_PJ_In_Port>(Vec(80, 168), Port::INPUT, module, Clock::PHASE_INPUT));
    addChild(ModuleLightWidget::create<TinyLight<GreenLight>>(Vec(102, 168), module, Clock::EXT_PHASE_MODE_LED));

    addOutput(Port::create<ZZC_PJ_Out_Port>(Vec(45, 231), Port::OUTPUT, module, Clock::CLOCK_OUTPUT));
    addOutput(Port::create<ZZC_PJ_Out_Port>(Vec(80, 231), Port::OUTPUT, module, Clock::PHASE_OUTPUT));

    addOutput(Port::create<ZZC_PJ_Out_Port>(Vec(115, 72), Port::OUTPUT, module, Clock::VBPS_OUTPUT));
    addOutput(Port::create<ZZC_PJ_Out_Port>(Vec(115, 119), Port::OUTPUT, module, Clock::VSPB_OUTPUT));

    addChild(ModuleLightWidget::create<SmallLight<ZZC_YellowLight>>(Vec(93, 42.5), module, Clock::CLOCK_LED));

    Display32Widget *bpmDisplay = new Display32Widget();
    bpmDisplay->box.pos = Vec(46.0f, 121.0f);
    bpmDisplay->box.size = Vec(58.0f, 21.0f);
    bpmDisplay->value = &module->bpm;
    addChild(bpmDisplay);

    addInput(Port::create<ZZC_PJ_In_Port>(Vec(10, 168), Port::INPUT, module, Clock::EXT_RUN_INPUT));
    addParam(ParamWidget::create<ZZC_LEDBezelDark>(Vec(11.3f, 199.0f), module, Clock::RUN_SWITCH_PARAM, 0.0f, 1.0f, 0.0f));
    addChild(ModuleLightWidget::create<LedLight<ZZC_YellowLight>>(Vec(13.1f, 200.7f), module, Clock::RUN_LED));
    addOutput(Port::create<ZZC_PJ_Out_Port>(Vec(10, 231), Port::OUTPUT, module, Clock::RUN_OUTPUT));

    addInput(Port::create<ZZC_PJ_In_Port>(Vec(115, 168), Port::INPUT, module, Clock::EXT_RESET_INPUT));
    addParam(ParamWidget::create<ZZC_LEDBezelDark>(Vec(116.3f, 199.0f), module, Clock::RESET_SWITCH_PARAM, 0.0f, 1.0f, 0.0f));
    addChild(ModuleLightWidget::create<LedLight<ZZC_YellowLight>>(Vec(118.1f, 200.7f), module, Clock::RESET_LED));
    addOutput(Port::create<ZZC_PJ_Out_Port>(Vec(115, 231), Port::OUTPUT, module, Clock::RESET_OUTPUT));

    addInput(Port::create<ZZC_PJ_In_Port>(Vec(10, 319), Port::INPUT, module, Clock::SWING_8THS_INPUT));
    addInput(Port::create<ZZC_PJ_In_Port>(Vec(115, 319), Port::INPUT, module, Clock::SWING_16THS_INPUT));
    addOutput(Port::create<ZZC_PJ_Out_Port>(Vec(45, 319), Port::OUTPUT, module, Clock::CLOCK_8THS_OUTPUT));
    addOutput(Port::create<ZZC_PJ_Out_Port>(Vec(80, 319), Port::OUTPUT, module, Clock::CLOCK_16THS_OUTPUT));

    DisplayIntpartWidget *swing8thsDisplay = new DisplayIntpartWidget();
    swing8thsDisplay->box.pos = Vec(43.0f, 276.0f);
    swing8thsDisplay->box.size = Vec(29.0f, 21.0f);
    swing8thsDisplay->value = &module->swing8thsFinal;
    addChild(swing8thsDisplay);

    DisplayIntpartWidget *swing16thsDisplay = new DisplayIntpartWidget();
    swing16thsDisplay->box.pos = Vec(78.0f, 276.0f);
    swing16thsDisplay->box.size = Vec(29.0f, 21.0f);
    swing16thsDisplay->value = &module->swing16thsFinal;
    addChild(swing16thsDisplay);

		addChild(Widget::create<ZZC_Screw>(Vec(RACK_GRID_WIDTH, 0)));
		addChild(Widget::create<ZZC_Screw>(Vec(box.size.x - 2 * RACK_GRID_WIDTH, 0)));
		addChild(Widget::create<ZZC_Screw>(Vec(RACK_GRID_WIDTH, RACK_GRID_HEIGHT - RACK_GRID_WIDTH)));
		addChild(Widget::create<ZZC_Screw>(Vec(box.size.x - 2 * RACK_GRID_WIDTH, RACK_GRID_HEIGHT - RACK_GRID_WIDTH)));
  }
};


Model *modelClock = Model::create<Clock, ClockWidget>("ZZC", "Clock", "Clock", CLOCK_TAG);
