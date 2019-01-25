#include "rack.hpp"


using namespace rack;

// Forward-declare the Plugin, defined in Template.cpp
extern Plugin *plugin;

// Forward-declare each Model, defined in each module source file
extern Model *modelSCVCA;
extern Model *modelSH8;
extern Model *modelClock;
extern Model *modelDivider;
extern Model *modelFN3;
extern Model *modelSRC;

struct ZZC_PJ301MPort : SVGPort {
	ZZC_PJ301MPort() {
		setSVG(SVG::load(assetPlugin(plugin, "res/ZZC-PJ301M.svg")));
	}
};

struct ZZC_PJ301MIPort : SVGPort {
	ZZC_PJ301MIPort() {
		setSVG(SVG::load(assetPlugin(plugin, "res/ZZC-PJ301MI.svg")));
	}
};

struct ZZC_BigKnob : SVGKnob {
	ZZC_BigKnob() {
		minAngle = -0.83 * M_PI;
		maxAngle = 0.83 * M_PI;
		setSVG( SVG::load(assetPlugin(plugin, "res/ZZC-BigKnob.svg")) );
		shadow->box.size = Vec(55, 55);
		shadow->box.pos = Vec(2, 10);
	}
};

struct ZZC_PreciseKnob : SVGKnob {
	ZZC_PreciseKnob() {
		minAngle = -0.83 * M_PI;
		maxAngle = 0.83 * M_PI;
		setSVG( SVG::load(assetPlugin(plugin, "res/ZZC-PreciseKnob.svg")) );
		shadow->box.size = Vec(44, 44);
		shadow->box.pos = Vec(3.5f, 10);
	}
};

struct ZZC_PreciseSnapKnob : ZZC_PreciseKnob {
	ZZC_PreciseSnapKnob() {
		snap = true;
		smooth = false;
	}
};

struct ZZC_SmallKnob : SVGKnob {
	ZZC_SmallKnob() {
		minAngle = -0.83 * M_PI;
		maxAngle = 0.83 * M_PI;
		setSVG( SVG::load(assetPlugin(plugin, "res/ZZC-SmallKnob.svg")) );
	}
};

struct ZZC_SmallKnobInner : SVGKnob {
	ZZC_SmallKnobInner() {
		minAngle = -0.83 * M_PI;
		maxAngle = 0.83 * M_PI;
		setSVG( SVG::load(assetPlugin(plugin, "res/ZZC-SmallKnobInner.svg")) );
		shadow->opacity = 0.0;
	}
};

struct ZZC_ToothKnob : SVGKnob {
	ZZC_ToothKnob() {
		minAngle = -0.83 * M_PI;
		maxAngle = 0.83 * M_PI;
		setSVG( SVG::load(assetPlugin(plugin, "res/ZZC-ToothKnob.svg")) );
	}
};

struct ZZC_ToothSnapKnob : ZZC_ToothKnob {
	ZZC_ToothSnapKnob() {
		snap = true;
		smooth = false;
	}
};

struct ZZC_ToothFastKnob : ZZC_ToothKnob {
	ZZC_ToothFastKnob() {
		smooth = false;
	}
};

struct ZZC_Tooth5Knob : SVGKnob {
	ZZC_Tooth5Knob() {
		minAngle = -0.83 * M_PI;
		maxAngle = 0.83 * M_PI;
		setSVG( SVG::load(assetPlugin(plugin, "res/ZZC-Tooth5Knob.svg")) );
	}
};

struct ZZC_Tooth5SnapKnob : ZZC_Tooth5Knob {
	ZZC_Tooth5SnapKnob() {
		snap = true;
		smooth = false;
	}
};

struct ZZC_Tooth3Knob : SVGKnob {
	ZZC_Tooth3Knob() {
		minAngle = -0.83 * M_PI;
		maxAngle = 0.83 * M_PI;
		setSVG( SVG::load(assetPlugin(plugin, "res/ZZC-Tooth3Knob.svg")) );
	}
};

struct ZZC_Tooth3SnapKnob : ZZC_Tooth3Knob {
	ZZC_Tooth3SnapKnob() {
		snap = true;
		smooth = false;
	}
};

struct ZZC_CKSSTwo : SVGSwitch, ToggleSwitch {
	ZZC_CKSSTwo() {
		addFrame(SVG::load(assetPlugin(plugin,"res/ZZC-CKSSTwo_0.svg")));
		addFrame(SVG::load(assetPlugin(plugin,"res/ZZC-CKSSTwo_1.svg")));
	}
};

struct ZZC_CKSSThree : SVGSwitch, ToggleSwitch {
	ZZC_CKSSThree() {
		addFrame(SVG::load(assetPlugin(plugin,"res/ZZC-CKSSThree_0.svg")));
		addFrame(SVG::load(assetPlugin(plugin,"res/ZZC-CKSSThree_1.svg")));
		addFrame(SVG::load(assetPlugin(plugin,"res/ZZC-CKSSThree_2.svg")));
	}
};

struct ZZC_FN3_Wave : SVGSwitch, ToggleSwitch {
	ZZC_FN3_Wave() {
		addFrame(SVG::load(assetPlugin(plugin,"res/ZZC-FN-3-Wave_0.svg")));
		addFrame(SVG::load(assetPlugin(plugin,"res/ZZC-FN-3-Wave_1.svg")));
		addFrame(SVG::load(assetPlugin(plugin,"res/ZZC-FN-3-Wave_2.svg")));
	}
};

struct ZZC_FN3_UniBi : SVGSwitch, ToggleSwitch {
	ZZC_FN3_UniBi() {
		addFrame(SVG::load(assetPlugin(plugin,"res/ZZC-FN-3-UniBi_0.svg")));
		addFrame(SVG::load(assetPlugin(plugin,"res/ZZC-FN-3-UniBi_1.svg")));
	}
};