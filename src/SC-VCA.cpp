#include "ZZC.hpp"


struct SCVCA : Module {
	enum ParamIds {
		GAIN_PARAM,
		CLIP_PARAM,
		CLIP_MODE_PARAM,
		CLIP_SOFTNESS_PARAM,
		NUM_PARAMS
	};
	enum InputIds {
		CV_INPUT,
		SIG1_INPUT,
		SIG2_INPUT,
		GAIN_INPUT,
		CLIP_INPUT,
		CLIP_SOFTNESS_INPUT,
		NUM_INPUTS
	};
	enum OutputIds {
		SIG1_OUTPUT,
		SIG2_OUTPUT,
		NUM_OUTPUTS
	};
	enum LightIds {
		CLIPPING_POS_LIGHT,
		CLIPPING_NEG_LIGHT,
		NUM_LIGHTS
	};

	float SoftLimit(float x) {
		return x * (27.0f + x * x) / (27.0f + 9.0f * x * x);
	}

	float SoftClip(float x) {
		return x > 3.0f ? 1.0f : SoftLimit(x);
	}

	void SoftClipTo(float x, float ceiling, float softness, Output &out) {
		float ws_range = ceiling * softness;
		float clean_range = ceiling - ws_range;
		float abso = fabsf(x);

		if (abso > clean_range) {
			out.value = copysignf(clean_range + SoftClip((abso - clean_range) / ws_range) * ws_range, x);
		} else {
			out.value = x;
		}
	}

	void HardClipTo(float x, float ceiling, Output &out) {
		out.value = clamp(x, -ceiling, ceiling);
	}

	void ClipItMaybe(float x, float ceiling, float clip_mode, float clip_softness, Output &out) {
		if (clip_mode == 0.0f) {
			SoftClipTo(x, ceiling, clip_softness, out);
		} else {
			HardClipTo(x, ceiling, out);
		}
	}

	SCVCA() : Module(NUM_PARAMS, NUM_INPUTS, NUM_OUTPUTS, NUM_LIGHTS) {}
	void step() override;
};


void SCVCA::step() {
	float gain = params[GAIN_PARAM].value;
	float clip = params[CLIP_PARAM].value;
	float softness = params[CLIP_SOFTNESS_PARAM].value;

	if (inputs[GAIN_INPUT].active) { gain = gain * inputs[GAIN_INPUT].value / 10.0f; }
	if (inputs[CLIP_INPUT].active) { clip = clip * clamp(inputs[CLIP_INPUT].value, 0.0f, 10.0f) / 10.0f; }
	if (inputs[CLIP_SOFTNESS_INPUT].active) { softness = softness * clamp(inputs[CLIP_SOFTNESS_INPUT].value, 0.0f, 10.0f) / 10.0f; }

	float gained_1 = inputs[SIG1_INPUT].value * gain;
	float gained_2 = inputs[SIG2_INPUT].value * gain;
	ClipItMaybe(gained_1, clip, params[CLIP_MODE_PARAM].value, softness, outputs[SIG1_OUTPUT]);
	ClipItMaybe(gained_2, clip, params[CLIP_MODE_PARAM].value, softness, outputs[SIG2_OUTPUT]);

	lights[CLIPPING_POS_LIGHT].setBrightnessSmooth(fmaxf(
		fminf(1.0f, gained_1 - outputs[SIG1_OUTPUT].value),
		fminf(1.0f, gained_2 - outputs[SIG2_OUTPUT].value)
	));
	lights[CLIPPING_NEG_LIGHT].setBrightnessSmooth(fmaxf(
		fmaxf(-1.0f, -(gained_1 - outputs[SIG1_OUTPUT].value)),
		fmaxf(-1.0f, -(gained_2 - outputs[SIG2_OUTPUT].value))
	));
}


struct SCVCAWidget : ModuleWidget {
	SCVCAWidget(SCVCA *module) : ModuleWidget(module) {
		setPanel(SVG::load(assetPlugin(plugin, "res/SC-VCA.svg")));

		addChild(Widget::create<ScrewBlack>(Vec(RACK_GRID_WIDTH, 0)));
		addChild(Widget::create<ScrewBlack>(Vec(box.size.x - 2 * RACK_GRID_WIDTH, 0)));
		addChild(Widget::create<ScrewBlack>(Vec(RACK_GRID_WIDTH, RACK_GRID_HEIGHT - RACK_GRID_WIDTH)));
		addChild(Widget::create<ScrewBlack>(Vec(box.size.x - 2 * RACK_GRID_WIDTH, RACK_GRID_HEIGHT - RACK_GRID_WIDTH)));

		addParam(ParamWidget::create<ZZC_BigKnob>(Vec(8, 60), module, SCVCA::GAIN_PARAM, 0.0f, 2.0f, 1.0f));
		addParam(ParamWidget::create<ZZC_SmallKnobInner>(Vec(24, 76), module, SCVCA::CLIP_PARAM, 0.0f, 10.0f, 5.0f));
		addParam(ParamWidget::create<ZZC_CKSSTwo>(Vec(27, 187), module, SCVCA::CLIP_MODE_PARAM, 0.0f, 1.0f, 0.0f));
		addParam(ParamWidget::create<ZZC_SmallKnob>(Vec(42, 222), module, SCVCA::CLIP_SOFTNESS_PARAM, 0.0f, 1.0f, 0.5f));

		addInput(Port::create<ZZC_PJ301MPort>(Vec(8, 143), Port::INPUT, module, SCVCA::GAIN_INPUT));
		addInput(Port::create<ZZC_PJ301MPort>(Vec(44, 143), Port::INPUT, module, SCVCA::CLIP_INPUT));
		addInput(Port::create<ZZC_PJ301MPort>(Vec(8, 222), Port::INPUT, module, SCVCA::CLIP_SOFTNESS_INPUT));

		addInput(Port::create<ZZC_PJ301MPort>(Vec(8, 275), Port::INPUT, module, SCVCA::SIG1_INPUT));
		addOutput(Port::create<ZZC_PJ301MIPort>(Vec(8, 319), Port::OUTPUT, module, SCVCA::SIG1_OUTPUT));
		addInput(Port::create<ZZC_PJ301MPort>(Vec(42, 275), Port::INPUT, module, SCVCA::SIG2_INPUT));
		addOutput(Port::create<ZZC_PJ301MIPort>(Vec(42, 319), Port::OUTPUT, module, SCVCA::SIG2_OUTPUT));

		addChild(ModuleLightWidget::create<SmallLight<GreenRedLight>>(Vec(56.5f, 42.5f), module, SCVCA::CLIPPING_POS_LIGHT));
	}
};


Model *modelSCVCA = Model::create<SCVCA, SCVCAWidget>("ZZC", "SC-VCA", "Soft Clipping VCA", AMPLIFIER_TAG, LIMITER_TAG, WAVESHAPER_TAG);
