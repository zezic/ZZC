#include "ZZC.hpp"


struct SH8 : Module {
	enum ParamIds {
    NUM_PARAMS
  };
  enum InputIds {
    TRIG1_INPUT,
    TRIG2_INPUT,
    TRIG3_INPUT,
    TRIG4_INPUT,
    TRIG5_INPUT,
    TRIG6_INPUT,
    TRIG7_INPUT,
    TRIG8_INPUT,
    SIG_INPUT,
    NUM_INPUTS
  };
  enum OutputIds {
    HOLD1_OUTPUT,
    HOLD2_OUTPUT,
    HOLD3_OUTPUT,
    HOLD4_OUTPUT,
    HOLD5_OUTPUT,
    HOLD6_OUTPUT,
    HOLD7_OUTPUT,
    HOLD8_OUTPUT,
    NUM_OUTPUTS
  };
  enum LightIds {
    NUM_LIGHTS
  };

  float last_trig_inputs [8];

	SH8() : Module(NUM_PARAMS, NUM_INPUTS, NUM_OUTPUTS, NUM_LIGHTS) {}
	void step() override;
};


void SH8::step() {
	float last_trig_input = 10.0f;
  float carry = 0.0f;
	for (int i = 0; i < 8; i++) {
		last_trig_input = last_trig_inputs[i];
    if (inputs[i].active) {
      carry = inputs[i].value;
    }
    if (last_trig_input <= 0.0f && carry > 0.0f) {
      outputs[i].value = inputs[SIG_INPUT].value;
    }
    last_trig_inputs[i] = carry;
	}
}


struct SH8Widget : ModuleWidget {
  SH8Widget(SH8 *module) : ModuleWidget(module) {
		setPanel(SVG::load(assetPlugin(plugin, "res/SH-8.svg")));

		addChild(Widget::create<ScrewBlack>(Vec(RACK_GRID_WIDTH, 0)));
		addChild(Widget::create<ScrewBlack>(Vec(box.size.x - 2 * RACK_GRID_WIDTH, 0)));
		addChild(Widget::create<ScrewBlack>(Vec(RACK_GRID_WIDTH, RACK_GRID_HEIGHT - RACK_GRID_WIDTH)));
		addChild(Widget::create<ScrewBlack>(Vec(box.size.x - 2 * RACK_GRID_WIDTH, RACK_GRID_HEIGHT - RACK_GRID_WIDTH)));

		addInput(Port::create<ZZC_PJ301MPort>(Vec(25, 53), Port::INPUT, module, SH8::SIG_INPUT));

    addInput(Port::create<ZZC_PJ301MPort>(Vec(7.25f, 109), Port::INPUT, module, SH8::TRIG1_INPUT));
    addInput(Port::create<ZZC_PJ301MPort>(Vec(7.25f, 139), Port::INPUT, module, SH8::TRIG2_INPUT));
    addInput(Port::create<ZZC_PJ301MPort>(Vec(7.25f, 169), Port::INPUT, module, SH8::TRIG3_INPUT));
    addInput(Port::create<ZZC_PJ301MPort>(Vec(7.25f, 199), Port::INPUT, module, SH8::TRIG4_INPUT));
    addInput(Port::create<ZZC_PJ301MPort>(Vec(7.25f, 229), Port::INPUT, module, SH8::TRIG5_INPUT));
    addInput(Port::create<ZZC_PJ301MPort>(Vec(7.25f, 259), Port::INPUT, module, SH8::TRIG6_INPUT));
    addInput(Port::create<ZZC_PJ301MPort>(Vec(7.25f, 289), Port::INPUT, module, SH8::TRIG7_INPUT));
    addInput(Port::create<ZZC_PJ301MPort>(Vec(7.25f, 319), Port::INPUT, module, SH8::TRIG8_INPUT));

    addOutput(Port::create<ZZC_PJ301MIPort>(Vec(42.25f, 109), Port::OUTPUT, module, SH8::HOLD1_OUTPUT));
    addOutput(Port::create<ZZC_PJ301MIPort>(Vec(42.25f, 139), Port::OUTPUT, module, SH8::HOLD2_OUTPUT));
    addOutput(Port::create<ZZC_PJ301MIPort>(Vec(42.25f, 169), Port::OUTPUT, module, SH8::HOLD3_OUTPUT));
    addOutput(Port::create<ZZC_PJ301MIPort>(Vec(42.25f, 199), Port::OUTPUT, module, SH8::HOLD4_OUTPUT));
    addOutput(Port::create<ZZC_PJ301MIPort>(Vec(42.25f, 229), Port::OUTPUT, module, SH8::HOLD5_OUTPUT));
    addOutput(Port::create<ZZC_PJ301MIPort>(Vec(42.25f, 259), Port::OUTPUT, module, SH8::HOLD6_OUTPUT));
    addOutput(Port::create<ZZC_PJ301MIPort>(Vec(42.25f, 289), Port::OUTPUT, module, SH8::HOLD7_OUTPUT));
    addOutput(Port::create<ZZC_PJ301MIPort>(Vec(42.25f, 319), Port::OUTPUT, module, SH8::HOLD8_OUTPUT));
  }
};


Model *modelSH8 = Model::create<SH8, SH8Widget>("ZZC", "SH-8", "SH-8 Sample & Hold", SAMPLE_AND_HOLD_TAG);
