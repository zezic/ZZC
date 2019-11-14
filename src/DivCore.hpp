#include "ZZC.hpp"

using simd::float_4;

inline float_4 eucMod(float_4 a, float_4 b) {
  float_4 mod = fmod(a, b);
  mod = ifelse(mod < 0.f, mod + b, mod);
  return mod;
}

template <typename T>
struct DivCore {
  T phase = 0.f;
  T phase10 = 0.f;
  T lastPhaseIn = 0.f;
  T lastPhaseInDelta = 0.f;
  T multiplier = 1.f;

  void reset(T resetMask) {
    phase = ifelse(resetMask, 0.f, phase);
    phase10 = ifelse(resetMask, phase * 10.f, phase10);
    lastPhaseIn = ifelse(resetMask, phase, lastPhaseIn);
    lastPhaseInDelta = ifelse(resetMask, 0.f, lastPhaseInDelta);
  }

  T process(T newPhaseIn) {
    T newPhaseInMod = fmod(newPhaseIn, 1.f);
    T phaseInDelta = newPhaseInMod - lastPhaseIn;
    T signMask = sgn(phaseInDelta) != sgn(lastPhaseInDelta);
    T bigDeltaMask = abs(phaseInDelta) > 0.1f;
    T phaseInWrapMask = signMask & bigDeltaMask;
    phaseInDelta = ifelse(phaseInWrapMask, lastPhaseInDelta, phaseInDelta);
    T multipliedDelta = phaseInDelta * multiplier;

    T newPhase = eucMod(phase + multipliedDelta, 1.f);
    T flipMask = abs(newPhase - phase) > 0.9f;
    phase = newPhase;
    phase10 = phase * 10.f;

    lastPhaseInDelta = phaseInDelta;
    lastPhaseIn = newPhaseInMod;

    return flipMask;
  }
};
