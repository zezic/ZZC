#include "ZZC.hpp"
#include "window.hpp"

const float M_PI_X2 = M_PI * 2.0f;

inline float fn3Sin(float phase) {
  return (sinf(phase * M_PI_X2 - M_PI_2) + 1.0f) * 0.5f;
}

inline float fn3Sqr(float phase) {
  return phase < 0.5f ? 1.0f : 0.0f;
}

inline float fn3Tri(float phase) {
  return phase < 0.5f ? phase * 2.0f : (1.0f - phase) * 2.0f;
}

inline float applyPW(float phase, float pw) {
  if (pw == 0.0f) {
    return 0.5f + phase / 2.0f;
  }
  if (phase > pw) {
    return (phase - pw) / (1.0f - pw) / 2.0f + 0.5f;
  } else {
    return phase / pw / 2.0f;
  }
}

struct FN3TextDisplayWidget : TransparentWidget {
  float *hook = nullptr;
  float prevHook = 0.0f;
  float *text = nullptr;
  bool displayText = false;
  double textUpdatedAt = 0.0;

  std::shared_ptr<Font> font;

  FN3TextDisplayWidget() {
    font = Font::load(assetPlugin(pluginInstance, "res/fonts/Nunito/Nunito-Black.ttf"));
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
  float *pw = nullptr;
  float *shift = nullptr;

  void draw(const DrawArgs &args) override {
    float waveVal = wave ? *wave : 0.0f;
    float pwVal = pw ? *pw : 0.5f;
    float shiftVal = shift ? *shift : 0.0f;
    drawBackground(args);

    NVGcolor graphColor = nvgRGB(0xff, 0xd4, 0x2a);

    nvgBeginPath(args.vg);
    float firstCoord = true;
    for (float i = 0.0f; i < 1.00f; i = i + 0.01f) {
      float x, y, value, phase;
      value = 0.0f;
      x = 2.0f + (box.size.x - 4.0f) * i;
      phase = applyPW(eucmod(i + shiftVal, 1.0f), pwVal);
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
    nvgStroke(args.vg);
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

  float phase = 0.0f;
  float pw = 0.5f;
  float shift = 0.0f;
  float wave = 0.0f;

  float pwParam = 0.5f;
  float lastPwParam = 0.5f;
  float pwDisplay = 50.0f;

  float shiftParam = 0.0f;
  float lastShiftParam = 0.0f;
  float shiftDisplay = 0.0f;

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

  FN3() : Module(NUM_PARAMS, NUM_INPUTS, NUM_OUTPUTS, NUM_LIGHTS) {
  }
  void process(const ProcessArgs &args) override;
};


void FN3::process(const ProcessArgs &args) {
  if (params[PW_PARAM].value != lastPwParam) {
    pwParam = snap(params[PW_PARAM].value);
    lastPwParam = params[PW_PARAM].value;
  }
  pw = clamp(pwParam + (inputs[PW_INPUT].isConnected() ? inputs[PW_INPUT].value / 10.0f : 0.0f), 0.0f, 1.0f);
  pwDisplay = pw * 100.0f;

  if (params[SHIFT_PARAM].value != lastShiftParam) {
    shiftParam = snap(params[SHIFT_PARAM].value);
    lastShiftParam = params[SHIFT_PARAM].value;
  }
  shift = shiftParam + (inputs[SHIFT_INPUT].isConnected() ? inputs[SHIFT_INPUT].value / -5.0f : 0.0f);
  shiftDisplay = shift * -100.0f;

  phase = applyPW(eucmod((inputs[PHASE_INPUT].isConnected() ? inputs[PHASE_INPUT].value / 10.0f : 0.0f) + shift, 1.0f), pw);
  wave = params[WAVE_PARAM].value;

  if (wave == 0.0f) {
    outputs[WAVE_OUTPUT].value = fn3Sin(phase) * 10.0f - (params[OFFSET_PARAM].value == 1.0f ? 5.0f : 0.0f);
  } else if (wave == 1.0f) {
    outputs[WAVE_OUTPUT].value = fn3Tri(phase) * 10.0f - (params[OFFSET_PARAM].value == 1.0f ? 5.0f : 0.0f);
  } else {
    outputs[WAVE_OUTPUT].value = fn3Sqr(phase) * 10.0f - (params[OFFSET_PARAM].value == 1.0f ? 5.0f : 0.0f);
  }
}


struct FN3Widget : ModuleWidget {
  FN3Widget(FN3 *module) : ModuleWidget(module) {
    setPanel(SVG::load(assetPlugin(pluginInstance, "res/panels/FN-3.svg")));

    addParam(createParam<ZZC_Knob27>(Vec(9, 58), module, FN3::PW_PARAM, 0.0f, 1.0f, 0.5f));
    addInput(createInput<ZZC_PJ_Port>(Vec(10.5, 93), module, FN3::PW_INPUT));

    FN3DisplayWidget *display = new FN3DisplayWidget();
    display->box.pos = Vec(8.0f, 126.0f);
    display->box.size = Vec(29.0f, 49.0f);
    if (module) {
      display->wave = &module->wave;
      display->pw = &module->pw;
      display->shift = &module->shift;
    }
    addChild(display);
    addParam(createParam<ZZC_FN3WaveSwitch>(Vec(8, 126), module, FN3::WAVE_PARAM, 0.0f, 2.0f, 0.0f));
    addParam(createParam<ZZC_FN3UniBiSwitch>(Vec(8, 152), module, FN3::OFFSET_PARAM, 0.0f, 1.0f, 0.0f));

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
    addParam(createParam<ZZC_Knob25>(Vec(10, 229), module, FN3::SHIFT_PARAM, 1.0f, -1.0f, 0.0f));

    addInput(createInput<ZZC_PJ_Port>(Vec(10.5, 275), module, FN3::PHASE_INPUT));
    addOutput(createOutput<ZZC_PJ_Port>(Vec(10.5, 320), module, FN3::WAVE_OUTPUT));

    addChild(createWidget<ZZC_Screw>(Vec(box.size.x / 2 - RACK_GRID_WIDTH / 2, 0)));
    addChild(createWidget<ZZC_Screw>(Vec(box.size.x / 2 - RACK_GRID_WIDTH / 2, RACK_GRID_HEIGHT - RACK_GRID_WIDTH)));
  }
};


Model *modelFN3 = createModel<FN3, FN3Widget>("FN-3");
