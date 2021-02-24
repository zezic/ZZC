#include "ZZC.hpp"

struct Phasor : Module {
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

  Phasor();
  void process(const ProcessArgs &args) override;
  json_t *dataToJson() override;
  void dataFromJson(json_t *rootJ) override;
};