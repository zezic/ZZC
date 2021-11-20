#ifndef KNOB_SENSITIVITY_CONST
#define KNOB_SENSITIVITY_CONST
static const float KNOB_SENSITIVITY = 0.0015f;
#endif

using namespace rack;

struct ZZC_DirectKnobDisplay : TransparentWidget {
  NVGcolor backdropColor = nvgRGB(0x63, 0x63, 0x55);
  NVGcolor valueColor = nvgRGB(0xff, 0xd4, 0x2a);
  NVGcolor posColor = nvgRGB(0x9c, 0xd7, 0x43);
  NVGcolor negColor = nvgRGB(0xe7, 0x34, 0x2d);

  float value = 0.f;
  float drawnValue = 0.0f;
  double lastDrawnAt = 0.0;
  float minVal;
  float maxVal;
  float center = 0.75;
  float girth = 0.4;
  float strokeWidth = 3.0;
  float startFrom = center;
  float range = girth;
  bool colored = false;
  float visibleDelta = 0.0f;
  float defaultValue = 0.0f;

  void setLimits(float low, float high) {
    this->visibleDelta = (high - low) / 50.0f;
    minVal = low;
    maxVal = high;
    startFrom = minVal == 0.0 ? center - girth : center;
    range = minVal == 0.0 ? girth * 2.0 : girth;
  }

  void enableColor() {
    colored = true;
  }

  void draw(const DrawArgs &args) override {
    lastDrawnAt = glfwGetTime();
    drawnValue = value;
    // return;
    nvgLineCap(args.vg, NSVG_CAP_ROUND);
    nvgStrokeWidth(args.vg, strokeWidth);

    nvgStrokeColor(args.vg, backdropColor);
    nvgBeginPath(args.vg);
    nvgArc(
      args.vg,
      box.size.x / 2.0, box.size.y / 2.0, box.size.x / 2.0 - strokeWidth / 2.0,
      (center + girth) * 2 * M_PI,
      (center - girth) * 2 * M_PI,
      1
    );
    nvgStroke(args.vg);

    if (drawnValue == 0.0) {
      return;
    }
    if (colored) {
      nvgStrokeColor(args.vg, drawnValue > 0.0 ? posColor : negColor);
    } else {
      nvgStrokeColor(args.vg, valueColor);
    }
    nvgBeginPath(args.vg);
    nvgArc(
      args.vg,
      box.size.x / 2.0, box.size.y / 2.0, box.size.x / 2.0 - strokeWidth / 2.0,
      startFrom * 2 * M_PI,
      (startFrom + (drawnValue / maxVal) * range) * 2 * M_PI,
      drawnValue >= 0 ? 2 : 1
    );
    nvgStroke(args.vg);
  }

  bool shouldUpdate(float *newValue) {
    if (*newValue == this->drawnValue) { return false; }
    if (glfwGetTime() - this->lastDrawnAt < 0.016) { return false; } // ~60FPS
    return fabsf(*newValue - this->drawnValue) > this->visibleDelta || *newValue == this->defaultValue;
  }
};

struct ZZC_CallbackKnob : Knob {
  widget::FramebufferWidget *fb;
  CircularShadow *shadow;
  widget::TransformWidget *tw;
  widget::SvgWidget *sw;

  ZZC_DirectKnobDisplay *disp = nullptr;
  float *value = nullptr;
  bool randomizable = true;
  float speed = 2.0;
  float rotation = 0.0f;
  float lastRotation = 0.0f;
  float minAngle = -1.0 * M_PI;
  float maxAngle = 1.0 * M_PI;
  float strokeWidth = 3.0;
  float padding = strokeWidth + 2.0;
  float rotationMult = 1.0;
  float deltaMult = 1.0f;
  bool dirty = true;
  bool showDisplay = true;
  float lastQuantityValue = 0.f;
  float *lastQuantityValueP = nullptr;

  ZZC_CallbackKnob() {
    fb = new widget::FramebufferWidget;
    addChild(fb);

    if (showDisplay) {
      disp = new ZZC_DirectKnobDisplay();
      fb->addChild(disp);
      disp->box.pos = math::Vec(0, 0);
    }

    shadow = new CircularShadow;
    fb->addChild(shadow);
    shadow->box.size = math::Vec();

    tw = new widget::TransformWidget;
    fb->addChild(tw);

    sw = new widget::SvgWidget;
    tw->addChild(sw);
  }

  void attachValue(float *valuePointer, float limitLow, float limitHigh, float defaultValue) {
    value = valuePointer;
    deltaMult = (limitHigh - limitLow) * 0.5f;
    if (disp) {
      // disp->value = value;
      disp->setLimits(limitLow, limitHigh);
      disp->defaultValue = defaultValue;
    }
  }

  void setSvg(std::shared_ptr<Svg> svg) {

    padding = strokeWidth + 2.0;

    sw->setSvg(svg);
    sw->box.pos = showDisplay ? math::Vec(padding, padding) : math::Vec(0, 0);
    tw->box.size = sw->box.size;
    fb->box.size = showDisplay ? math::Vec(padding * 2, padding * 2).plus(sw->box.size) : sw->box.size;
    box.size = sw->box.size;
    shadow->box.size = sw->box.size;
    // Move shadow downward by 20% and take value display into account
    shadow->box.pos = showDisplay ? math::Vec(padding, padding).plus(math::Vec(0, sw->box.size.y * 0.2)) : math::Vec(0, sw->box.size.y * 0.2);

    if (disp) {
      disp->strokeWidth = strokeWidth;
    }

    if (disp) {
      disp->box.size = fb->box.size;
    }
  }

  virtual void onInput(float factor) = 0;
  virtual void onAbsInput(float value) = 0;
  virtual void onReset() = 0;

  void onDragMove(const event::DragMove &e) override {
    if (e.button != GLFW_MOUSE_BUTTON_LEFT) {
		  return;
    }
    float delta = KNOB_SENSITIVITY * -e.mouseDelta.y * speed;

    // Drag slower if mod is held
		int mods = APP->window->getMods();
		if ((mods & RACK_MOD_MASK) == RACK_MOD_CTRL) {
			delta /= 16.f;
		}
		// Drag even slower if mod+shift is held
		if ((mods & RACK_MOD_MASK) == (RACK_MOD_CTRL | GLFW_MOD_SHIFT)) {
			delta /= 256.f;
		}
    rotation += delta * rotationMult;
    this->onInput(delta * deltaMult);
    dirty = true;
  }
  void onDoubleClick(const event::DoubleClick &e) override {
    this->onReset();
    this->fb->dirty = true;
  }

  void step() override {
    if (disp && value && disp->shouldUpdate(value)) {
      dirty = true;
      fb->dirty = true;
    }
    if (dirty && (rotation != lastRotation)) {
      float angle;
      angle = rescale(rotation, -1.0, 1.0, minAngle, maxAngle);
      angle = fmodf(angle, 2*M_PI);
      tw->identity();
      // Rotate SVG
      math::Vec center = sw->box.getCenter();
      tw->translate(center);
      tw->rotate(angle);
      tw->translate(center.neg());
      lastRotation = rotation;
      fb->dirty = true;
    }

    engine::ParamQuantity* paramQuantity = this->getParamQuantity();
    if (paramQuantity) {
      float quantityValue = paramQuantity->getValue();
      if (lastQuantityValueP) {
        if (lastQuantityValue != quantityValue) {
          this->onAbsInput(quantityValue);
        }
      } else {
        lastQuantityValueP = &lastQuantityValue;
      }
      lastQuantityValue = quantityValue;
    }
    Widget::step();
  }

  void enableColor() {
    if (disp) {
      disp->enableColor();
    }
  }

  void draw(const DrawArgs &args) override {
    Widget::draw(args);
  }
};
