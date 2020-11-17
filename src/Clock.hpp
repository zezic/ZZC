#include "ZZC.hpp"

struct ZZC_TransportMessage {
  bool hasClock = false;
  float clockPhase = 0.f;
  bool clockReset = false;
  bool clockFlip = false;

  bool hasDivider = false;
  float dividerPhase = 0.f;
  bool dividerReset = false;
  bool dividerFlip = false;

  bool hasDiv = false;
  float divPhase = 0.f;
  bool divReset = false;
  bool divFlip = false;

  bool hasDivExp = false;
  float divExpPhase = 0.f;
  bool divExpReset = false;
  bool divExpFlip = false;
};

struct Clock : Module {
  enum ParamIds {
    BPM_PARAM,
    SWING_8THS_PARAM,
    SWING_16THS_PARAM,
    RUN_SWITCH_PARAM,
    RESET_SWITCH_PARAM,
    REVERSE_SWITCH_PARAM,
    USE_COMPATIBLE_BPM_CV_PARAM,
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
  bool resetWasHitForMessage = false;

  float clockLight = 0.0f;
  float resetLight = 0.0f;
  float reverseLight = 0.0f;

  dsp::SchmittTrigger runButtonTrigger;
  dsp::SchmittTrigger externalRunTrigger;
  dsp::SchmittTrigger resetButtonTrigger;
  dsp::SchmittTrigger externalResetTrigger;
  dsp::SchmittTrigger reverseButtonTrigger;
  dsp::SchmittTrigger externalClockTrigger;

  /* Expander stuff */
  ZZC_TransportMessage leftMessages[2];
  ZZC_TransportMessage rightMessages[2];
  ZZC_TransportMessage cleanMessage;

  /* Settings */
  bool baseClockGateMode = false;
  bool x2ClockGateMode = false;
  bool x4ClockGateMode = false;
  bool resetOnStart = false;
  bool resetOnStop = false;
  bool runInputIsGate = false;
  bool runOutputIsGate = false;
  bool useCompatibleBPMCV = true;
  bool snapCV = false;
  int externalClockPPQN = 1;
  float phaseOutputOffset = 0.0f;

  Clock();
  void toggle();
  inline void processButtons();
  inline void processSwingInputs();
  inline void triggerThsByPhase(float phase, float lastPhase);
  inline enum Modes detectMode();
  void process(const ProcessArgs &args) override;
  json_t *dataToJson() override;
  void dataFromJson(json_t *rootJ) override;
};


