#include "ZZC.hpp"
#include "dsp/Wavetable.hpp"

struct WavetablePlayer : Module {
  enum ParamIds {
    INDEX_PARAM,
    INDEX_CV_ATT_PARAM,
    MIPMAP_PARAM,
    INDEX_INTER_PARAM,
    // XTRA_PARAM,
    NUM_PARAMS
  };
  enum InputIds {
    PHASE_INPUT,
    INDEX_CV_INPUT,
    NUM_INPUTS
  };
  enum OutputIds {
    WAVE_OUTPUT,
    // INTER_OUTPUT,
    // XTRA_OUTPUT,
    NUM_OUTPUTS
  };
  enum LightIds {
    NUM_LIGHTS
  };

  std::shared_ptr<Wavetable> wtPtr = std::shared_ptr<Wavetable>(nullptr);
  float wave = 0.f;
  float level = 0.f;
  int lastMipmapLevel = 0;
  bool wtIsReady = false;

  float index = 0.f;
  int indexIntpart = 0;
  float interpolation = 0.f;
  bool indexInter = true;

  float lastPhase = 0.f;
  dsp::ClockDivider debugDivider;

  std::string filename;

  WavetablePlayer();
  void process(const ProcessArgs &args) override;
  json_t *dataToJson() override;
  void dataFromJson(json_t *rootJ) override;

  void selectFile();
  void switchFile(int delta);
  bool tryToLoadWT(std::string path);
};
