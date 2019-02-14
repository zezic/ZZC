#include "rack.hpp"

using namespace rack;

extern Plugin *plugin;

struct ZZC_Screw : SVGScrew {
	ZZC_Screw() {
		sw->setSVG(SVG::load(assetPlugin(plugin, "res/screws/ZZC-Screw.svg")));
		box.size = sw->box.size;
	}
};
