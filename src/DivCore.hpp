#include "ZZC.hpp"

using simd::float_4;

struct SchmittTriggerT {
  float lastValue;

  bool process(float value) {
    if (value > 1.f && lastValue <= 1.f) {
      lastValue = value;
      return true;
    }
    lastValue = value;
    return false;
  }
};

template <typename T>
struct DivCore {
  T phase = 0.f;
  T phase10 = 0.f;
  T lastPhaseIn = 0.f;
  T lastPhaseInDelta = 0.f;
  T multiplier = 1.f;

  void process(T newPhaseIn) {
    T newPhaseInMod = fmod(newPhaseIn, 1.f);
    T phaseInDelta = newPhaseInMod - lastPhaseIn;
    T signMask = sgn(phaseInDelta) != sgn(lastPhaseInDelta);
    T bigDeltaMask = abs(phaseInDelta) > 0.1f;
    T phaseInWrapMask = signMask & bigDeltaMask;
    phaseInDelta = ifelse(phaseInWrapMask, lastPhaseInDelta, phaseInDelta);
    T multipliedDelta = phaseInDelta * multiplier;
    phase = phase + multipliedDelta;
    phase = fmod(phase, 1.f);
    phase = ifelse(phase < 0.f, 1.f - phase, phase);
    phase10 = phase * 10.f;

    lastPhaseInDelta = phaseInDelta;
    lastPhaseIn = newPhaseInMod;
  }
};
