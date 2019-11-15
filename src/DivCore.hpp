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

  bool process(float phaseIn) {
    phaseIn = fmod(phaseIn, 10.f);
    float phaseInDelta = phaseIn - lastPhaseIn;
    if (std::abs(phaseInDelta) > 0.1f && (sgn(phaseInDelta) != sgn(lastPhaseInDelta))) {
      phaseInDelta = lastPhaseInDelta;
    }
    lastPhaseInDelta = phaseInDelta;
    phase += phaseInDelta * ratio;
    while (phase >= 10.0) {
      phase -= 10.0;
    }
    while (phase < 0.0) {
      phase += 10.0;
    }
    bool flipped = abs(lastPhase - phase) > 9.0;
    lastPhase = phase;
    lastPhaseIn = phaseIn;
    return flipped;
  }
};
