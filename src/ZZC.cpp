#include "ZZC.hpp"

Plugin *pluginInstance;

void init(Plugin *p) {
  pluginInstance = p;

  // Add all Models defined throughout the pluginInstance
  p->addModel(modelClock);
  p->addModel(modelDivider);
  p->addModel(modelFN3);
  p->addModel(modelSCVCA);
  p->addModel(modelSH8);
  p->addModel(modelSRC);
  p->addModel(modelDiv);
  p->addModel(modelDivExp);
  p->addModel(modelPolygate);
  // p->addModel(modelWavetablePlayer);
  // p->addModel(modelPhasor);

  // Any other pluginInstance initialization may go here.
  // As an alternative, consider lazy-loading assets and lookup tables when your module is created to reduce startup times of Rack.
}
