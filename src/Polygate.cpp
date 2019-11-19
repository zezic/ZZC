#include "ZZC.hpp"

const int NUM_CHANNELS = 16;

struct SmartTrigger {
  bool state = false;
  float holdTime = 0.f;

  void init() {
    this->state = false;
    this->holdTime = 0.f;
  }

  void process(float value, float sampleTime) {
    if (value > 0.5f) {
      if (this->holdTime == 0.f) {
        this->state ^= true;
      }
      this->holdTime += sampleTime;
    } else {
      if (this->holdTime > 0.00136f) { // ~ 0.35s / 256
        this->state ^= true;
      }
      this->holdTime = 0.f;
    }
  }
};

enum PolygateModes {
  SMART_MODE,
  MOMENTARY_MODE,
  NUM_MODES
};

struct Polygate : Module {
  enum ParamIds {
    ENUMS(GATE_PARAM, NUM_CHANNELS),
    NUM_PARAMS
  };
  enum InputIds {
    NUM_INPUTS
  };
  enum OutputIds {
    GATE_OUTPUT,
    NUM_OUTPUTS
  };
  enum LightIds {
    ENUMS(GATE_LED, NUM_CHANNELS),
    NUM_LIGHTS
  };

  SmartTrigger triggers[NUM_CHANNELS];
  dsp::ClockDivider paramDivider;

  /* Settings */
  float range = 5.f;
  bool invertOutput = false;
  PolygateModes mode = SMART_MODE;
  PolygateModes lastMode = SMART_MODE;

  Polygate() {
    config(NUM_PARAMS, NUM_INPUTS, NUM_OUTPUTS, NUM_LIGHTS);
    for (int i = 0; i < NUM_CHANNELS; i++) {
      configParam(GATE_PARAM + i, 0.0f, 1.0f, 0.0f, "Channel " + std::to_string(i + 1));
    }
    paramDivider.setDivision(256);
  }
  void process(const ProcessArgs &args) override;

  void onReset() override {
    for (int i = 0; i < NUM_CHANNELS; i++) {
      this->triggers[i].state = false;
    }
  }

  void onRandomize() override {
    for (int i = 0; i < NUM_CHANNELS; i++) {
      this->triggers[i].state = random::normal() > 0.5 ? true : false;
    }
  }

  json_t *dataToJson() override {
    json_t *rootJ = json_object();
    json_t *gatesArray = json_array();
    for (int i = 0; i < NUM_CHANNELS; i++) {
      json_array_append(gatesArray, json_boolean(this->triggers[i].state));
    }
    json_object_set_new(rootJ, "gates", gatesArray);
    json_object_set_new(rootJ, "range", json_real(range));
    json_object_set_new(rootJ, "invertOutput", json_boolean(invertOutput));
    json_object_set_new(rootJ, "mode", json_integer(mode));
    return rootJ;
  }

  void dataFromJson(json_t *rootJ) override {
    json_t *gatesArray = json_object_get(rootJ, "gates");
    json_t *rangeJ = json_object_get(rootJ, "range");
    json_t *invertOutputJ = json_object_get(rootJ, "invertOutput");
    json_t *modeJ = json_object_get(rootJ, "mode");
    if (gatesArray) {
      size_t size = json_array_size(gatesArray);
      for (size_t i = 0; i < size; i++) {
        triggers[i].state = json_boolean_value(json_array_get(gatesArray, i));
      }
    }
    if (rangeJ) { range = json_real_value(rangeJ); }
    if (invertOutputJ) { invertOutput = json_boolean_value(invertOutputJ); }
    if (modeJ) {
      mode = PolygateModes(json_integer_value(modeJ));
      lastMode = mode;
    }
  }
};

void Polygate::process(const ProcessArgs &args) {
  outputs[GATE_OUTPUT].setChannels(NUM_CHANNELS);

  bool processParams = paramDivider.process();
  if (this->mode == SMART_MODE) {
    if (this->lastMode == MOMENTARY_MODE) {
      for (int c = 0; c < NUM_CHANNELS; c++) {
        triggers[c].init();
      }
    }
    if (processParams) {
      for (int c = 0; c < NUM_CHANNELS; c++) {
          triggers[c].process(params[GATE_PARAM + c].getValue(), args.sampleTime);
        bool state = triggers[c].state;
        outputs[GATE_OUTPUT].setVoltage(state ^ this->invertOutput ? range : 0.f, c);
        if (state) {
          lights[GATE_LED + c].value = 1.1f;
        }
      }
    }
  } else if (this->mode == MOMENTARY_MODE) {
    for (int c = 0; c < NUM_CHANNELS; c++) {
      if (processParams) {
        bool pressed = params[GATE_PARAM + c].getValue() > 0.5f;
        outputs[GATE_OUTPUT].setVoltage(pressed ^ this->invertOutput ? range : 0.f, c);
        if (pressed) {
          lights[GATE_LED + c].value = 1.1f;
        }
      }
    }
  } else {
    // Should not happen
  }
  this->lastMode = this->mode;
}

struct PolygateWidget : ModuleWidget {
  PolygateWidget(Polygate *module) {
    setModule(module);
    setPanel(APP->window->loadSvg(asset::plugin(pluginInstance, "res/panels/Polygate.svg")));

    float groupX[2] = { 10.f, 42.f };
    float groupY[2] = { 53.f, 181.f };
    float yStep = 27.f;

    for (int c = 0; c < NUM_CHANNELS; c++) {
      float x = groupX[c / 8];
      float y = groupY[c % 8 / 4] + yStep * (c % 4);
      addParam(createParam<ZZC_LEDBezelDark>(Vec(x + 0.3f, y + 0.0f), module, Polygate::GATE_PARAM + c));
      addChild(createLight<LedLight<ZZC_YellowLight>>(Vec(x + 2.1f, y + 1.7f), module, Polygate::GATE_LED + c));
    }

    addOutput(createOutput<ZZC_PJ_Port>(Vec(25, 320), module, Polygate::GATE_OUTPUT));

    addChild(createWidget<ZZC_Screw>(Vec(RACK_GRID_WIDTH, 0)));
    addChild(createWidget<ZZC_Screw>(Vec(box.size.x - 2 * RACK_GRID_WIDTH, 0)));
    addChild(createWidget<ZZC_Screw>(Vec(RACK_GRID_WIDTH, RACK_GRID_HEIGHT - RACK_GRID_WIDTH)));
    addChild(createWidget<ZZC_Screw>(Vec(box.size.x - 2 * RACK_GRID_WIDTH, RACK_GRID_HEIGHT - RACK_GRID_WIDTH)));
  }
  void appendContextMenu(Menu *menu) override;
};

struct PolygateInvertOutputItem : MenuItem {
  Polygate *polygate;
  void onAction(const event::Action &e) override {
    polygate->invertOutput ^= true;
  }
  void step() override {
    rightText = CHECKMARK(polygate->invertOutput);
  }
};

struct PolygateRangeItem : MenuItem {
  Polygate *polygate;
  float target;
  void onAction(const event::Action &e) override {
    polygate->range = this->target;
  }
  void step() override {
    rightText = CHECKMARK(polygate->range == this->target);
  }
};

struct PolygateModeItem : MenuItem {
  Polygate *polygate;
  PolygateModes target;
  void onAction(const event::Action &e) override {
    polygate->mode = this->target;
  }
  void step() override {
    rightText = CHECKMARK(polygate->mode == this->target);
  }
};

void PolygateWidget::appendContextMenu(Menu *menu) {
  menu->addChild(new MenuSeparator());

  Polygate *polygate = dynamic_cast<Polygate*>(module);
  assert(polygate);

  PolygateInvertOutputItem *invertOutputItem = createMenuItem<PolygateInvertOutputItem>("Invert Output");
  invertOutputItem->polygate = polygate;
  menu->addChild(invertOutputItem);
  menu->addChild(new MenuSeparator());

  menu->addChild(construct<MenuLabel>(&MenuLabel::text, "Range"));
  std::vector<float> polygateRanges = {
    5.f,
    10.f
  };
  for (float range : polygateRanges) {
    PolygateRangeItem *item = new PolygateRangeItem;
    item->text = "+" + std::to_string((int) range) + "V";
    item->target = range;
    item->polygate = polygate;
    menu->addChild(item);
  }
  menu->addChild(new MenuSeparator());

  menu->addChild(construct<MenuLabel>(&MenuLabel::text, "Button Mode"));
  std::vector<std::string> polygateModeNames = {
    "Smart",
    "Momentary (Hold)"
  };
  for (unsigned int i = 0; i < polygateModeNames.size(); i++) {
    PolygateModeItem *item = new PolygateModeItem;
    item->text = polygateModeNames.at(i);
    item->target = PolygateModes(i);
    item->polygate = polygate;
    menu->addChild(item);
  }
}

Model *modelPolygate = createModel<Polygate, PolygateWidget>("Polygate");
