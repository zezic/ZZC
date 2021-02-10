#include "ZZC.hpp"
#include "dsp/Wavetable.hpp"

struct WavetablePlayer : Module {
  enum ParamIds {
    INDEX_PARAM,
    INDEX_CV_ATT_PARAM,
    NUM_PARAMS
  };
  enum InputIds {
    INDEX_CV_INPUT,
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
  float level = 0.f;
  int lastMipmapLevel = 0;
  bool wtIsReady = false;

  WavetablePlayer();
  void process(const ProcessArgs &args) override;
  json_t *dataToJson() override;
  void dataFromJson(json_t *rootJ) override;

  void selectFolder();
};
