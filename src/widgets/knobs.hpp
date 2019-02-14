#include "rack.hpp"

static const float KNOB_SENSITIVITY = 0.0015f;

using namespace rack;

extern Plugin *plugin;

struct ZZC_BaseKnob : SVGKnob {
  ZZC_BaseKnob() {
    minAngle = -0.83 * M_PI;
    maxAngle = 0.83 * M_PI;
  }
};

struct ZZC_BigKnob : ZZC_BaseKnob {
  ZZC_BigKnob() {
    setSVG(SVG::load(assetPlugin(plugin, "res/knobs/ZZC-Big-Knob.svg")));
    shadow->box.size = Vec(55, 55);
    shadow->box.pos = Vec(2, 10);
  }
};

struct ZZC_BigKnobInner : ZZC_BaseKnob {
  ZZC_BigKnobInner() {
    setSVG(SVG::load(assetPlugin(plugin, "res/knobs/ZZC-Big-Knob-Inner.svg")));
    shadow->opacity = 0.0;
  }
};


struct ZZC_PreciseKnob : ZZC_BaseKnob {
  ZZC_PreciseKnob() {
    setSVG(SVG::load(assetPlugin(plugin, "res/knobs/ZZC-Precise-Knob.svg")));
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
    setSVG( SVG::load(assetPlugin(plugin, "res/knobs/ZZC-Knob-19.svg")) );
  }
};

struct ZZC_Knob21 : ZZC_BaseKnob {
  ZZC_Knob21() {
    setSVG( SVG::load(assetPlugin(plugin, "res/knobs/ZZC-Knob-21.svg")) );
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
    setSVG( SVG::load(assetPlugin(plugin, "res/knobs/ZZC-Knob-23.svg")) );
  }
};

struct ZZC_SteppedKnob : ZZC_BaseKnob {
  ZZC_SteppedKnob() {
    snap = true;
    smooth = false;
    setSVG( SVG::load(assetPlugin(plugin, "res/knobs/ZZC-Stepped-Knob.svg")) );
    shadow->box.size = Vec(25, 25);
    shadow->box.pos = Vec(3, 5);
  }
};

struct ZZC_EncoderKnob : SVGKnob {
  float lastValue = 0.0f;
  float targetValue = 0.0f;

  ZZC_EncoderKnob() {
    minAngle = -1.0 * M_PI;
    maxAngle = 1.0 * M_PI;
    smooth = false;
    setSVG( SVG::load(assetPlugin(plugin, "res/knobs/ZZC-Encoder-Knob.svg")) );
  }

  void randomize() override {}

  void onDragMove(EventDragMove &e) override {
    float range = maxValue - minValue;
    float delta = KNOB_SENSITIVITY * -e.mouseRel.y * speed * range;

    if (windowIsModPressed()) {
      delta /= 16.f;
    }

    dragValue += delta;
    setValue(eucmod(dragValue, maxValue));
  }
};
