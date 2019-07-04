#include "rack0.hpp"

static const NVGcolor COLOR_ZZC_YELLOW = nvgRGB(0xff, 0xd4, 0x2a);
static const NVGcolor COLOR_NEG = nvgRGB(0xe7, 0x34, 0x2d);
static const NVGcolor COLOR_POS = nvgRGB(0x90, 0xc7, 0x3e);

using namespace rack;

extern Plugin *pluginInstance;

template <typename BASE>
struct LedLight : BASE {
  LedLight() {
    this->box.size = mm2px(Vec(6.3f, 6.3f));
  }
};


struct ZZC_BaseLight : GrayModuleLightWidget {
  std::vector<float> values = { 0.0f, 0.0f, 0.0f, 0.0f };
  double lastStepAt = 0.0;

  ZZC_BaseLight() {
  }
  void drawHalo(const DrawArgs &args) override {
    float radius = box.size.x / 2.0;
    float oradius = radius + 15.0;

    nvgBeginPath(args.vg);
    nvgRect(args.vg, radius - oradius, radius - oradius, 2*oradius, 2*oradius);

    NVGpaint paint;
    NVGcolor icol = color::mult(color, 0.04);
    NVGcolor ocol = nvgRGB(0, 0, 0);
    paint = nvgRadialGradient(args.vg, radius, radius, radius, oradius, icol, ocol);
    nvgFillPaint(args.vg, paint);
    nvgGlobalCompositeOperation(args.vg, NVG_LIGHTER);
    nvgFill(args.vg);
  }
  void step() override {
    if (module) {
      assert(module->lights.size() >= firstLightId + baseColors.size());
      double now = glfwGetTime();
      double timeDelta = now - lastStepAt;
      for (size_t i = 0; i < baseColors.size(); i++) {
        float value = module->lights[firstLightId + i].getBrightness();
        if (value == 1.0f) {
          values[i] = 1.0f;
        } else if (value == 1.1f) {
          module->lights[firstLightId + i].value = 0.0f;
          values[i] = 1.0f;
        } else if (value > 0.0f) {
          values[i] = value;
        } else if (values[i] > 0.0f){
          values[i] = fmaxf(0.0f, values[i] - values[i] * 8.0f * timeDelta);
        }
      }
      lastStepAt = now;
    } else {
      // Turn all lights on
      for (size_t i = 0; i < baseColors.size(); i++) {
        values[i] = 1.f;
      }
    }
    setBrightnesses(values);
  }
  void setBrightnesses(const std::vector<float> &brightnesses) {
    color = nvgRGBAf(0, 0, 0, 0);
    for (size_t i = 0; i < baseColors.size(); i++) {
      NVGcolor c = baseColors[i];
      c.a *= math::clamp(brightnesses[i], 0.f, 1.f);
      color = color::screen(color, c);
    }
    color = color::clamp(color);
  }
};

struct ZZC_YellowLight : ZZC_BaseLight {
  ZZC_YellowLight() {
    addBaseColor(COLOR_ZZC_YELLOW);
  }
};

struct ZZC_RedLight : ZZC_BaseLight {
  ZZC_RedLight() {
    addBaseColor(COLOR_NEG);
  }
};

struct ZZC_GreenRedLight : ZZC_BaseLight {
  ZZC_GreenRedLight() {
    addBaseColor(COLOR_POS);
    addBaseColor(COLOR_NEG);
  }
};