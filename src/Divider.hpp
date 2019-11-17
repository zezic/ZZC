#include "ZZC.hpp"
#include "Clock.hpp"

struct Divider : Module {
  enum ParamIds {
    IN_RATIO_PARAM,
    OUT_RATIO_PARAM,
    SWING_PARAM,
    NUM_PARAMS
  };
  enum InputIds {
    IN_RATIO_INPUT,
    OUT_RATIO_INPUT,
    SWING_INPUT,
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
    EXT_PHASE_MODE_LED,
    NUM_LIGHTS
  };

  float from = 1.0f;
  float to = 1.0f;
  float ratio = 1.0f;
  float swing = 50.0f;

  float phaseIn = 0.0f;
  float lastPhaseIn = 0.0f;
  float lastPhaseInDelta = 0.0f;
  bool lastPhaseInState = false;

  double halfPhaseOut = 0.0;
  double lastHalfPhaseOut = 0.0;
  float phaseOut = 0.0f;

  dsp::PulseGenerator clockPulseGenerator;
  dsp::PulseGenerator resetPulseGenerator;
  bool clockPulse = false;
  bool resetPulse = false;

  /* Expander stuff */
  ZZC_TransportMessage leftMessages[2];
  ZZC_TransportMessage rightMessages[2];
  ZZC_TransportMessage cleanMessage;

  /* Settings */
  bool gateMode = false;
  bool tickOnStart = false;

  dsp::SchmittTrigger clockTrigger;
  dsp::SchmittTrigger resetTrigger;

  inline void processRatioInputs();
  inline void processSwingInput();

  Divider();
  void process(const ProcessArgs &args) override;

  json_t *dataToJson() override;
  void dataFromJson(json_t *rootJ) override;
};
