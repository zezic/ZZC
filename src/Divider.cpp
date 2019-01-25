#include "ZZC.hpp"
#include "shared.hpp"
#include "widgets.hpp"


struct Divider : Module {
  enum ParamIds {
    IN_RATIO_PARAM,
    OUT_RATIO_PARAM,
    SWING_PARAM,
    NUM_PARAMS
  };
  enum InputIds {
    IN_RATIO_INPUT,
    OUT_RATIO_INPUT,
    SWING_INPUT,
    PHASE_INPUT,
    VBPS_INPUT,
    RESET_INPUT,
    NUM_INPUTS
  };
  enum OutputIds {
    CLOCK_OUTPUT,
    PHASE_OUTPUT,
    NUM_OUTPUTS
  };
  enum LightIds {
    EXT_PHASE_MODE_LED,
    EXT_VBPS_MODE_LED,
    NUM_LIGHTS
  };

  LowFrequencyOscillator oscillator;

  float from = 1.0f;
  float to = 1.0f;
  float ratio = 1.0f;
  float swing = 50.0f;

  float phaseIn = 0.0f;
  float lastPhaseIn = 0.0f;
  float lastPhaseInDelta = 0.0f;
  bool lastPhaseInState = false;

  double halfPhaseOut = 0.0;
  double lastHalfPhaseOut = 0.0;
  float phaseOut = 0.0f;

  PulseGenerator clockPulseGenerator;
  PulseGenerator resetPulseGenerator;
  bool clockPulse = false;
  bool resetPulse = false;

  SchmittTrigger clockTrigger;
  SchmittTrigger resetTrigger;

  float logMap(float input, float range) {
    if (input == 0.0f) {
      return 1.0f;
    } 
    return max(1.0f, powf(2, log2f(input * range)));
  }

  void processRatioInputs() {
    if (inputs[IN_RATIO_INPUT].active) {
      from = floorf(logMap(clamp(inputs[IN_RATIO_INPUT].value, 0.0f, 10.0f) / 10.0f, params[IN_RATIO_PARAM].value) + 0.01);
    } else {
      from = params[IN_RATIO_PARAM].value;
    }
    if (inputs[OUT_RATIO_INPUT].active) {
      to = floorf(logMap(clamp(inputs[OUT_RATIO_INPUT].value, 0.0f, 10.0f) / 10.0f, params[OUT_RATIO_PARAM].value) + 0.01);
    } else {
      to = params[OUT_RATIO_PARAM].value;
    }
    ratio = to / from;
  }

  void processSwingInput() {
    swing = params[SWING_PARAM].value;

    if (inputs[SWING_INPUT].active) {
      float swingInput = clamp(inputs[SWING_INPUT].value / 5.0f, -1.0f, 1.0f);
      if (swingInput < 0.0f) {
        swing += (swing - 1.0f) * swingInput;
      } else if (swingInput > 0.0f) {
        swing += (99.0f - swing) * swingInput;
      }
    }
  }

  Divider() : Module(NUM_PARAMS, NUM_INPUTS, NUM_OUTPUTS, NUM_LIGHTS) {
  }
  void step() override;
};


void Divider::step() {
  processRatioInputs();
  processSwingInput();

  if (resetTrigger.process(inputs[RESET_INPUT].value)) {
    phaseOut = 0.0f;
    halfPhaseOut = 0.0;
    lastHalfPhaseOut = 0.0f;
    clockPulseGenerator.trigger(1e-3f);
  } else if (inputs[PHASE_INPUT].active) {
    if (lastPhaseInState) {
      phaseIn = inputs[PHASE_INPUT].value;
      float phaseInDelta = phaseIn - lastPhaseIn;
      if (fabsf(phaseInDelta) > 0.1f && (sgn(phaseInDelta) != sgn(lastPhaseInDelta))) {
        phaseInDelta = lastPhaseInDelta;
      }
      lastPhaseInDelta = phaseInDelta;
      halfPhaseOut += phaseInDelta * ratio * 0.5f;
    }
  } else if (inputs[VBPS_INPUT].active) {
    float del = inputs[VBPS_INPUT].value * engineGetSampleTime() * 10.0f * ratio * 0.5f;
    halfPhaseOut += del;
  }

  // halfPhaseOut = eucmod(halfPhaseOut, 10.0f);
  while (halfPhaseOut >= 10.0) {
    halfPhaseOut = halfPhaseOut - 10.0;
  }
  while (halfPhaseOut < 0.0) {
    halfPhaseOut = halfPhaseOut + 10.0;
  }

  // Swing resulting phase
  float swingTresh = swing / 10.0f;
  if (halfPhaseOut < swingTresh) {
    phaseOut = halfPhaseOut / swingTresh * 10.0f;
  } else {
    float swingRem = 10.0f - swingTresh;
    float phaseGoes = halfPhaseOut - swingTresh;
    phaseOut = phaseGoes / swingRem * 10.0f;
  }

  // Trigger swinged beat
  if ((lastHalfPhaseOut < swingTresh && swingTresh <= halfPhaseOut) ||
      (lastHalfPhaseOut > swingTresh && swingTresh >= halfPhaseOut)) {
    clockPulseGenerator.trigger(1e-3f);
  }

  lastHalfPhaseOut = halfPhaseOut;

  lastPhaseIn = inputs[PHASE_INPUT].value;
  lastPhaseInState = inputs[PHASE_INPUT].active;

  clockPulse = clockPulseGenerator.process(engineGetSampleTime());

  outputs[PHASE_OUTPUT].value = phaseOut;
  outputs[CLOCK_OUTPUT].value = clockPulse ? 10.0f : 0.0f;

  lights[EXT_PHASE_MODE_LED].value = inputs[PHASE_INPUT].active ? 0.5f : 0.0f;
  lights[EXT_VBPS_MODE_LED].value = !inputs[PHASE_INPUT].active && inputs[VBPS_INPUT].active ? 0.5f : 0.0f;
}

struct DividerWidget : ModuleWidget {
  DividerWidget(Divider *module) : ModuleWidget(module) {
    setPanel(SVG::load(assetPlugin(plugin, "res/Divider.svg")));

    RatioDisplayWidget *ratioDisplay = new RatioDisplayWidget();
    ratioDisplay->box.pos = Vec(9.0f, 60.0f);
    ratioDisplay->box.size = Vec(57.0f, 21.0f);
    ratioDisplay->from = &module->from;
    ratioDisplay->to = &module->to;
    addChild(ratioDisplay);

		addParam(ParamWidget::create<ZZC_ToothSnapKnob>(Vec(8, 91), module, Divider::IN_RATIO_PARAM, 1.0f, 99.0f, 1.0f));
		addParam(ParamWidget::create<ZZC_ToothSnapKnob>(Vec(42, 91), module, Divider::OUT_RATIO_PARAM, 1.0f, 99.0f, 1.0f));
    addInput(Port::create<ZZC_PJ301MPort>(Vec(8, 126), Port::INPUT, module, Divider::IN_RATIO_INPUT));
    addInput(Port::create<ZZC_PJ301MPort>(Vec(42, 126), Port::INPUT, module, Divider::OUT_RATIO_INPUT));

    DisplayIntpartWidget *swingDisplay = new DisplayIntpartWidget();
    swingDisplay->box.pos = Vec(7.0f, 178.0f);
    swingDisplay->box.size = Vec(29.0f, 21.0f);
    swingDisplay->value = &module->swing;
    addChild(swingDisplay);

		addParam(ParamWidget::create<ZZC_ToothKnob>(Vec(42, 176), module, Divider::SWING_PARAM, 1.0f, 99.0f, 50.0f));

    addInput(Port::create<ZZC_PJ301MPort>(Vec(8, 233), Port::INPUT, module, Divider::SWING_INPUT));
    addInput(Port::create<ZZC_PJ301MPort>(Vec(42, 233), Port::INPUT, module, Divider::PHASE_INPUT));
    addChild(ModuleLightWidget::create<TinyLight<GreenLight>>(Vec(64, 233), module, Divider::EXT_PHASE_MODE_LED));
    addInput(Port::create<ZZC_PJ301MPort>(Vec(8, 275), Port::INPUT, module, Divider::VBPS_INPUT));
    addChild(ModuleLightWidget::create<TinyLight<GreenLight>>(Vec(30, 275), module, Divider::EXT_VBPS_MODE_LED));
    addInput(Port::create<ZZC_PJ301MPort>(Vec(42, 275), Port::INPUT, module, Divider::RESET_INPUT));
    addOutput(Port::create<ZZC_PJ301MIPort>(Vec(8, 319), Port::OUTPUT, module, Divider::CLOCK_OUTPUT));
    addOutput(Port::create<ZZC_PJ301MIPort>(Vec(42, 319), Port::OUTPUT, module, Divider::PHASE_OUTPUT));

		addChild(Widget::create<ScrewBlack>(Vec(RACK_GRID_WIDTH, 0)));
		addChild(Widget::create<ScrewBlack>(Vec(box.size.x - 2 * RACK_GRID_WIDTH, 0)));
		addChild(Widget::create<ScrewBlack>(Vec(RACK_GRID_WIDTH, RACK_GRID_HEIGHT - RACK_GRID_WIDTH)));
		addChild(Widget::create<ScrewBlack>(Vec(box.size.x - 2 * RACK_GRID_WIDTH, RACK_GRID_HEIGHT - RACK_GRID_WIDTH)));
  }
};


Model *modelDivider = Model::create<Divider, DividerWidget>("ZZC", "Divider", "Divider", CLOCK_MODULATOR_TAG);