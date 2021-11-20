#ifndef DISPLAYS_H
#define DISPLAYS_H
#include "displays.hpp"
#endif

#ifndef KNOB_SENSITIVITY_CONST
#define KNOB_SENSITIVITY_CONST
static const float KNOB_SENSITIVITY = 0.0015f;
#endif

using namespace rack;

struct XYDisplayViewWidget : BaseDisplayWidget {
  float x = 0.f;
  float y = 0.f;
  float drawnX = 0.0f;
  float drawnY = 0.0f;
  float padding = 2.0f;
  double lastDrawnAt = 0.0;

  NVGcolor lcdGhostColor = nvgRGB(0x1e, 0x1f, 0x1d);
  NVGcolor posColor = nvgRGB(0x9c, 0xd7, 0x43);
  NVGcolor negColor = nvgRGB(0xe7, 0x34, 0x2d);

  float scaleValue(float value, float area) {
    return padding + ((value + 1.0f) / 2.0f) * (area - padding * 2.0f);
  }

  void drawCross(const DrawArgs &args, Vec position) {
    if (!y || (y == 0.0f)) {
      nvgStrokeColor(args.vg, lcdGhostColor);
    } else if (y < 0.0f) {
      nvgStrokeColor(args.vg, negColor);
    } else {
      nvgStrokeColor(args.vg, posColor);
    }
    nvgStrokeWidth(args.vg, 1.0f);

    nvgBeginPath(args.vg);
    nvgMoveTo(args.vg, position.x - 5.5f, position.y);
    nvgLineTo(args.vg, position.x + 5.5f, position.y);
    nvgStroke(args.vg);

    nvgBeginPath(args.vg);
    nvgMoveTo(args.vg, position.x, position.y - 5.5f);
    nvgLineTo(args.vg, position.x, position.y + 5.5f);
    nvgStroke(args.vg);
  }

  void draw(const DrawArgs &args) override {
    drawBackground(args);
    nvgScissor(args.vg, padding, padding, box.size.x - padding * 2.0f, box.size.y - padding * 2.0f);
    drawCross(args, Vec(
      scaleValue(x, box.size.x),
      scaleValue(-y, box.size.y)
    ));
    nvgResetScissor(args.vg);
    this->drawnX = x;
    this->drawnY = y;
    this->lastDrawnAt = glfwGetTime();
  }

  bool shouldUpdate() {
    if (glfwGetTime() - this->lastDrawnAt < 0.016) { return false; } // ~60 FPS
    if (x == this->drawnX && y == this->drawnY) { return false; }
    if (x == 0.0f || y == 0.0f) { return true; }
    return fabsf(this->drawnX - x) > 0.05 || fabsf(this->drawnY - y) > 0.05;
  }
};

struct XYDisplayWidget : ParamWidget {
  engine::ParamQuantity *paramQuantityX = NULL;
  engine::ParamQuantity *paramQuantityY = NULL;
  float lastX = 0.0f;
  float lastY = 0.0f;
  float oldValueX = 0.f;
  float oldValueY = 0.f;
  float dragDelta;
  XYDisplayViewWidget *disp;
  float speed = 4.0;
  widget::FramebufferWidget *fb;

  XYDisplayWidget() {
    fb = new widget::FramebufferWidget;
	  addChild(fb);

    disp = new XYDisplayViewWidget();
    fb->addChild(disp);
  }

  void setupSize() {
    disp->box.pos = Vec(0, 0);
    disp->box.size = this->box.size;
    fb->box.size = this->box.size;
  }

  void onDragStart(const event::DragStart &e) override {
    if (e.button != GLFW_MOUSE_BUTTON_LEFT) {
		  return;
    }
    if (paramQuantityX && paramQuantityY) {
      oldValueX = paramQuantityX->getValue();
      oldValueY = paramQuantityY->getValue();
    }
    APP->window->cursorLock();
    dragDelta = 0.0;
  }

  void onDragMove(const event::DragMove &e) override {
    float deltaX = KNOB_SENSITIVITY * e.mouseDelta.x * speed;
    float deltaY = KNOB_SENSITIVITY * -e.mouseDelta.y * speed;

    // Drag slower if mod is held
    int mods = APP->window->getMods();
    if ((mods & RACK_MOD_MASK) == RACK_MOD_CTRL) {
      deltaX /= 16.f;
      deltaY /= 16.f;
    }
    // Drag even slower if mod+shift is held
    if ((mods & RACK_MOD_MASK) == (RACK_MOD_CTRL | GLFW_MOD_SHIFT)) {
      deltaX /= 256.f;
      deltaY /= 256.f;
    }

    onInput(deltaX, deltaY);

    fb->dirty = true;
  }

  void onDragEnd(const event::DragEnd &e) override {
    if (e.button != GLFW_MOUSE_BUTTON_LEFT) {
		  return;
    }

	  APP->window->cursorUnlock();
  }

  void onInput(float deltaX, float deltaY) {
    if (paramQuantityX && paramQuantityY) {
      paramQuantityX->setValue(math::clamp(paramQuantityX->getValue() + deltaX, paramQuantityX->getMinValue(), paramQuantityX->getMaxValue()));
      paramQuantityY->setValue(math::clamp(paramQuantityY->getValue() + deltaY, paramQuantityY->getMinValue(), paramQuantityY->getMaxValue()));
    }
  }

  void onReset() {
    if (paramQuantityX && paramQuantityY) {
      paramQuantityX->reset();
      paramQuantityY->reset();
      fb->dirty = true;
    }
  }

  void onChange(const event::Change &e) override {
    if (paramQuantityX && paramQuantityY) {
      fb->dirty = true;
    }
    ParamWidget::onChange(e);
  }

  void step() override {
    if (paramQuantityX && paramQuantityY) {
      disp->x = paramQuantityX->getValue();
      disp->y = paramQuantityY->getValue();
      if (disp->shouldUpdate()) {
        event::Change eChange;
        onChange(eChange);
      }
    }

    Widget::step();
  }

  void onButton(const event::Button &e) override {
    OpaqueWidget::onButton(e);
  }
};
