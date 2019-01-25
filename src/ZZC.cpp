#include "ZZC.hpp"


Plugin *plugin;


void init(Plugin *p) {
	plugin = p;
	p->slug = TOSTRING(SLUG);
	p->version = TOSTRING(VERSION);

	// Add all Models defined throughout the plugin
	p->addModel(modelSCVCA);
	p->addModel(modelSH8);
	p->addModel(modelClock);
	p->addModel(modelDivider);
	p->addModel(modelFN3);
	p->addModel(modelSRC);

	// Any other plugin initialization may go here.
	// As an alternative, consider lazy-loading assets and lookup tables when your module is created to reduce startup times of Rack.
}
