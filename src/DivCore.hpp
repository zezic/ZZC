#include "ZZC.hpp"

using simd::float_4;

inline float_4 eucMod(float_4 a, float_4 b) {
  float_4 mod = fmod(a, b);
  mod = ifelse(mod < 0.f, mod + b, mod);
  return mod;
}

template <typename T>
struct PolyDivCore {
  T phase = 0.f;
  T lastPhaseIn = 0.f;
  T lastPhaseInDelta = 0.f;
  T multiplier = 1.f;

  void reset(T resetMask) {
    phase = ifelse(resetMask, 0.f, phase);
    lastPhaseIn = ifelse(resetMask, phase, lastPhaseIn);
    lastPhaseInDelta = ifelse(resetMask, 0.f, lastPhaseInDelta);
  }

  T process(T newPhaseIn) {
    T newPhaseInMod = fmod(newPhaseIn, 10.f);
    T phaseInDelta = newPhaseInMod - lastPhaseIn;
    T signMask = sgn(phaseInDelta) != sgn(lastPhaseInDelta);
    T bigDeltaMask = abs(phaseInDelta) > 1.f;
    T phaseInWrapMask = signMask & bigDeltaMask;
    phaseInDelta = ifelse(phaseInWrapMask, lastPhaseInDelta, phaseInDelta);
    T multipliedDelta = phaseInDelta * multiplier;

    T newPhase = eucMod(phase + multipliedDelta, 10.f);
    T flipMask = abs(newPhase - phase) > 9.f;
    phase = newPhase;

    lastPhaseInDelta = phaseInDelta;
    lastPhaseIn = newPhaseInMod;

    return flipMask;
  }
};

struct MonoDivCore {
  float ratio = 1.f;
  float requestedRatio = 1.f;
  bool ratioIsRequested = false;
  float lastPhaseIn = 0.f;
  float lastPhaseInDelta = 0.f;
  double phase = 0.0;
  double lastPhase = 0.0;

  void reset() {
    phase = 0.0;
    lastPhase = 0.0;
    lastPhaseIn = 0.f;
    lastPhaseInDelta = 0.f;
  }

  void requestRatio(float newRatio) {
    if (newRatio == ratio) {
      this->ratioIsRequested = false;
      return;
    }
    this->requestedRatio = newRatio;
    this->ratioIsRequested = true;
  }

  void setRatio(float newRatio) {
    this->ratio = newRatio;
    this->ratioIsRequested = false;
  }

  bool process(float phaseIn) {
    phaseIn = fmod(phaseIn, 10.f);
    float phaseInDelta = phaseIn - lastPhaseIn;
    bool masterFlipped = std::abs(phaseInDelta) > 0.1f && (sgn(phaseInDelta) != sgn(lastPhaseInDelta));
    if (masterFlipped) {
      phaseInDelta = lastPhaseInDelta;
    }
    lastPhaseInDelta = phaseInDelta;
    double newPhase = phase + phaseInDelta * ratio;
    while (newPhase >= 10.0) {
      newPhase -= 10.0;
    }
    while (newPhase < 0.0) {
      newPhase += 10.0;
    }
    bool slaveFlipped = abs(lastPhase - newPhase) > 9.0;
    if (ratioIsRequested &&
        ((masterFlipped && (this->ratio >= 1.f)) ||
         (slaveFlipped && (this->ratio < 1.f)))) {
      this->ratioIsRequested = false;
      this->ratio = this->requestedRatio;
      bool goingReverse = phaseIn > 9.f;
      this->phase = goingReverse ? 10.0 - (10.0 - phaseIn) * this->ratio : phaseIn * this->ratio;
    } else {
      this->phase = newPhase;
    }
    lastPhase = this->phase;
    lastPhaseIn = phaseIn;
    return slaveFlipped;
  }
};

struct DivBase {
  float clockOutput = 0.f;
  float phaseOutput = 0.f;

  float combinedMultiplier = 1.f;
  bool combinedMultiplierDirty = false;
  float roundedMultiplier = 1.f;

  float paramMultiplier = 1.f;
  float cvMultiplier = 1.f;

  float lastParamMultiplier = 1.f;
  float lastCVMultiplier = 1.f;

  float lastFractionParam = 1.f;
  float lastCVVoltage = 0.f;

  MonoDivCore monoDivCore;
  PulseGenerator pulseGenerator;

  int fractionDisplay = 1;
  int fractionDisplayPolarity = 0;

  /* Settings */
  bool gateMode = false;
  bool sync = false;

  /* State */
  bool firstCall = true;

  void reset() {
    this->monoDivCore.reset();
  }

  void process(float phaseIn, float sampleTime);

  void handleFractionParam(float value);
  void handleCV(float cvVoltage);
  void combineMultipliers();
};
