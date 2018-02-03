#include "Template.hpp"


// The plugin-wide instance of the Plugin class
Plugin *plugin;

void init(rack::Plugin *p) {
	plugin = p;
	p->slug = TOSTRING(SLUG);
	p->version = TOSTRING(VERSION);
	p->website = "https://github.com/VCVRack/Template";
	p->manual = "https://github.com/VCVRack/Template/blob/master/README.md";

	// For each module, specify the ModuleWidget subclass, manufacturer slug (for saving in patches), manufacturer human-readable name, module slug, and module name
	p->addModel(createModel<MyModuleWidget>("Template", "MyModule", "My Module", OSCILLATOR_TAG));

	// Any other plugin initialization may go here.
	// As an alternative, consider lazy-loading assets and lookup tables when your module is created to reduce startup times of Rack.
}
