#include "rack.hpp"


using namespace rack;

// Forward-declare the Plugin, defined in Template.cpp
extern Plugin *plugin;

// Forward-declare each Model, defined in each module source file
extern Model *modelSCVCA;
extern Model *modelSH8;

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
