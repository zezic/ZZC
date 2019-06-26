#include "ZZC.hpp"


template <typename T>
static T softClip(T x) {
  x = simd::clamp(x, -3.f, 3.f);
  return x * (27 + x * x) / (27 + 9 * x * x);
}


template <typename T>
static T softClipTo(T input, T ceiling, T softness) {
  T clean_range;
  clean_range += 1.f;
  clean_range -= softness;
  clean_range *= ceiling;

  T ws_range = ceiling * softness;

  T clean_part = simd::clamp(input, -clean_range, clean_range);

  T ws_part = input;
  ws_part -= clean_part;
  ws_part /= ws_range;
  ws_part = softClip(ws_part);
  ws_part *= ws_range;

  return clean_part + ws_part;
}


struct SCVCA : Module {
  enum ParamIds {
    GAIN_PARAM,
    CLIP_PARAM,
    CLIP_SOFTNESS_PARAM,
    NUM_PARAMS
  };
  enum InputIds {
    CV_INPUT,
    SIG1_INPUT,
    SIG2_INPUT,
    GAIN_INPUT,
    CLIP_INPUT,
    CLIP_SOFTNESS_INPUT,
    NUM_INPUTS
  };
  enum OutputIds {
    SIG1_OUTPUT,
    SIG2_OUTPUT,
    NUM_OUTPUTS
  };
  enum LightIds {
    CLIPPING_POS_LIGHT,
    CLIPPING_NEG_LIGHT,
    NUM_LIGHTS
  };

  void processChannel(
    Input &in,
    Param &gainParam, Param &softnessParam, Param &clipParam,
    Input &gainInput, Input &softnessInput, Input &clipInput,
    Output &out
  ) {
    // Get input
    int channels = in.getChannels();
    simd::float_4 v[4];
    for (int c = 0; c < channels; c += 4) {
      v[c / 4] = simd::float_4::load(in.getVoltages(c));
    }

    // Apply knob gain
    float gain = gainParam.getValue();
    for (int c = 0; c < channels; c += 4) {
      v[c / 4] *= gain;
    }

    // Apply CV gain
    if (gainInput.isConnected()) {
      if (gainInput.isPolyphonic()) {
        for (int c = 0; c < channels; c += 4) {
          simd::float_4 cv = simd::float_4::load(gainInput.getVoltages(c)) / 10.f;
          cv = clamp(cv, 0.f, 1.f);
          v[c / 4] *= cv;
        }
      } else {
        float cv = gainInput.getVoltage() / 10.f;
        cv = clamp(cv, 0.f, 1.f);
        for (int c = 0; c < channels; c += 4) {
          v[c / 4] *= cv;
        }
      }
    }

    // Load clip level CV
    simd::float_4 clipValue[4];
    if (clipInput.isConnected()) {
      if (clipInput.isPolyphonic()) {
        for (int c = 0; c < channels; c += 4) {
          simd::float_4 clipValue = simd::float_4::load(clipInput.getVoltages(c)) / 10.f;
          clipValue = clamp(clipValue, 0.f, 1.f);
        }
      } else {
        for (int c = 0; c < channels; c += 4) {
          clipValue[c / 4] += clipParam.getValue() * clamp(clipInput.getVoltage() / 10.f, 0.f, 1.f);
        }
      }
    } else {
      for (int c = 0; c < channels; c += 4) {
        clipValue[c / 4] += clipParam.getValue();
      }
    }

    // Load clip softness CV
    simd::float_4 softnessValue[4];
    if (softnessInput.isConnected()) {
      if (softnessInput.isPolyphonic()) {
        for (int c = 0; c < channels; c += 4) {
          simd::float_4 softnessValue = simd::float_4::load(softnessInput.getVoltages(c)) / 10.f;
          softnessValue = clamp(softnessValue, 0.f, 1.f);
        }
      } else {
        for (int c = 0; c < channels; c += 4) {
          softnessValue[c / 4] += softnessParam.getValue() * clamp(softnessInput.getVoltage() / 10.f, 0.f, 1.f);
        }
      }
    } else {
      for (int c = 0; c < channels; c += 4) {
        softnessValue[c / 4] += softnessParam.getValue();
      }
    }

    // Apply softclipping
    for (int c = 0; c < channels; c += 4) {
      v[c / 4] = softClipTo(v[c / 4], clipValue[c / 4], softnessValue[c / 4]);
    }

    // Set output
    out.setChannels(channels);
    for (int c = 0; c < channels; c += 4) {
      v[c / 4].store(out.getVoltages(c));
    }
  }

  SCVCA() {
    config(NUM_PARAMS, NUM_INPUTS, NUM_OUTPUTS, NUM_LIGHTS);
    configParam(GAIN_PARAM, 0.0f, 2.0f, 1.0f);
    configParam(CLIP_PARAM, 0.0f, 10.0f, 5.0f);
    configParam(CLIP_SOFTNESS_PARAM, 0.0f, 1.0f, 0.5f);
  }
  void process(const ProcessArgs &args) override;
};


void SCVCA::process(const ProcessArgs &args) {
  processChannel(
    inputs[SIG1_INPUT],
    params[GAIN_PARAM], params[CLIP_SOFTNESS_PARAM], params[CLIP_PARAM],
    inputs[GAIN_INPUT], inputs[CLIP_SOFTNESS_INPUT], inputs[CLIP_INPUT],
    outputs[SIG1_OUTPUT]
  );
  processChannel(
    inputs[SIG2_INPUT],
    params[GAIN_PARAM], params[CLIP_SOFTNESS_PARAM], params[CLIP_PARAM],
    inputs[GAIN_INPUT], inputs[CLIP_SOFTNESS_INPUT], inputs[CLIP_INPUT],
    outputs[SIG2_OUTPUT]
  );
  /*
  lights[CLIPPING_POS_LIGHT].setSmoothBrightness(fmaxf(
    fminf(1.0f, gained_1 - outputs[SIG1_OUTPUT].value),
    fminf(1.0f, gained_2 - outputs[SIG2_OUTPUT].value)
  ), args.sampleTime);
  lights[CLIPPING_NEG_LIGHT].setSmoothBrightness(fmaxf(
    fmaxf(-1.0f, -(gained_1 - outputs[SIG1_OUTPUT].value)),
    fmaxf(-1.0f, -(gained_2 - outputs[SIG2_OUTPUT].value))
  ), args.sampleTime);
  */
}


struct SCVCAWidget : ModuleWidget {
  SCVCAWidget(SCVCA *module) {
    setModule(module);
    setPanel(APP->window->loadSvg(asset::plugin(pluginInstance, "res/panels/SC-VCA.svg")));

    addParam(createParam<ZZC_BigKnob>(Vec(4, 74.7), module, SCVCA::GAIN_PARAM));
    addParam(createParam<ZZC_BigKnobInner>(Vec(24, 94.7), module, SCVCA::CLIP_PARAM));
    addParam(createParam<ZZC_Knob25>(Vec(42.5, 175.7), module, SCVCA::CLIP_SOFTNESS_PARAM));

    addInput(createInput<ZZC_PJ_Port>(Vec(8, 221), module, SCVCA::GAIN_INPUT));
    addInput(createInput<ZZC_PJ_Port>(Vec(42.5, 221), module, SCVCA::CLIP_INPUT));
    addInput(createInput<ZZC_PJ_Port>(Vec(8, 176), module, SCVCA::CLIP_SOFTNESS_INPUT));

    addInput(createInput<ZZC_PJ_Port>(Vec(8, 275), module, SCVCA::SIG1_INPUT));
    addInput(createInput<ZZC_PJ_Port>(Vec(42.5, 275), module, SCVCA::SIG2_INPUT));
    addOutput(createOutput<ZZC_PJ_Port>(Vec(8, 319.75), module, SCVCA::SIG1_OUTPUT));
    addOutput(createOutput<ZZC_PJ_Port>(Vec(42.5, 319.75), module, SCVCA::SIG2_OUTPUT));

    addChild(createLight<SmallLight<GreenRedLight>>(Vec(34.2f, 43.9f), module, SCVCA::CLIPPING_POS_LIGHT));

    addChild(createWidget<ZZC_Screw>(Vec(RACK_GRID_WIDTH, 0)));
    addChild(createWidget<ZZC_Screw>(Vec(box.size.x - 2 * RACK_GRID_WIDTH, 0)));
    addChild(createWidget<ZZC_Screw>(Vec(RACK_GRID_WIDTH, RACK_GRID_HEIGHT - RACK_GRID_WIDTH)));
    addChild(createWidget<ZZC_Screw>(Vec(box.size.x - 2 * RACK_GRID_WIDTH, RACK_GRID_HEIGHT - RACK_GRID_WIDTH)));
  }
};


Model *modelSCVCA = createModel<SCVCA, SCVCAWidget>("SC-VCA");
