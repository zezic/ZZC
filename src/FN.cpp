#include "ZZC.hpp"
#include "window.hpp"

using simd::float_4;

const float M_PI_X2 = M_PI * 2.0f;

template <typename T>
inline T fn3Sin(T phase) {
  return (simd::sin(phase * M_PI_X2 - M_PI_2) + 1.f) * 0.5f;
}

template <typename T>
inline T fn3Sqr(T phase) {
  T high = (phase < 0.5f);
  return simd::ifelse(high, 1.f, 0.0f);
}

template <typename T>
inline T fn3Tri(T phase) {
  return 2.f * simd::fabs(phase - simd::round(phase));
}

template <typename T>
inline T applyPW(T phase, T pw) {
  return simd::ifelse(
    pw == 0.f,
    0.5f + phase / 2.f,
    simd::ifelse(
      phase > pw,
      (phase - pw) / (1.0f - pw) / 2.0f + 0.5f,
      phase / pw / 2.0f
    )
  );
}

struct FN3TextDisplayWidget : TransparentWidget {
  float *hook = nullptr;
  float prevHook = 0.0f;
  float *text = nullptr;
  bool displayText = false;
  double textUpdatedAt = 0.0;

  std::shared_ptr<Font> font;

  FN3TextDisplayWidget() {
    font = APP->window->loadFont(asset::plugin(pluginInstance, "res/fonts/Nunito/Nunito-Black.ttf"));
  };

  void draw(const DrawArgs &args) override {
    NVGcolor lcdColor = nvgRGB(0x12, 0x12, 0x12);
    NVGcolor lcdTextColor = nvgRGB(0xff, 0xd4, 0x2a);

    if (hook && prevHook == *hook) {
      double curTime = glfwGetTime();
      if ((curTime - textUpdatedAt) > 2.0) {
        return;
      }
    } else {
      textUpdatedAt = glfwGetTime();
      prevHook = hook ? *hook : 0.0f;
    }

    // LCD
    nvgBeginPath(args.vg);
    nvgRoundedRect(args.vg, 0.0, 0.0, box.size.x, box.size.y, 0.0);
    nvgFillColor(args.vg, lcdColor);
    nvgFill(args.vg);

    char textString[10];
    if (text) {
      snprintf(textString, sizeof(textString), "%3.1f", *text > 0.04 ? *text - 0.04 : *text + 0.04);
    } else {
      snprintf(textString, sizeof(textString), "%3.1f", 0.00);
    }

    // Text
    nvgFontSize(args.vg, 8.5);
    nvgFontFaceId(args.vg, font->handle);
    nvgTextLetterSpacing(args.vg, 0.1);
    nvgTextAlign(args.vg, NVG_ALIGN_CENTER);
    Vec textPos = Vec(box.size.x / 2.0f, box.size.y * 0.7f);
    nvgFillColor(args.vg, lcdTextColor);
    nvgText(args.vg, textPos.x, textPos.y, textString, NULL);
  }

};

struct FN3DisplayWidget : BaseDisplayWidget {
  float *wave = nullptr;
  simd::float_4 *pw = nullptr;
  simd::float_4 *shift = nullptr;
  int *channels = nullptr;
  NVGcolor monoColor = nvgRGB(0xff, 0xd4, 0x2a);
  NVGcolor polyColor = nvgRGB(0x29, 0xb2, 0xef);

  void draw(const DrawArgs &args) override {
    float waveVal = wave ? *wave : 0.0f;
    int channelsVal = channels ? *channels : 1;
    drawBackground(args);

    NVGcolor graphColor = channelsVal == 1 ? monoColor : polyColor;

		for (int c = 0; c < channelsVal; c += 4) {
      simd::float_4 pwValSimd = pw ? pw[c / 4] : 0.5f;
      simd::float_4 shiftValSimd = shift ? shift[c / 4] : 0.f;
      int chunkSize = std::min(4, channelsVal - c);
      for (int subC = 0; subC < chunkSize; subC++) {
        float pwVal = pwValSimd[subC];
        float shiftVal = shiftValSimd[subC];
        nvgBeginPath(args.vg);
        float firstCoord = true;
        for (float i = 0.0f; i < 1.00f; i = i + 0.01f) {
          float x, y, value, phase;
          value = 0.0f;
          x = 2.0f + (box.size.x - 4.0f) * i;
          phase = applyPW(eucMod(i + shiftVal, 1.0f), pwVal);
          if (waveVal == 0.0f) {
            value = fn3Sin(phase);
          } else if (waveVal == 1.0f) {
            value = fn3Tri(phase);
          } else if (waveVal == 2.0f) {
            value = fn3Sqr(phase);
          }
          y = (1.0f - value) * (box.size.y / 4.0f) + (0.375f * box.size.y);

          if (firstCoord) {
            nvgMoveTo(args.vg, x, y);
            firstCoord = false;
            continue;
          }
          nvgLineTo(args.vg, x, y);
        }

        nvgStrokeColor(args.vg, graphColor);
        nvgLineCap(args.vg, NVG_ROUND);
        nvgMiterLimit(args.vg, 2.0f);
        nvgStrokeWidth(args.vg, 1.0f);
        nvgGlobalCompositeOperation(args.vg, NVG_LIGHTER);
        nvgStroke(args.vg);
      }
    }
  }
};


struct FN3 : Module {
  enum ParamIds {
    PW_PARAM,
    WAVE_PARAM,
    OFFSET_PARAM,
    SHIFT_PARAM,
    NUM_PARAMS
  };
  enum InputIds {
    PW_INPUT,
    SHIFT_INPUT,
    PHASE_INPUT,
    NUM_INPUTS
  };
  enum OutputIds {
    WAVE_OUTPUT,
    NUM_OUTPUTS
  };
  enum LightIds {
    NUM_LIGHTS
  };

  simd::float_4 phase[4] = {0.f};
  simd::float_4 pw[4] = {0.5f};
  simd::float_4 shift[4] = {0.f};
  simd::float_4 output[4] = {0.f};
  float wave = 0.f;
  int channels = 1;

  float pwParam = 0.5f;
  float lastPwParam = 0.5f;
  float pwDisplay = 50.0f;

  float shiftParam = 0.f;
  float lastShiftParam = 0.f;
  float shiftDisplay = 0.f;

  inline float snap(float value) {
    if (value > 0.33f && value < 0.34f) {
      return 1.0f / 3.0f;
    } else if (value > 0.66f && value < 0.67f) {
      return 1.0f / 1.5f;
    } else if (value < -0.33f && value > -0.34f) {
      return -1.0f / 3.0f;
    } else if (value < -0.66f && value > -0.67f) {
      return -1.0f / 1.5f;
    } else {
      return roundf(value * 100.0f) / 100.0f;
    }
  }

  FN3() {
    config(NUM_PARAMS, NUM_INPUTS, NUM_OUTPUTS, NUM_LIGHTS);
    configParam(PW_PARAM, 0.0f, 1.0f, 0.5f, "Pulse Width");
    configParam(WAVE_PARAM, 0.0f, 2.0f, 0.0f, "Waveform");
    configParam(OFFSET_PARAM, 0.0f, 1.0f, 0.0f, "Bipolar Mode");
    configParam(SHIFT_PARAM, 1.0f, -1.0f, 0.0f, "Phase Shift");
  }
  void process(const ProcessArgs &args) override;
};


void FN3::process(const ProcessArgs &args) {
  channels = std::max(1, inputs[PHASE_INPUT].getChannels());
  channels = std::max(channels, inputs[PW_INPUT].getChannels());
  channels = std::max(channels, inputs[SHIFT_INPUT].getChannels());

  if (params[PW_PARAM].getValue() != lastPwParam) {
    pwParam = snap(params[PW_PARAM].getValue());
    lastPwParam = params[PW_PARAM].getValue();
  }

  if (inputs[PW_INPUT].isConnected()) {
		for (int c = 0; c < channels; c += 4) {
      pw[c / 4] = inputs[PW_INPUT].getVoltageSimd<float_4>(c) / 10.f + pwParam;
      pw[c / 4] = clamp(pw[c / 4], 0.f, 1.f);
    }
  } else {
		for (int c = 0; c < channels; c += 4) {
      pw[c / 4] = pwParam;
      pw[c / 4] = clamp(pw[c / 4], 0.f, 1.f);
    }
  }
  pwDisplay = pwParam * 100.0f;

  if (params[SHIFT_PARAM].getValue() != lastShiftParam) {
    shiftParam = snap(params[SHIFT_PARAM].getValue());
    lastShiftParam = params[SHIFT_PARAM].getValue();
  }

  if (inputs[SHIFT_INPUT].isConnected()) {
		for (int c = 0; c < channels; c += 4) {
      shift[c / 4] = inputs[SHIFT_INPUT].getVoltageSimd<float_4>(c) / -5.f + shiftParam;
    }
  } else {
		for (int c = 0; c < channels; c += 4) {
      shift[c / 4] = shiftParam;
    }
  }
  shiftDisplay = shiftParam * -100.0f;

  if (inputs[PHASE_INPUT].isConnected()) {
		for (int c = 0; c < channels; c += 4) {
      simd::float_4 newPhase = inputs[PHASE_INPUT].getVoltageSimd<float_4>(c) / 10.f + shift[c / 4];
      newPhase -= simd::floor(newPhase); // wrap
      phase[c / 4] = applyPW(newPhase, pw[c / 4]);
    }
  } else {
		for (int c = 0; c < channels; c += 4) {
      phase[c / 4] = shift[c / 4];
      phase[c / 4] -= simd::floor(phase[c / 4]); // wrap
      phase[c / 4] = applyPW(phase[c / 4], pw[c / 4]);
    }
  }

  wave = params[WAVE_PARAM].getValue();

  if (wave == 0.0f) {
    // outputs[WAVE_OUTPUT].setVoltage(fn3Sin(phase) * 10.0f - (params[OFFSET_PARAM].getValue() == 1.0f ? 5.0f : 0.0f));
		for (int c = 0; c < channels; c += 4) {
      output[c / 4] = fn3Sin(phase[c / 4]) * 10.f;
    }
  } else if (wave == 1.0f) {
    // outputs[WAVE_OUTPUT].setVoltage(fn3Tri(phase) * 10.0f - (params[OFFSET_PARAM].getValue() == 1.0f ? 5.0f : 0.0f));
		for (int c = 0; c < channels; c += 4) {
      output[c / 4] = fn3Tri(phase[c / 4]) * 10.f;
    }
  } else {
    // outputs[WAVE_OUTPUT].setVoltage(fn3Sqr(phase) * 10.0f - (params[OFFSET_PARAM].getValue() == 1.0f ? 5.0f : 0.0f));
		for (int c = 0; c < channels; c += 4) {
      output[c / 4] = fn3Sqr(phase[c / 4]) * 10.f;
    }
  }
  if (params[OFFSET_PARAM].getValue() == 1.f) {
		for (int c = 0; c < channels; c += 4) {
      output[c / 4] -= 5.f;
    }
  }
  for (int c = 0; c < channels; c += 4) {
    outputs[WAVE_OUTPUT].setVoltageSimd(output[c / 4], c);
  }
  outputs[WAVE_OUTPUT].setChannels(channels);
}


struct FN3Widget : ModuleWidget {
  FN3Widget(FN3 *module) {
    setModule(module);
    setPanel(APP->window->loadSvg(asset::plugin(pluginInstance, "res/panels/FN-3.svg")));

    addParam(createParam<ZZC_Knob27>(Vec(9, 58), module, FN3::PW_PARAM));
    addInput(createInput<ZZC_PJ_Port>(Vec(10.5, 93), module, FN3::PW_INPUT));

    FN3DisplayWidget *display = new FN3DisplayWidget();
    display->box.pos = Vec(8.0f, 126.0f);
    display->box.size = Vec(29.0f, 49.0f);
    if (module) {
      display->wave = &module->wave;
      display->pw = module->pw;
      display->shift = module->shift;
      display->channels = &module->channels;
    }
    addChild(display);
    addParam(createParam<ZZC_FN3WaveSwitch>(Vec(8, 126), module, FN3::WAVE_PARAM));
    addParam(createParam<ZZC_FN3UniBiSwitch>(Vec(8, 152), module, FN3::OFFSET_PARAM));

    FN3TextDisplayWidget *pwDisplay = new FN3TextDisplayWidget();
    pwDisplay->box.pos = Vec(11.0f, 129.0f);
    pwDisplay->box.size = Vec(23.0f, 13.0f);
    if (module) {
      pwDisplay->hook = &module->pwParam;
      pwDisplay->text = &module->pwDisplay;
    }
    addChild(pwDisplay);

    FN3TextDisplayWidget *shiftDisplay = new FN3TextDisplayWidget();
    shiftDisplay->box.pos = Vec(11.0f, 129.0f);
    shiftDisplay->box.size = Vec(23.0f, 13.0f);
    if (module) {
      shiftDisplay->hook = &module->shiftParam;
      shiftDisplay->text = &module->shiftDisplay;
    }
    addChild(shiftDisplay);

    addInput(createInput<ZZC_PJ_Port>(Vec(10.5, 194), module, FN3::SHIFT_INPUT));
    addParam(createParam<ZZC_Knob25>(Vec(10, 229), module, FN3::SHIFT_PARAM));

    addInput(createInput<ZZC_PJ_Port>(Vec(10.5, 275), module, FN3::PHASE_INPUT));
    addOutput(createOutput<ZZC_PJ_Port>(Vec(10.5, 320), module, FN3::WAVE_OUTPUT));

    addChild(createWidget<ZZC_Screw>(Vec(box.size.x / 2 - RACK_GRID_WIDTH / 2, 0)));
    addChild(createWidget<ZZC_Screw>(Vec(box.size.x / 2 - RACK_GRID_WIDTH / 2, RACK_GRID_HEIGHT - RACK_GRID_WIDTH)));
  }
};


Model *modelFN3 = createModel<FN3, FN3Widget>("FN-3");
