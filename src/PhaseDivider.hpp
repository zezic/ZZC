#include "ZZC.hpp"

struct PhaseDivider {
  float ratio = 1.f;
  float requestedRatio = 1.f;
  bool ratioIsRequested = false;
  bool hardSyncIsRequested = false;
  float lastPhaseIn = 0.f;
  float lastPhaseInDelta = 0.f;
  double phase = 0.0;
  double lastPhase = 0.0;

  void reset() {
    this->phase = 0.0;
    this->lastPhase = 0.0;
    this->lastPhaseIn = 0.f;
    this->lastPhaseInDelta = 0.f;
  }

  void requestRatio(float newRatio) {
    if (newRatio == this->ratio) {
      this->ratioIsRequested = false;
      return;
    }
    this->requestedRatio = newRatio;
    this->ratioIsRequested = true;
  }

  void requestHardSync() {
    this->hardSyncIsRequested = true;
  }

  void setRatio(float newRatio) {
    this->ratio = newRatio;
    this->ratioIsRequested = false;
  }

  bool process(float phaseIn) {
    phaseIn = std::fmod(phaseIn, 10.f);
    float phaseInDelta = phaseIn - this->lastPhaseIn;
    bool masterFlipped = (std::abs(phaseInDelta) > 0.1f) && (math::sgn(phaseInDelta) != math::sgn(this->lastPhaseInDelta));
    if (masterFlipped) {
      phaseInDelta = lastPhaseInDelta;
    }
    this->lastPhaseInDelta = phaseInDelta;
    double newPhase = this->phase + phaseInDelta * this->ratio;
    while (newPhase >= 10.0) {
      newPhase -= 10.0;
    }
    while (newPhase < 0.0) {
      newPhase += 10.0;
    }
    bool slaveFlipped = std::abs(this->lastPhase - newPhase) > 9.0;
    if ((this->ratioIsRequested || this->hardSyncIsRequested) &&
        ((masterFlipped && (this->ratio >= 1.f)) ||
         (slaveFlipped && (this->ratio < 1.f)))) {
      if (this->ratioIsRequested) {
        this->ratio = this->requestedRatio;
      }
      this->ratioIsRequested = false;
      this->hardSyncIsRequested = false;
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
