#include "ZZC.hpp"

struct RatioDisplayData {
  float numerator = 1.f;
  float denominator = 1.f;
};

struct Phasor : Module {
  enum ParamIds {
    REVERSE_PARAM,
    LFO_PARAM,
    FREQ_CRSE_PARAM,
    FREQ_FINE_PARAM,
    NUMERATOR_PARAM,
    DENOMINATOR_PARAM,
    HARDSYNC_PARAM,
    PM_PARAM,
    FM_PARAM,
    NUM_PARAMS
  };
  enum InputIds {
    SYNC_INPUT,
    PM_INPUT,
    FM_INPUT,
    VOCT_INPUT,
    NUM_INPUTS
  };
  enum OutputIds {
    SIN_OUTPUT,
    COS_OUTPUT,
    PHASE_OUTPUT,
    NUM_OUTPUTS
  };
  enum LightIds {
    HARDSYNC_LED,
    USELESS_LIGHT,
    NUM_LIGHTS
  };

  std::shared_ptr<RatioDisplayData> rddPtr;
  dsp::ClockDivider rddFeedDivider;

  Phasor();
  void process(const ProcessArgs &args) override;
  json_t *dataToJson() override;
  void dataFromJson(json_t *rootJ) override;
};