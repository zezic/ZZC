#include "ZZC.hpp"
#include "DivCore.hpp"

void DivBase::process(float phaseIn, float sampleTime) {
  monoDivCore.ratio = roundedMultiplier;

  bool flipped = monoDivCore.process(phaseIn);

  if (this->gateMode) {
    clockOutput = monoDivCore.phase < 5.0 ? 10.f : 0.f;
  } else {
    if (flipped) {
      pulseGenerator.trigger(1e-3f);
    }
    clockOutput = pulseGenerator.process(sampleTime) ? 10.f : 0.f;
  }
  phaseOutput = monoDivCore.phase;
}

void DivBase::handleFractionParam(float value) {
  if (value == lastFractionParam) { return; }
  float fractionParam = trunc(value);
  float fractionAbs = std::max(1.f, abs(fractionParam));
  this->paramMultiplier = fractionParam >= 0.f ? fractionAbs : 1.f / fractionAbs;
  this->lastFractionParam = value;
  this->combinedMultiplierDirty = true;
}

void DivBase::handleCV(float cvVoltage) {
  if (cvVoltage == lastCVVoltage) { return; }
  this->cvMultiplier = dsp::approxExp2_taylor5(cvVoltage + 20) / 1048576;
  this->lastCVVoltage = cvVoltage;
  this->combinedMultiplierDirty = true;
}

void DivBase::combineMultipliers() {
  if (!this->combinedMultiplierDirty) { return; }
  this->combinedMultiplier = paramMultiplier * cvMultiplier;

  float combinedMultiplierLo = 1.f / roundf(1.f / this->combinedMultiplier);
  float combinedMultiplierHi = roundf(this->combinedMultiplier);

  this->roundedMultiplier = clamp(this->combinedMultiplier < 1.f ? combinedMultiplierLo : combinedMultiplierHi, 0.f, 199.f);
  this->combinedMultiplierDirty = false;

  // Manage fraction display
  if (this->roundedMultiplier == 1.f) {
    this->fractionDisplay = 1;
    this->fractionDisplayPolarity = 0;
  } else if (this->roundedMultiplier > 1.f) {
    this->fractionDisplay = roundf(this->roundedMultiplier);
    this->fractionDisplayPolarity = 1;
  } else {
    this->fractionDisplay = roundf(clamp(1.f / this->roundedMultiplier, 1.f, 199.f));
    this->fractionDisplayPolarity = -1;
  }
}
