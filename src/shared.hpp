#include "math.hpp"
#include "dsp/digital.hpp"

using namespace rack;

struct LowFrequencyOscillator {
  float phase = 0.0f;
  float lastPhase = 0.0f;
  float freq = 1.0f;
  float freqCorrectionSuggestion = 0.0f;
  float freqCorrection = 0.0f;

  dsp::SchmittTrigger clockTrigger;

  struct NormalizationResult {
    float value;
    bool normalized;
  };

  LowFrequencyOscillator() {}

  void setPitch(float pitch) {
    freq = pitch;
  }

  void reset(float value) {
    this->phase = std::fmod(value, 1.0f);
  }

  bool step(float dt) {
    float deltaPhase = (freq + this->freqCorrection) * dt;
    float summ = phase + deltaPhase;
    this->phase = rack::math::eucMod(summ, 1.0f);
    bool flipped = freq >= 0.0f ? summ >= 1.0f : summ < 0.0f;
    if (flipped) {
      if (this->freqCorrectionSuggestion != 0.0f) {
        this->freqCorrection = this->freqCorrectionSuggestion;
        this->freqCorrectionSuggestion = 0.0f;
      } else {
        this->freqCorrection = 0.0f;
      }
    }
    return flipped;
  }

  void adjustPhase(float pulse) {
    if (!this->clockTrigger.process(pulse)) { return; }
    if (phase == 0.0f) {
      this->freqCorrection = 0.0f;
      return;
    }
    if (freq >= 0.0f) {
      if (phase < 0.5f) {
        // We are moving too fast
        this->freqCorrectionSuggestion = this->freq * ((1.0f - this->phase) / 1.0f) - this->freq;
      } else {
        // We are lagging behind
        this->freqCorrectionSuggestion = this->freq / this->phase - this->freq;
      }
    } else {
      if (phase < 0.5f) {
        // We are lagging behind
        this->freqCorrectionSuggestion = (-this->freq) / (1.0f - this->phase) + this->freq;
      } else {
        // We are moving too fast
        this->freqCorrectionSuggestion = (-this->freq) * ((this->phase) / 1.0f) + this->freq;
      }
    }
  }
};

struct ClockTracker {
  int triggersPassed;
  float period;
  float freq;
  bool freqDetected;

  dsp::SchmittTrigger clockTrigger;

  void init() {
    triggersPassed = 0;
    period = 0.0f;
    freq = 0.0f;
    freqDetected = false;
  }

  void process(float dt, float pulse) {
    period += dt;
    if (clockTrigger.process(pulse)) {
      if (triggersPassed < 3) {
        triggersPassed += 1;
      }
      if (triggersPassed > 2) {
        freqDetected = true;
        freq = 1.0f / period;
      }
      period = 0.0f;
    }
  }
};
