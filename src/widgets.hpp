struct BaseDisplayWidget : TransparentWidget {

  void drawBackground(NVGcontext *vg) {
    NVGcolor backgroundColor = nvgRGB(0x01, 0x01, 0x01);
    NVGcolor lcdColor = nvgRGB(0x12, 0x12, 0x12);

    // Background
    nvgBeginPath(vg);
    nvgRoundedRect(vg, 0.0, 0.0, box.size.x, box.size.y, 3.0);
    nvgFillColor(vg, backgroundColor);
    nvgFill(vg);

    // LCD
    nvgBeginPath(vg);
    nvgRoundedRect(vg, 2.0, 2.0, box.size.x - 4.0, box.size.y - 4.0, 1.0);
    nvgFillColor(vg, lcdColor);
    nvgFill(vg);
  }
};

struct Display32Widget : BaseDisplayWidget {
  float *value;
  std::shared_ptr<Font> font;

  Display32Widget() {
    font = Font::load(assetPlugin(plugin, "res/DSEG7ClassicMini-Italic.ttf"));
  };

  void draw(NVGcontext *vg) override {
    drawBackground(vg);
    float valueToDraw = fabsf(*value);
    NVGcolor lcdGhostColor = nvgRGB(0x1e, 0x1f, 0x1d);
    NVGcolor lcdTextColor = nvgRGB(0xff, 0xd4, 0x2a);

    // Text (integer part)
    nvgFontSize(vg, 11);
    nvgFontFaceId(vg, font->handle);
    nvgTextLetterSpacing(vg, 1.0);
    nvgTextAlign(vg, NVG_ALIGN_RIGHT);

    char integerPartString[10];
    if (valueToDraw >= 1000.0f) {
      sprintf(integerPartString, "F00.");
    } else {
      sprintf(integerPartString, "%3.0f.", floor(valueToDraw));
    }

    Vec textPos = Vec(36.0f, 16.0f); 

    nvgFillColor(vg, lcdGhostColor);
    nvgText(vg, textPos.x, textPos.y, "888.", NULL);
    nvgFillColor(vg, lcdTextColor);
    nvgText(vg, textPos.x, textPos.y, integerPartString, NULL);

    // Text (fractional part)
    nvgFontSize(vg, 9);
    nvgTextLetterSpacing(vg, 0.0);

    char fractionalPartString[10];
    float remainder = fmod(valueToDraw, 1.0f) * 100.0f;
    float intpart;
    modf(remainder, &intpart);
    if (intpart == 0.0f || valueToDraw >= 1000.0f) {
      sprintf(fractionalPartString, "00");
    } else {
      sprintf(fractionalPartString, "%2.0f", intpart);
      if (fractionalPartString[0] == ' ') {
        fractionalPartString[0] = '0';
      }
    }

    textPos = Vec(52.0f, 16.0f); 

    nvgFillColor(vg, lcdGhostColor);
    nvgText(vg, textPos.x, textPos.y, "88", NULL);
    nvgFillColor(vg, lcdTextColor);
    nvgText(vg, textPos.x, textPos.y, fractionalPartString, NULL);
  }
};

struct DisplayIntpartWidget : BaseDisplayWidget {
  float *value;
  std::shared_ptr<Font> font;

  DisplayIntpartWidget() {
    font = Font::load(assetPlugin(plugin, "res/DSEG7ClassicMini-Italic.ttf"));
  };

  void draw(NVGcontext *vg) override {
    drawBackground(vg);
    NVGcolor lcdGhostColor = nvgRGB(0x1e, 0x1f, 0x1d);
    NVGcolor lcdTextColor = nvgRGB(0xff, 0xd4, 0x2a);

    // Text (integer part)
    nvgFontSize(vg, 11);
    nvgFontFaceId(vg, font->handle);
    nvgTextLetterSpacing(vg, 1.0);
    nvgTextAlign(vg, NVG_ALIGN_RIGHT);

    char integerPartString[10];
    sprintf(integerPartString, "%8.0f", *value);

    Vec textPos = Vec(box.size.x - 5.0f, 16.0f); 

    nvgFillColor(vg, lcdGhostColor);
    nvgText(vg, textPos.x, textPos.y, "88", NULL);
    nvgFillColor(vg, lcdTextColor);
    nvgText(vg, textPos.x, textPos.y, integerPartString, NULL);
  }
};

struct RatioDisplayWidget : BaseDisplayWidget {
  float *from;
  float *to;
  std::shared_ptr<Font> font;

  RatioDisplayWidget() {
    font = Font::load(assetPlugin(plugin, "res/DSEG7ClassicMini-Italic.ttf"));
  };

  void draw(NVGcontext *vg) override {
    drawBackground(vg);
    NVGcolor lcdGhostColor = nvgRGB(0x1e, 0x1f, 0x1d);
    NVGcolor lcdTextColor = nvgRGB(0xff, 0xd4, 0x2a);

    nvgFontSize(vg, 11);
    nvgFontFaceId(vg, font->handle);
    nvgTextLetterSpacing(vg, 1.0);

    // Text (from)
    nvgTextAlign(vg, NVG_ALIGN_RIGHT);

    char fromString[10];
    sprintf(fromString, "%2.0f", *from);

    Vec textPos = Vec(box.size.x / 2.0f - 3.0f, 16.0f); 

    nvgFillColor(vg, lcdGhostColor);
    nvgText(vg, textPos.x, textPos.y, "88", NULL);
    nvgFillColor(vg, lcdTextColor);
    nvgText(vg, textPos.x, textPos.y, fromString, NULL);

    // Text (from)
    nvgTextAlign(vg, NVG_ALIGN_LEFT);

    char toString[10];
    sprintf(toString, "%2.0f", *to);
    if (toString[0] == ' ') {
      toString[0] = toString[1];
      toString[1] = ' ';
    }

    textPos = Vec(box.size.x / 2.0f + 2.0f, 16.0f); 

    nvgFillColor(vg, lcdGhostColor);
    nvgText(vg, textPos.x, textPos.y, "88", NULL);
    nvgFillColor(vg, lcdTextColor);
    nvgText(vg, textPos.x, textPos.y, toString, NULL);

    // Text (:)
    nvgTextAlign(vg, NVG_ALIGN_CENTER);

    textPos = Vec(box.size.x / 2.0f, 16.0f); 

    nvgFillColor(vg, lcdTextColor);
    nvgText(vg, textPos.x, textPos.y, ":", NULL);
  }
};

template <typename BASE>
struct LedLight : BASE {
	LedLight() {
	  this->box.size = mm2px(Vec(6.3f, 6.3f));
	}
};

struct LEDBezelDark : SVGSwitch, MomentarySwitch {
	LEDBezelDark() {
		addFrame(SVG::load(assetPlugin(plugin, "res/LEDBezelDark.svg")));
	}
};