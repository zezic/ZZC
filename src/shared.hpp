#include "math.hpp"
#include "dsp/digital.hpp"

#ifndef ZZC_SHARED_H
#define ZZC_SHARED_H

using namespace rack;

template <typename T>
struct TPulseGenerator {
  T remaining = 0.f;

  T process(float deltaTime) {
    T output = remaining > 0.f;
    remaining = simd::ifelse(output, remaining - deltaTime, remaining);
    return output;
  }

  void trigger(T mask, float duration = 1e-3f) {
    remaining = simd::ifelse(mask & (duration > remaining), duration, remaining);
  }
};

struct LowFrequencyOscillator {
  float phase = 0.0f;
  float lastPhase = 0.0f;
  float freq = 1.0f;
  float freqCorrection = 0.0f;

  int PPQN = 1;

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
    this->freqCorrection = 0.0f;
  }

  bool step(float dt) {
    float deltaPhase = (freq + this->freqCorrection) * dt;
    float summ = phase + deltaPhase;
    this->lastPhase = this->phase;
    this->phase = rack::math::eucMod(summ, 1.0f);
    bool flipped = freq >= 0.0f ? summ >= 1.0f : summ < 0.0f;
    return flipped;
  }

  void adjustPhase(float pulse) {
    if (!this->clockTrigger.process(pulse)) { return; }
    if (phase == 0.0f) {
      this->freqCorrection = 0.0f;
      return;
    }
    float segmentLength = 1.0f / this->PPQN;
    float absoluteSegmentPhase = std::fmod(this->phase, segmentLength);
    float scaledPhase = absoluteSegmentPhase * this->PPQN;
    float deviation = scaledPhase - (scaledPhase < 0.5f ? 0.0f : 1.0f);
    this->freqCorrection = -std::abs(this->freq) * deviation;
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

#endif
