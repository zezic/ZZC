#include "ZZC.hpp"
#include "dsp/Wavetable.hpp"

struct WavetablePlayer : Module {
  enum ParamIds {
    INDEX_PARAM,
    INDEX_CV_ATT_PARAM,
    XTRA_PARAM,
    MIPMAP_PARAM,
    INDEX_INTER_PARAM,
    NUM_PARAMS
  };
  enum InputIds {
    PHASE_INPUT,
    INDEX_CV_INPUT,
    NUM_INPUTS
  };
  enum OutputIds {
    INTER_OUTPUT,
    WAVE_OUTPUT,
    XTRA_OUTPUT,
    NUM_OUTPUTS
  };
  enum LightIds {
    NUM_LIGHTS
  };

  Wavetable* wt;
  std::shared_ptr<Wavetable> wtPtr;
  float wave = 0.f;
  float level = 0.f;
  int lastMipmapLevel = 0;
  bool wtIsReady = false;

  std::string filename = "";

  WavetablePlayer();
  void process(const ProcessArgs &args) override;
  json_t *dataToJson() override;
  void dataFromJson(json_t *rootJ) override;

  void selectFile();
  bool tryToLoadWT(std::string path);
};
