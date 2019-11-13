#include "ZZC.hpp"

using simd::float_4;

template <typename T>
struct PolySchmittTrigger {
  T lastValue = 0.f;

  T process(T value) {
    T output = (value >= 1.f) & (lastValue < 1.f);
    lastValue = value;
    return output;
  }
};

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

  void process(T newPhaseIn) {
    T newPhaseInMod = fmod(newPhaseIn, 1.f);
    T phaseInDelta = newPhaseInMod - lastPhaseIn;
    T signMask = sgn(phaseInDelta) != sgn(lastPhaseInDelta);
    T bigDeltaMask = abs(phaseInDelta) > 0.1f;
    T phaseInWrapMask = signMask & bigDeltaMask;
    phaseInDelta = ifelse(phaseInWrapMask, lastPhaseInDelta, phaseInDelta);
    T multipliedDelta = phaseInDelta * multiplier;
    phase = phase + multipliedDelta;
    phase = eucMod(phase, 1.f);
    phase = ifelse(phase < 0.f, 1.f - phase, phase);
    phase10 = phase * 10.f;

    lastPhaseInDelta = phaseInDelta;
    lastPhaseIn = newPhaseInMod;
  }
};
