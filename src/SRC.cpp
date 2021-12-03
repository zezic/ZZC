#include "ZZC.hpp"


enum ModeIds {
  MUSICAL_MODE,
  DECIMAL_MODE,
  FREE_MODE,
  NUM_MODES
};


void writeMusicalNotation(char *output, size_t size, float voltage) {
  char notes[20][20] = {"c", "ic", "d", "id", "e", "f", "if", "g", "ig", "a", "ia", "b"};
  int noteIdx = (int)(eucMod(voltage, 1.0f) / (1.0f / 12.05f));
  char *note = notes[noteIdx];
  if (voltage >= 6.0f) {
    snprintf(output, size, "%sh", note);
  } else if (voltage < -4.0f) {
    snprintf(output, size, "%sl", note);
  } else {
    snprintf(output, size, "%s%2.0f", note, std::floor(voltage) + 4);
  }
}

struct VoltageDisplayWidget : BaseDisplayWidget {
  float *value = nullptr;
  int *mode = nullptr;

  void drawLayer(const DrawArgs &args, int layer) override {
    if (layer != 1) { return; }

    std::shared_ptr<Font> font = APP->window->loadFont(asset::plugin(pluginInstance, "res/fonts/DSEG/DSEG7ClassicMini-Italic.ttf"));
    if (!font) { return; }

    // Text (integer part)
    nvgFontSize(args.vg, 11);
    nvgFontFaceId(args.vg, font->handle);
    nvgTextLetterSpacing(args.vg, 1.0);
    nvgTextAlign(args.vg, NVG_ALIGN_RIGHT);

    char text[10];
    size_t size = sizeof(text);
    if (mode) {
      if (*mode == MUSICAL_MODE) {
        writeMusicalNotation(text, size, *value);
      } else {
        snprintf(text, size, "%2.1f", fabsf(*value));
      }
    } else {
      snprintf(text, size, "c4");
    }

    Vec textPos = Vec(box.size.x - 5.0f, 16.0f);

    nvgFillColor(args.vg, lcdGhostColor);
    if (mode) {
      nvgText(args.vg, textPos.x, textPos.y, *mode == MUSICAL_MODE ? "188" : "18.8", NULL);
    } else {
      nvgText(args.vg, textPos.x, textPos.y, "188", NULL);
    }
    nvgFillColor(args.vg, lcdTextColor);
    nvgText(args.vg, textPos.x, textPos.y, text, NULL);

    nvgGlobalCompositeBlendFunc(args.vg, NVG_ONE_MINUS_DST_COLOR, NVG_ONE);
    drawHalo(args);
  }
};


struct SRC : Module {
  enum ParamIds {
    COARSE_PARAM,
    FINE_PARAM,
    ON_SWITCH_PARAM,
    NUM_PARAMS
  };
  enum InputIds {
    CV_INPUT,
    ON_INPUT,
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

  dsp::SchmittTrigger onButtonTrigger;
  dsp::SchmittTrigger externalOnTrigger;

  bool on = true;
  int mode = MUSICAL_MODE;
  bool quantizeInput = false;
  bool onHold = false;
  float voltage = 0.0f;

  void processButtons() {
    if (onHold) {
      on = (bool)params[ON_SWITCH_PARAM].getValue() ^ (bool)inputs[ON_INPUT].getVoltage();
    } else if (onButtonTrigger.process(params[ON_SWITCH_PARAM].getValue() || externalOnTrigger.process(inputs[ON_INPUT].getVoltage()))) {
      on = !on;
    }
  }

  float quantize(float voltage) {
    if (mode == MUSICAL_MODE) {
      return roundf(voltage * 12.0f) / 12.0f;
    } else if (mode == DECIMAL_MODE) {
      return roundf(voltage * 10.0f) / 10.0f;
    }
    return voltage;
  }

  SRC() {
    config(NUM_PARAMS, NUM_INPUTS, NUM_OUTPUTS, NUM_LIGHTS);
    configParam(COARSE_PARAM, -10.0f, 10.0f, 0.0f, "Coarse Tuning");
    configParam(FINE_PARAM, -1.0f, 1.0f, 0.0f, "Fine Tuning");
    configParam(ON_SWITCH_PARAM, 0.0f, 1.0f, 0.0f, "On/Off");
  }
  void process(const ProcessArgs &args) override;

  json_t *dataToJson() override {
    json_t *rootJ = json_object();
    json_object_set_new(rootJ, "on", json_boolean(on));
    json_object_set_new(rootJ, "mode", json_integer(mode));
    json_object_set_new(rootJ, "quantizeInput", json_boolean(quantizeInput));
    json_object_set_new(rootJ, "onHold", json_boolean(onHold));
    return rootJ;
  }

  void dataFromJson(json_t *rootJ) override {
    json_t *onJ = json_object_get(rootJ, "on");
    json_t *modeJ = json_object_get(rootJ, "mode");
    json_t *quantizeInputJ = json_object_get(rootJ, "quantizeInput");
    json_t *onHoldJ = json_object_get(rootJ, "onHold");
    if (onJ) { on = json_boolean_value(onJ); }
    if (modeJ) { mode = json_integer_value(modeJ); }
    if (quantizeInputJ) { quantizeInput = json_boolean_value(quantizeInputJ); }
    if (onHoldJ) { onHold = json_boolean_value(onHoldJ); }
  }
};


void SRC::process(const ProcessArgs &args) {
  processButtons();
  float coarse = params[COARSE_PARAM].getValue();
  float fine = quantize(params[FINE_PARAM].getValue());
  voltage = clamp(coarse + fine + (inputs[CV_INPUT].isConnected() ? (quantizeInput ? quantize(inputs[CV_INPUT].getVoltage()) : inputs[CV_INPUT].getVoltage()) : 0.0f), -11.0f, 11.0f);

  if (outputs[VOLTAGE_OUTPUT].isConnected()) {
    outputs[VOLTAGE_OUTPUT].setVoltage(on ? voltage : 0.0f);
  }
  lights[VOLTAGE_POS_LIGHT].setBrightness(fmaxf(0.0f, voltage / 11.0f));
  lights[VOLTAGE_NEG_LIGHT].setBrightness(fmaxf(0.0f, voltage / -11.0f));
  if (on) {
    lights[ON_LED].value = 1.1f;
  }
}


struct SRCWidget : ModuleWidget {
  SRCWidget(SRC *module);
  void appendContextMenu(Menu *menu) override;
};

SRCWidget::SRCWidget(SRC *module) {
  setModule(module);
  setPanel(APP->window->loadSvg(asset::plugin(pluginInstance, "res/panels/SRC.svg")));

  addChild(createLight<SmallLight<GreenRedLight>>(Vec(25.0f, 42.5f), module, SRC::VOLTAGE_POS_LIGHT));

  VoltageDisplayWidget *display = new VoltageDisplayWidget();
  display->box.pos = Vec(6.0f, 60.0f);
  display->box.size = Vec(33.0f, 21.0f);
  if (module) {
    display->value = &module->voltage;
    display->mode = &module->mode;
  }
  addChild(display);

  addParam(createParam<ZZC_SelectKnob>(Vec(9, 105), module, SRC::COARSE_PARAM));
  addParam(createParam<ZZC_Knob25>(Vec(10, 156), module, SRC::FINE_PARAM));

  addInput(createInput<ZZC_PJ_Port>(Vec(10.5, 200), module, SRC::CV_INPUT));
  addInput(createInput<ZZC_PJ_Port>(Vec(10.5, 242), module, SRC::ON_INPUT));

  addParam(createParam<ZZC_LEDBezelDark>(Vec(11.3f, 276.0f), module, SRC::ON_SWITCH_PARAM));
  addChild(createLight<LedLight<ZZC_YellowLight>>(Vec(13.35f, 277.95f), module, SRC::ON_LED));

  addOutput(createOutput<ZZC_PJ_Port>(Vec(10.5, 320), module, SRC::VOLTAGE_OUTPUT));

  addChild(createWidget<ZZC_Screw>(Vec(RACK_GRID_WIDTH, 0)));
  addChild(createWidget<ZZC_Screw>(Vec(RACK_GRID_WIDTH, RACK_GRID_HEIGHT - RACK_GRID_WIDTH)));
}


struct SRCMusicalItem : MenuItem {
  SRC *src;
  void onAction(const event::Action &e) override {
    src->mode = MUSICAL_MODE;
  }
  void step() override {
    rightText = CHECKMARK(src->mode == MUSICAL_MODE);
  }
};
struct SRCDecimalItem : MenuItem {
  SRC *src;
  void onAction(const event::Action &e) override {
    src->mode = DECIMAL_MODE;
  }
  void step() override {
    rightText = CHECKMARK(src->mode == DECIMAL_MODE);
  }
};
struct SRCFreeItem : MenuItem {
  SRC *src;
  void onAction(const event::Action &e) override {
    src->mode = FREE_MODE;
  }
  void step() override {
    rightText = CHECKMARK(src->mode == FREE_MODE);
  }
};
struct SRCOnToggleItem : MenuItem {
  SRC *src;
  void onAction(const event::Action &e) override {
    src->onHold = false;
  }
  void step() override {
    rightText = CHECKMARK(!src->onHold);
  }
};
struct SRCOnHoldItem : MenuItem {
  SRC *src;
  void onAction(const event::Action &e) override {
    src->onHold = true;
  }
  void step() override {
    rightText = CHECKMARK(src->onHold);
  }
};
struct SRCQuantizeItem : MenuItem {
  SRC *src;
  void onAction(const event::Action &e) override {
    src->quantizeInput ^= true;
  }
  void step() override {
    rightText = CHECKMARK(src->quantizeInput);
  }
};

void SRCWidget::appendContextMenu(Menu *menu) {
  menu->addChild(new MenuSeparator());

  SRC *src = dynamic_cast<SRC*>(module);
  assert(src);

  SRCMusicalItem *musicalItem = createMenuItem<SRCMusicalItem>("Fine: Snap to 1/12V");
  SRCDecimalItem *decimalItem = createMenuItem<SRCDecimalItem>("Fine: Snap to 1/10V");
  SRCFreeItem *freeItem = createMenuItem<SRCFreeItem>("Fine: Don't snap");
  SRCOnToggleItem *onToggleItem = createMenuItem<SRCOnToggleItem>("ON: Toggle");
  SRCOnHoldItem *onHoldItem = createMenuItem<SRCOnHoldItem>("ON: Hold");
  SRCQuantizeItem *quantizeItem = createMenuItem<SRCQuantizeItem>("Quantize CV like Fine knob");
  musicalItem->src = src;
  decimalItem->src = src;
  freeItem->src = src;
  onToggleItem->src = src;
  onHoldItem->src = src;
  quantizeItem->src = src;
  menu->addChild(musicalItem);
  menu->addChild(decimalItem);
  menu->addChild(freeItem);
  menu->addChild(new MenuSeparator());
  menu->addChild(onToggleItem);
  menu->addChild(onHoldItem);
  menu->addChild(new MenuSeparator());
  menu->addChild(quantizeItem);
}


Model *modelSRC = createModel<SRC, SRCWidget>("SRC");
