#include "ZZC.hpp"
#include "dsp/Wavetable.hpp"

struct WavetablePlayer : Module {
  enum ParamIds {
    NUM_PARAMS
  };
  enum InputIds {
    NUM_INPUTS
  };
  enum OutputIds {
    WAVE_OUTPUT,
    NUM_OUTPUTS
  };
  enum LightIds {
    NUM_LIGHTS
  };

  Wavetable* wt;
  float phase = 0.f;
  float wave = 0.f;
  bool wtIsReady = false;

  WavetablePlayer();
  void process(const ProcessArgs &args) override;
  json_t *dataToJson() override;
  void dataFromJson(json_t *rootJ) override;

  void selectFolder();
};
