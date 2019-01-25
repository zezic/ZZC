#include "ZZC.hpp"
#include "shared.hpp"
#include "widgets.hpp"


struct SRC : Module {
  enum ParamIds {
    COARSE_PARAM,
    FINE_PARAM,
    ON_SWITCH_PARAM,
    NUM_PARAMS
  };
  enum InputIds {
    COARSE_INPUT,
    FINE_INPUT,
    NUM_INPUTS
  };
  enum OutputIds {
    VOLTAGE_OUTPUT,
    NUM_OUTPUTS
  };
  enum LightIds {
    VOLTAGE_POS_LIGHT,
    VOLTAGE_NEG_LIGHT,
    ON_LED,
    NUM_LIGHTS
  };

  SchmittTrigger onButtonTrigger;

  bool on = true;
	bool musical = false;
  float voltage = 0.0f;

  void processButtons() {
    if (onButtonTrigger.process(params[ON_SWITCH_PARAM].value)) {
      on = !on;
    }
  }

	SRC() : Module(NUM_PARAMS, NUM_INPUTS, NUM_OUTPUTS, NUM_LIGHTS) {}
	void step() override;

  json_t *toJson() override {
    json_t *rootJ = json_object();
    json_object_set_new(rootJ, "on", json_boolean(on));
    json_object_set_new(rootJ, "musical", json_boolean(musical));
    return rootJ;
  }

  void fromJson(json_t *rootJ) override {
    json_t *onJ = json_object_get(rootJ, "on");
    json_t *musicalJ = json_object_get(rootJ, "musical");
    on = json_boolean_value(onJ);
    musical = json_boolean_value(musicalJ);
  }
};


void SRC::step() {
  processButtons();
  float coarse = params[COARSE_PARAM].value;
  if (inputs[COARSE_INPUT].active) {
    coarse = roundf(clamp(coarse + inputs[COARSE_INPUT].value, -10.0f, 10.0f));
  }
  float fine = params[FINE_PARAM].value;
  if (inputs[FINE_INPUT].active) {
    fine = roundf(clamp(fine + inputs[FINE_INPUT].value, -10.0f, 10.0f));
  }
  voltage = coarse + fine / 10.0f;
  
  if (outputs[VOLTAGE_OUTPUT].active) {
    outputs[VOLTAGE_OUTPUT].value = on ? voltage : 0.0f;
  }
	lights[VOLTAGE_POS_LIGHT].setBrightnessSmooth(fmaxf(0.0f, voltage / 11.0f));
	lights[VOLTAGE_NEG_LIGHT].setBrightnessSmooth(fmaxf(0.0f, voltage / -11.0f));
  lights[ON_LED].value = on ? 0.5f : 0.0f;
}


struct SRCWidget : ModuleWidget {
	SRCWidget(SRC *module) : ModuleWidget(module) {
		setPanel(SVG::load(assetPlugin(plugin, "res/SRC.svg")));

		addChild(ModuleLightWidget::create<SmallLight<GreenRedLight>>(Vec(25.0f, 42.5f), module, SRC::VOLTAGE_POS_LIGHT));

    VoltageDisplayWidget *display = new VoltageDisplayWidget();
    display->box.pos = Vec(6.0f, 60.0f);
    display->box.size = Vec(33.0f, 21.0f);
    display->value = &module->voltage;
    addChild(display);

		addParam(ParamWidget::create<ZZC_ToothSnapKnob>(Vec(10, 106), module, SRC::COARSE_PARAM, -10.0f, 10.0f, 0.0f));
    addInput(Port::create<ZZC_PJ301MPort>(Vec(10, 141), Port::INPUT, module, SRC::COARSE_INPUT));

		addParam(ParamWidget::create<ZZC_Tooth5SnapKnob>(Vec(11, 190), module, SRC::FINE_PARAM, -10.0f, 10.0f, 0.0f));
    addInput(Port::create<ZZC_PJ301MPort>(Vec(10, 222), Port::INPUT, module, SRC::FINE_INPUT));

    addParam(ParamWidget::create<LEDBezelDark>(Vec(11.3f, 270.0f), module, SRC::ON_SWITCH_PARAM, 0.0f, 1.0f, 0.0f));
    addChild(ModuleLightWidget::create<LedLight<YellowLight>>(Vec(13.1f, 271.7f), module, SRC::ON_LED));

		addOutput(Port::create<ZZC_PJ301MIPort>(Vec(10, 319), Port::OUTPUT, module, SRC::VOLTAGE_OUTPUT));

		addChild(Widget::create<ScrewBlack>(Vec(RACK_GRID_WIDTH, 0)));
		addChild(Widget::create<ScrewBlack>(Vec(box.size.x - 2 * RACK_GRID_WIDTH, 0)));
		addChild(Widget::create<ScrewBlack>(Vec(RACK_GRID_WIDTH, RACK_GRID_HEIGHT - RACK_GRID_WIDTH)));
		addChild(Widget::create<ScrewBlack>(Vec(box.size.x - 2 * RACK_GRID_WIDTH, RACK_GRID_HEIGHT - RACK_GRID_WIDTH)));
	}
};


Model *modelSRC = Model::create<SRC, SRCWidget>("ZZC", "SRC", "Voltage Source", UTILITY_TAG);
