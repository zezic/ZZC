#include "dsp/digital.hpp"

struct LowFrequencyOscillator {
  float phase = 0.0f;
  float lastPhase = 0.0f;
  float freq = 1.0f;

  struct NormalizationResult {
    float value;
    bool normalized;
  };

  LowFrequencyOscillator() {}

  void setPitch(float pitch) {
    freq = pitch;
  }

  void reset(float value) {
    NormalizationResult result = normalize(value);
    phase = result.value;
  } 

  NormalizationResult normalize(float value) {
    if (value >= 1.0f) {
      value = fmod(value, 1.0f);
      NormalizationResult result = { value, true };
      return result;
    }
    if (value < 0.0f) {
      value = 1.0f - fmod(fabs(value), 1.0f);
      NormalizationResult result = { value, true };
      return result;
    }
    NormalizationResult result = { value, false };
    return result;
  }

  bool step(float dt) {
    float deltaPhase = freq * dt;
    NormalizationResult result = normalize(phase + deltaPhase);
    lastPhase = phase;
    phase = result.value;
    return result.normalized;
  }
};

struct ClockTracker {
  int triggersPassed;
  float period;
  float freq;
  bool freqDetected;

  SchmittTrigger clockTrigger;

  void init() {
    triggersPassed = 0;
    period = 0.0f;
    freq = 0.0f;
    freqDetected = false;
  }

  void process(float dt, float clock, float smooth) {
    period += dt;
    if (clockTrigger.process(clock)) {
      if (triggersPassed < 2) {
        triggersPassed += 1;
      }
      if (triggersPassed > 1) {
        freqDetected = true;
        float targetFreq = 1.0f / period;
        if (smooth > 1.0f) {
          freq += (targetFreq - freq) / smooth;
        } else {
          freq = targetFreq;
        }
      }
      period = 0.0f;
    }
  }
};