#ifndef KNOB_SENSITIVITY_CONST
#define KNOB_SENSITIVITY_CONST
static const float KNOB_SENSITIVITY = 0.0015f;
#endif

using namespace rack;

extern Plugin *pluginInstance;

struct ZZC_BaseKnob : SvgKnob {
  ZZC_BaseKnob() {
    minAngle = -0.75 * M_PI;
    maxAngle = 0.75 * M_PI;
  }
};

struct ZZC_BigKnob : ZZC_BaseKnob {
  ZZC_BigKnob() {
    setSvg(APP->window->loadSvg(asset::plugin(pluginInstance, "res/knobs/ZZC-Big-Knob.svg")));
    shadow->box.size = Vec(67, 67);
    shadow->box.pos = Vec(0, 6);
    shadow->blurRadius = 15.0f;
    shadow->opacity = 0.7f;
  }
};

struct ZZC_BigKnobSnappy : ZZC_BigKnob {
  ZZC_BigKnobSnappy() {
    snap = true;
    smooth = false;
  }
};

struct ZZC_BigKnobInner : ZZC_BaseKnob {
  ZZC_BigKnobInner() {
    setSvg(APP->window->loadSvg(asset::plugin(pluginInstance, "res/knobs/ZZC-Big-Knob-Inner.svg")));
    shadow->box.size = Vec(33, 33);
    shadow->box.pos = Vec(-3, 1);
    shadow->blurRadius = 15.0f;
    shadow->opacity = 1.0f;
  }
};


struct ZZC_PreciseKnob : ZZC_BaseKnob {
  ZZC_PreciseKnob() {
    setSvg(APP->window->loadSvg(asset::plugin(pluginInstance, "res/knobs/ZZC-Precise-Knob.svg")));
    shadow->box.size = Vec(44, 44);
    shadow->box.pos = Vec(3.5f, 10);
  }
};

struct ZZC_PreciseKnobSnappy : ZZC_PreciseKnob {
  ZZC_PreciseKnobSnappy() {
    snap = true;
    smooth = false;
  }
};

struct ZZC_Knob19 : ZZC_BaseKnob {
  ZZC_Knob19() {
    setSvg( APP->window->loadSvg(asset::plugin(pluginInstance, "res/knobs/ZZC-Knob-19.svg")) );
  }
};

struct ZZC_Knob19NoRand : ZZC_Knob19 {
  ZZC_Knob19NoRand() {
  }
};

struct ZZC_Knob19SnappyNoRand : ZZC_Knob19NoRand {
  ZZC_Knob19SnappyNoRand() {
    snap = true;
    smooth = false;
  }
};

struct ZZC_Knob21 : ZZC_BaseKnob {
  ZZC_Knob21() {
    setSvg( APP->window->loadSvg(asset::plugin(pluginInstance, "res/knobs/ZZC-Knob-21.svg")) );
  }
};

struct ZZC_Knob21Snappy : ZZC_Knob21 {
  ZZC_Knob21Snappy() {
    snap = true;
    smooth = false;
  }
};

struct ZZC_Knob23 : ZZC_BaseKnob {
  ZZC_Knob23() {
    setSvg( APP->window->loadSvg(asset::plugin(pluginInstance, "res/knobs/ZZC-Knob-23.svg")) );
  }
};

struct ZZC_Knob25 : ZZC_BaseKnob {
  ZZC_Knob25() {
    setSvg( APP->window->loadSvg(asset::plugin(pluginInstance, "res/knobs/ZZC-Knob-25.svg")) );
    shadow->box.size = Vec(29, 29);
    shadow->box.pos = Vec(-2, 2);
    shadow->blurRadius = 15.0f;
    shadow->opacity = 1.0f;
  }
};

struct ZZC_Knob25NoRand : ZZC_Knob25 {
  ZZC_Knob25NoRand() {
  }
};

struct ZZC_Knob25SnappyNoRand : ZZC_Knob25 {
  ZZC_Knob25SnappyNoRand() {
    snap = true;
    smooth = false;
  }
};

struct ZZC_Knob27 : ZZC_BaseKnob {
  ZZC_Knob27() {
    setSvg( APP->window->loadSvg(asset::plugin(pluginInstance, "res/knobs/ZZC-Knob-27.svg")) );
    shadow->box.size = Vec(33, 33);
    shadow->box.pos = Vec(-3, 2);
    shadow->blurRadius = 15.0f;
    shadow->opacity = 1.0f;
  }
};

struct ZZC_Knob27Snappy : ZZC_Knob27 {
  ZZC_Knob27Snappy() {
    snap = true;
    smooth = false;
  }
};

struct ZZC_Knob27Blind : ZZC_BaseKnob {
  ZZC_Knob27Blind() {
    setSvg( APP->window->loadSvg(asset::plugin(pluginInstance, "res/knobs/ZZC-Knob-27-Encoder.svg")) );
    speed = 0.1f;
    minAngle = -5.f * M_PI;
    maxAngle = 5.f * M_PI;
    snap = true;
    shadow->box.size = Vec(33, 33);
    shadow->box.pos = Vec(-3, 2);
    shadow->blurRadius = 15.0f;
    shadow->opacity = 1.0f;
  }
};

struct ZZC_CrossKnob : ZZC_BaseKnob {
  ZZC_CrossKnob() {
    setSvg( APP->window->loadSvg(asset::plugin(pluginInstance, "res/knobs/ZZC-Cross-Knob.svg")) );
    shadow->box.size = Vec(43, 43);
    shadow->box.pos = Vec(3, 8);
    shadow->blurRadius = 15.0f;
    shadow->opacity = 1.0f;
  }
};

struct ZZC_CrossKnob29 : ZZC_BaseKnob {
  ZZC_CrossKnob29() {
    setSvg( APP->window->loadSvg(asset::plugin(pluginInstance, "res/knobs/ZZC-Cross-Knob-29.svg")) );
    shadow->box.size = Vec(31, 31);
    shadow->box.pos = Vec(5, 10);
    shadow->blurRadius = 15.0f;
    shadow->opacity = 1.0f;
  }
};

struct ZZC_CrossKnob45 : ZZC_BaseKnob {
  ZZC_CrossKnob45() {
    setSvg( APP->window->loadSvg(asset::plugin(pluginInstance, "res/knobs/ZZC-Cross-Knob-45.svg")) );
    shadow->box.size = Vec(49, 49);
    shadow->box.pos = Vec(5, 12);
    shadow->blurRadius = 21.0f;
    shadow->opacity = 1.0f;
  }
};

struct ZZC_CrossKnobSnappy : ZZC_CrossKnob {
  ZZC_CrossKnobSnappy() {
    snap = true;
    smooth = false;
  }
};

struct ZZC_SelectKnob : ZZC_BaseKnob {
  ZZC_SelectKnob() {
    setSvg( APP->window->loadSvg(asset::plugin(pluginInstance, "res/knobs/ZZC-Select-Knob.svg")) );
    shadow->box.size = Vec(33, 33);
    shadow->box.pos = Vec(-3, 2);
    shadow->blurRadius = 15.0f;
    shadow->opacity = 1.0f;
    snap = true;
    smooth = false;
  }
};

struct ZZC_SteppedKnob : ZZC_BaseKnob {
  ZZC_SteppedKnob() {
    snap = true;
    smooth = false;
    setSvg( APP->window->loadSvg(asset::plugin(pluginInstance, "res/knobs/ZZC-Stepped-Knob.svg")) );
    shadow->box.size = Vec(25, 25);
    shadow->box.pos = Vec(3, 5);
  }
};

struct ZZC_EncoderKnob : SvgKnob {
  float lastValue = 0.0f;
  float targetValue = 0.0f;

  ZZC_EncoderKnob() {
    minAngle = -1.0 * M_PI;
    maxAngle = 1.0 * M_PI;
    smooth = false;
    setSvg( APP->window->loadSvg(asset::plugin(pluginInstance, "res/knobs/ZZC-Encoder-Knob.svg")) );
    shadow->box.size = Vec(49, 49);
    shadow->box.pos = Vec(6, 12);
    shadow->blurRadius = 15.0f;
    shadow->opacity = 0.8f;
  }

  void onDragMove(const event::DragMove &e) override {
    engine::ParamQuantity* paramQuantity = this->getParamQuantity();
    if (paramQuantity) {
      float range = paramQuantity->getRange();
      float delta = KNOB_SENSITIVITY * -e.mouseDelta.y * speed * range;
      int mods = APP->window->getMods();
      if ((mods & RACK_MOD_MASK) == RACK_MOD_CTRL) {
        delta /= 16.f;
      }

      float newValue = paramQuantity->getSmoothValue();
      newValue += delta;
      paramQuantity->setValue(eucMod(newValue, paramQuantity->getMaxValue()));
    }
  }
};

struct ZZC_KnobWithDot19 : ZZC_BaseKnob {
  ZZC_KnobWithDot19() {
    setSvg( APP->window->loadSvg(asset::plugin(pluginInstance, "res/knobs/ZZC-Knob-With-Dot-19.svg")) );
    shadow->box.size = Vec(25, 25);
    shadow->box.pos = Vec(-3, 2);
    shadow->blurRadius = 15.0f;
    shadow->opacity = 1.0f;
  }
};
