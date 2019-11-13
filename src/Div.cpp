#include "ZZC.hpp"
#include "DivCore.hpp"

struct Div : Module {
  enum ParamIds {
    FRACTION_PARAM,
    NUM_PARAMS
  };
  enum InputIds {
    CV_INPUT,
    PHASE_INPUT,
    RESET_INPUT,
    NUM_INPUTS
  };
  enum OutputIds {
    CLOCK_OUTPUT,
    PHASE_OUTPUT,
    NUM_OUTPUTS
  };
  enum LightIds {
    NUM_LIGHTS
  };

  DivCore<float_4> divCore[4];

  int fractionDisplay = 1;
  int fractionDisplayPolarity = 0;
  bool hasPolyMultiplier = false;

  Div() {
    config(NUM_PARAMS, NUM_INPUTS, NUM_OUTPUTS, NUM_LIGHTS);
    configParam(FRACTION_PARAM, -199.0f, 199.0f, 0.0f, "Fraction");
  }

  void process(const ProcessArgs &args) override;
};

simd::float_4 maskBase = { 0.f, 1.f, 2.f, 3.f };

void Div::process(const ProcessArgs &args) {
  float fractionParam = std::trunc(params[FRACTION_PARAM].getValue());
  float fractionAbs = std::max(1.f, abs(fractionParam));

  int phaseChannels = inputs[PHASE_INPUT].getChannels();
  int cvChannels = inputs[CV_INPUT].getChannels();
  int channels = std::max(phaseChannels, cvChannels);
  outputs[PHASE_OUTPUT].setChannels(channels);


  float paramMultiplier = fractionParam >= 0.f ? fractionAbs : 1.f / fractionAbs;

  simd::float_4 dummyPhaseInValue = phaseChannels > 0 ? inputs[PHASE_INPUT].getVoltage(phaseChannels - 1) * 0.1f : 0.f;
  simd::float_4 dummyCVInValue = cvChannels > 0 ? inputs[CV_INPUT].getVoltage(cvChannels - 1) : 0.f;

  if (phaseChannels > 0) {
    for (int c = 0; c < channels; c += 4) {
      int blockIdx = c / 4;
      auto* core = &divCore[blockIdx];
      simd::float_4 combinedMultiplier = paramMultiplier;
      if (cvChannels > 0) {
        float realCVInputsForBlock = c < cvChannels ? cvChannels - c : 0.f;
        simd::float_4 cvVoltage = simd::ifelse(
          maskBase < realCVInputsForBlock,
          simd::float_4::load(inputs[CV_INPUT].getVoltages(c)),
          dummyCVInValue
        );

        // Not sure how to make it output 2 when cvVoltage is 1
        simd::float_4 cvMultiplier = dsp::approxExp2_taylor5(cvVoltage + 0.001f);
        // simd::float_4 cvMultiplier = simd::pow(2.f, cvVoltage);
        combinedMultiplier *= cvMultiplier;
      }
      simd::float_4 combinedMultiplierLo = 1.f / simd::floor(1.f / combinedMultiplier);
      simd::float_4 combinedMultiplierHi = simd::floor(combinedMultiplier);

      simd::float_4 roundedMultiplier = simd::ifelse(
        combinedMultiplier < 1.f,
        combinedMultiplierLo,
        combinedMultiplierHi
      );
      core->multiplier = clamp(roundedMultiplier, 0.f, 199.f);

      float realPhaseInputsForBlock = c < phaseChannels ? phaseChannels - c : 0.f;
      simd::float_4 phaseInValue = simd::ifelse(
        maskBase < realPhaseInputsForBlock,
        simd::float_4::load(inputs[PHASE_INPUT].getVoltages(c)) * 0.1f,
        dummyPhaseInValue
      );
      core->process(phaseInValue);
      core->phase10.store(outputs[PHASE_OUTPUT].getVoltages(c));
    }
  }

  // Manage fraction display
  hasPolyMultiplier = cvChannels > 1;
  if (hasPolyMultiplier) {
    fractionDisplay = fractionAbs;
  } else {
    float multiplier = divCore[0].multiplier[0];
    if (multiplier == 1.f) {
      fractionDisplay = 1;
      fractionDisplayPolarity = 0;
    } else if (multiplier > 1.f) {
      fractionDisplay = clamp(multiplier, 1.f, 199.f);
      fractionDisplayPolarity = 1;
    } else {
      fractionDisplay = clamp(1.f / multiplier, 1.f, 199.f);
      fractionDisplayPolarity = -1;
    }
  }

}

struct DivWidget : ModuleWidget {
  DivWidget(Div *module);
};

DivWidget::DivWidget(Div *module) {
  setModule(module);
  setPanel(APP->window->loadSvg(asset::plugin(pluginInstance, "res/panels/Div.svg")));

  addParam(createParam<ZZC_Knob27Blind>(Vec(9, 58), module, Div::FRACTION_PARAM));

  IntDisplayWidget *display = new IntDisplayWidget();

  display->box.pos = Vec(6, 94);
  display->box.size = Vec(33, 21);
  display->textGhost = "188";
  if (module) {
    display->value = &module->fractionDisplay;
    display->isPoly = &module->hasPolyMultiplier;
    display->polarity = &module->fractionDisplayPolarity;
  }
  addChild(display);

  addInput(createInput<ZZC_PJ_Port>(Vec(10.5, 124), module, Div::CV_INPUT));
  addInput(createInput<ZZC_PJ_Port>(Vec(10.5, 181), module, Div::PHASE_INPUT));
  addInput(createInput<ZZC_PJ_Port>(Vec(10.5, 224), module, Div::RESET_INPUT));

  addOutput(createOutput<ZZC_PJ_Port>(Vec(10.5, 272), module, Div::CLOCK_OUTPUT));
  addOutput(createOutput<ZZC_PJ_Port>(Vec(10.5, 320), module, Div::PHASE_OUTPUT));

  addChild(createWidget<ZZC_Screw>(Vec(RACK_GRID_WIDTH, 0)));
  addChild(createWidget<ZZC_Screw>(Vec(RACK_GRID_WIDTH, RACK_GRID_HEIGHT - RACK_GRID_WIDTH)));
}

Model *modelDiv = createModel<Div, DivWidget>("Div");
