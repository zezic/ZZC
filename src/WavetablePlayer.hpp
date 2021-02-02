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
    NUM_OUTPUTS
  };
  enum LightIds {
    NUM_LIGHTS
  };

  Wavetable* wt;

  WavetablePlayer();
  void process(const ProcessArgs &args) override;
  json_t *dataToJson() override;
  void dataFromJson(json_t *rootJ) override;

  void selectFolder();
};
