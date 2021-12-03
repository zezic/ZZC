#include <iostream>

using namespace rack;

extern Plugin *pluginInstance;

struct BaseDisplayWidget : TransparentWidget {
  NVGcolor backgroundColor = nvgRGB(0x01, 0x01, 0x01);
  NVGcolor lcdColor = nvgRGB(0x12, 0x12, 0x12);
  NVGcolor lcdGhostColor = nvgRGBA(0xff, 0xff, 0xff, 0x10);
  NVGcolor lcdTextColor = nvgRGB(0xff, 0xd4, 0x2a);
  NVGcolor haloColor = lcdTextColor;

  void draw(const DrawArgs &args) override {
    drawBackground(args);
  }

  void drawBackground(const DrawArgs &args) {
    // Background
    nvgBeginPath(args.vg);
    nvgRoundedRect(args.vg, 0.0, 0.0, box.size.x, box.size.y, 3.0);
    nvgFillColor(args.vg, backgroundColor);
    nvgFill(args.vg);

    // LCD
    nvgBeginPath(args.vg);
    nvgRoundedRect(args.vg, 2.0, 2.0, box.size.x - 4.0, box.size.y - 4.0, 1.0);
    nvgFillColor(args.vg, lcdColor);
    nvgFill(args.vg);
  }

  void drawHalo(const DrawArgs& args) {
    // Don't draw halo if rendering in a framebuffer, e.g. screenshots or Module Browser
    if (args.fb)
      return;

    const float halo = settings::haloBrightness;
    if (halo == 0.f)
      return;

    // If light is off, rendering the halo gives no effect.
    if (this->lcdTextColor.r == 0.f && this->lcdTextColor.g == 0.f && this->lcdTextColor.b == 0.f)
      return;

    math::Vec c = box.size.div(2);
    float radius = 0.f;
    float oradius = std::max(box.size.x, box.size.y);

    nvgBeginPath(args.vg);
    nvgRect(args.vg, c.x - oradius, c.y - oradius, 2 * oradius, 2 * oradius);

    float ourHalo = std::min(halo * 0.5f, 0.15f);
    NVGcolor icol = color::mult(this->haloColor, ourHalo);
    NVGcolor ocol = nvgRGBA(this->haloColor.r, this->haloColor.g, this->haloColor.b, 0);
    // NVGcolor ocol = nvgRGBA(0xff, 0x00, 0x00, 0x1f);
    NVGpaint paint = nvgRadialGradient(args.vg, c.x, c.y, radius, oradius, icol, ocol);
    nvgFillPaint(args.vg, paint);
    nvgFill(args.vg);
  }
};

struct Display32Widget : BaseDisplayWidget {
  float *value = nullptr;
  bool *disabled = nullptr;

  void drawLayer(const DrawArgs &args, int layer) override {
    if (layer != 1) { return; }

    std::shared_ptr<Font> font = APP->window->loadFont(asset::plugin(pluginInstance, "res/fonts/DSEG/DSEG7ClassicMini-Italic.ttf"));
    if (!font) { return; }

    float valueToDraw = std::abs(value ? *value : 120.0f);

    // Text (integer part)
    nvgFontSize(args.vg, 11);
    nvgFontFaceId(args.vg, font->handle);
    nvgTextLetterSpacing(args.vg, 1.0);
    nvgTextAlign(args.vg, NVG_ALIGN_RIGHT);

    char integerPartString[10];
    if (valueToDraw >= 1000.0f || (disabled && *disabled)) {
      snprintf(integerPartString, sizeof(integerPartString), "---.");
    } else {
      snprintf(integerPartString, sizeof(integerPartString), "%3.0f.", floor(valueToDraw));
    }

    Vec textPos = Vec(36.0f, 16.0f);

    nvgFillColor(args.vg, lcdGhostColor);
    nvgText(args.vg, textPos.x, textPos.y, "888.", NULL);
    nvgFillColor(args.vg, lcdTextColor);
    nvgText(args.vg, textPos.x, textPos.y, integerPartString, NULL);

    // Text (fractional part)
    nvgFontSize(args.vg, 9);
    nvgTextLetterSpacing(args.vg, 0.0);

    char fractionalPartString[10];
    float remainder = fmod(valueToDraw, 1.0f) * 100.0f;
    float intpart;
    std::modf(remainder, &intpart);
    if (disabled && *disabled) {
      snprintf(fractionalPartString, sizeof(fractionalPartString), "--");
    } else if (valueToDraw >= 1000.0f) {
      snprintf(fractionalPartString, sizeof(fractionalPartString), "--");
    } else if (intpart == 0.0f) {
      snprintf(fractionalPartString, sizeof(fractionalPartString), "00");
    } else {
      snprintf(fractionalPartString, sizeof(fractionalPartString), "%2.0f", intpart);
      if (fractionalPartString[0] == ' ') {
        fractionalPartString[0] = '0';
      }
    }

    textPos = Vec(52.0f, 16.0f);

    nvgFillColor(args.vg, lcdGhostColor);
    nvgText(args.vg, textPos.x, textPos.y, "88", NULL);
    nvgFillColor(args.vg, lcdTextColor);
    nvgText(args.vg, textPos.x, textPos.y, fractionalPartString, NULL);

    nvgGlobalCompositeBlendFunc(args.vg, NVG_ONE_MINUS_DST_COLOR, NVG_ONE);
    drawHalo(args);
  }
};

struct DisplayIntpartWidget : BaseDisplayWidget {
  float *value = nullptr;

  void drawLayer(const DrawArgs &args, int layer) override {
    if (layer != 1) { return; }

    std::shared_ptr<Font> font = APP->window->loadFont(asset::plugin(pluginInstance, "res/fonts/DSEG/DSEG7ClassicMini-Italic.ttf"));
    if (!font) { return; }

    // Text (integer part)
    nvgFontSize(args.vg, 11);
    nvgFontFaceId(args.vg, font->handle);
    nvgTextLetterSpacing(args.vg, 1.0);
    nvgTextAlign(args.vg, NVG_ALIGN_RIGHT);

    char integerPartString[10];
    snprintf(integerPartString, sizeof(integerPartString), "%8.0f", value ? *value : 8.0f);

    Vec textPos = Vec(box.size.x - 5.0f, 16.0f);

    nvgFillColor(args.vg, lcdGhostColor);
    nvgText(args.vg, textPos.x, textPos.y, "88", NULL);
    nvgFillColor(args.vg, lcdTextColor);
    nvgText(args.vg, textPos.x, textPos.y, integerPartString, NULL);

    nvgGlobalCompositeBlendFunc(args.vg, NVG_ONE_MINUS_DST_COLOR, NVG_ONE);
    drawHalo(args);
  }
};

struct IntDisplayWidget : BaseDisplayWidget {
  int *value = nullptr;
  int *polarity = nullptr;
  bool *isPoly = nullptr;
  bool *blinking = nullptr;
  int blinkingPhase = 0;
  std::string textGhost = "88";
  NVGcolor lcdTextColorBlink = nvgRGB(0x8a, 0x72, 0x17);
  NVGcolor negColor = nvgRGB(0xe7, 0x34, 0x2d);
  NVGcolor negColorBlink = nvgRGB(0x8a, 0x1f, 0x1b);
  NVGcolor posColor = nvgRGB(0x9c, 0xd7, 0x43);
  NVGcolor posColorBlink = nvgRGB(0x51, 0x70, 0x23);
  NVGcolor polyColor = nvgRGB(0x76, 0xdc, 0xfa);
  NVGcolor polyColorBlink = nvgRGB(0x43, 0x7e, 0x8f);

  void drawLayer(const DrawArgs &args, int layer) override {
    if (layer != 1) { return; }

    std::shared_ptr<Font> font = APP->window->loadFont(asset::plugin(pluginInstance, "res/fonts/DSEG/DSEG7ClassicMini-Italic.ttf"));
    if (!font) { return; }

    nvgFontSize(args.vg, 11);
    nvgFontFaceId(args.vg, font->handle);
    nvgTextLetterSpacing(args.vg, 1.0);
    nvgTextAlign(args.vg, NVG_ALIGN_RIGHT);

    char integerString[10];
    snprintf(integerString, sizeof(integerString), "%d", value ? *value : 1);

    Vec textPos = Vec(box.size.x - 5.0f, 16.0f);

    bool isBlinking = blinking && *blinking;
    if (isBlinking) {
      blinkingPhase = (blinkingPhase + 1) % 24;
    }
    bool blink = isBlinking && blinkingPhase < 12;

    nvgFillColor(args.vg, lcdGhostColor);
    nvgText(args.vg, textPos.x, textPos.y, textGhost.c_str(), NULL);
    if (isPoly && *isPoly) {
      NVGcolor fillColor = blink ? polyColorBlink : polyColor;
      nvgFillColor(args.vg, fillColor);
      this->haloColor = fillColor;
    } else {
      if (polarity) {
        if (*polarity == 0) {
          NVGcolor fillColor = blink ? lcdTextColorBlink : lcdTextColor;
          nvgFillColor(args.vg, fillColor);
          this->haloColor = fillColor;
        } else {
          NVGcolor fillColor = *polarity > 0 ? (blink ? posColorBlink : posColor) : (blink ? negColorBlink : negColor);
          nvgFillColor(args.vg, fillColor);
          this->haloColor = fillColor;
        }
      } else {
        NVGcolor fillColor = blink ? lcdTextColorBlink : lcdTextColor;
        nvgFillColor(args.vg, fillColor);
        this->haloColor = fillColor;
      }
    }
    nvgText(args.vg, textPos.x, textPos.y, integerString, NULL);

    nvgGlobalCompositeBlendFunc(args.vg, NVG_ONE_MINUS_DST_COLOR, NVG_ONE);
    drawHalo(args);
  }
};

struct RatioDisplayWidget : BaseDisplayWidget {
  float *from = nullptr;
  float *to = nullptr;

  void drawLayer(const DrawArgs &args, int layer) override {
    if (layer != 1) { return; }

    std::shared_ptr<Font> font = APP->window->loadFont(asset::plugin(pluginInstance, "res/fonts/DSEG/DSEG7ClassicMini-Italic.ttf"));
    if (!font) { return; }

    nvgFontSize(args.vg, 11);
    nvgFontFaceId(args.vg, font->handle);
    nvgTextLetterSpacing(args.vg, 1.0);

    // Text (from)
    nvgTextAlign(args.vg, NVG_ALIGN_RIGHT);

    char fromString[10];
    snprintf(fromString, sizeof(fromString), "%2.0f", from ? *from : 1.0);

    Vec textPos = Vec(box.size.x / 2.0f - 3.0f, 16.0f);

    nvgFillColor(args.vg, lcdGhostColor);
    nvgText(args.vg, textPos.x, textPos.y, "88", NULL);
    nvgFillColor(args.vg, lcdTextColor);
    nvgText(args.vg, textPos.x, textPos.y, fromString, NULL);

    // Text (from)
    nvgTextAlign(args.vg, NVG_ALIGN_LEFT);

    char toString[10];
    snprintf(toString, sizeof(toString), "%2.0f", to ? *to : 1.0);
    if (toString[0] == ' ') {
      toString[0] = toString[1];
      toString[1] = ' ';
    }

    textPos = Vec(box.size.x / 2.0f + 2.0f, 16.0f);

    nvgFillColor(args.vg, lcdGhostColor);
    nvgText(args.vg, textPos.x, textPos.y, "88", NULL);
    nvgFillColor(args.vg, lcdTextColor);
    nvgText(args.vg, textPos.x, textPos.y, toString, NULL);

    // Text (:)
    nvgTextAlign(args.vg, NVG_ALIGN_CENTER);

    textPos = Vec(box.size.x / 2.0f, 16.0f);

    nvgFillColor(args.vg, lcdTextColor);
    nvgText(args.vg, textPos.x, textPos.y, ":", NULL);

    nvgGlobalCompositeBlendFunc(args.vg, NVG_ONE_MINUS_DST_COLOR, NVG_ONE);
    drawHalo(args);
  }
};
