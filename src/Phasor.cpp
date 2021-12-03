#include "ZZC.hpp"
#include "Phasor.hpp"

Phasor::Phasor() {
  this->rddPtr = std::make_shared<RatioDisplayData>();
  this->rddFeedDivider.setDivision(1000);
  config(NUM_PARAMS, NUM_INPUTS, NUM_OUTPUTS, NUM_LIGHTS);
  configParam(NUMERATOR_PARAM, 1.f, 99.f, 1.f, "Ratio Numerator");
  configParam(DENOMINATOR_PARAM, 1.f, 99.f, 1.f, "Ratio Denominator");
}

json_t *Phasor::dataToJson() {
  json_t *rootJ = json_object();
  return rootJ;
}

void Phasor::dataFromJson(json_t *rootJ) {
}

void Phasor::process(const ProcessArgs &args) {
  if (this->rddFeedDivider.process()) {
    RatioDisplayData* rdd = this->rddPtr.get();
    rdd->numerator = this->params[NUMERATOR_PARAM].getValue();
    rdd->denominator = this->params[DENOMINATOR_PARAM].getValue();
  }
}

struct PhasorWidget : ModuleWidget {
  PhasorWidget(Phasor *module);
  void appendContextMenu(Menu *menu) override;
};

struct VerticalRatioDisplayWidget : BaseDisplayWidget {
  float *value = nullptr;
  int *mode = nullptr;
  NVGcolor lcdGhostColor = nvgRGB(0x1e, 0x1f, 0x1d);
  NVGcolor lcdTextColor = nvgRGB(0xff, 0xd4, 0x2a);
  std::shared_ptr<RatioDisplayData> rddPtr;

  void draw(const DrawArgs &args) override {
    std::shared_ptr<Font> font = APP->window->loadFont(asset::plugin(pluginInstance, "res/fonts/DSEG/DSEG7ClassicMini-Italic.ttf"));
    if (!font) { return; }

    drawBackground(args);

    nvgFontSize(args.vg, 11);
    nvgFontFaceId(args.vg, font->handle);
    nvgTextLetterSpacing(args.vg, 1.0);
    nvgTextAlign(args.vg, NVG_ALIGN_RIGHT);

    Vec textPos0 = Vec(box.size.x - 5.f, 16.0f);
    Vec textPos1 = Vec(box.size.x - 5.f, 36.0f);

    nvgFillColor(args.vg, lcdGhostColor);
    nvgText(args.vg, textPos0.x, textPos0.y, "88", NULL);
    nvgText(args.vg, textPos1.x, textPos1.y, "88", NULL);

    float value0 = rddPtr ? rddPtr.get()->numerator : 1.f;
    float value1 = rddPtr ? rddPtr.get()->denominator : 1.f;

    nvgFillColor(args.vg, lcdTextColor);
    nvgText(args.vg, textPos0.x, textPos0.y, string::f("%d", (int)value0).c_str(), NULL);
    nvgText(args.vg, textPos1.x, textPos1.y, string::f("%d", (int)value1).c_str(), NULL);

    nvgBeginPath(args.vg);
    nvgRect(args.vg, 4.f, this->box.size.y * 0.5f - 0.5f, this->box.size.x - 8.f, 1.f);
    nvgFillColor(args.vg, lcdGhostColor);
    nvgFill(args.vg);

    if (value0 != value1) {
      float delta = (value0 - value1) / 98.f;
      float barLength = std::abs(delta) * 0.5f * (this->box.size.x - 4.f * 2.f);
      nvgBeginPath(args.vg);
      nvgRect(
        args.vg,
        delta > 0.f ? this->box.size.x * 0.5f : this->box.size.x * 0.5f - barLength,
        this->box.size.y * 0.5f - 0.5f,
        barLength,
        1.f
      );
      nvgFillColor(args.vg, lcdTextColor);
      nvgFill(args.vg);
    }
  }
};

PhasorWidget::PhasorWidget(Phasor *module) {
  setModule(module);
  setPanel(APP->window->loadSvg(asset::plugin(pluginInstance, "res/panels/Phasor.svg")));

  VerticalRatioDisplayWidget *display = new VerticalRatioDisplayWidget();
  display->box.pos = Vec(46.f, 131.f);
  display->box.size = Vec(28.f, 41.f);
  if (module) {
    display->rddPtr = module->rddPtr;
  }
  addChild(display);

  addParam(createParam<ZZC_CrossKnob45>(Vec(30.5f, 54.5f), module, Phasor::FREQ_CRSE_PARAM));
  addParam(createParam<ZZC_Switch2Vertical>(Vec(12.f, 71.f), module, Phasor::REVERSE_PARAM));
  addParam(createParam<ZZC_Switch2Vertical>(Vec(93.f, 71.f), module, Phasor::LFO_PARAM));

  addParam(createParam<ZZC_SelectKnob>(Vec(10.f, 138.828f), module, Phasor::NUMERATOR_PARAM));
  addParam(createParam<ZZC_SelectKnob>(Vec(83.f, 138.828f), module, Phasor::DENOMINATOR_PARAM));

  addInput(createInput<ZZC_PJ_Port>(Vec(12.164f, 196.07f), module, Phasor::SYNC_INPUT));

  addParam(createParam<ZZC_LEDBezelDark>(Vec(84.586f, 197.32f), module, Phasor::HARDSYNC_PARAM));
  addChild(createLight<LedLight<ZZC_YellowLight>>(Vec(86.386f, 199.12f), module, Phasor::HARDSYNC_LED));

  addParam(createParam<ZZC_CrossKnob29>(Vec(39.5f, 209.173f), module, Phasor::FREQ_FINE_PARAM));

  addParam(createParam<ZZC_KnobWithDot19>(Vec(14.913f, 244.197), module, Phasor::PM_PARAM));
  addParam(createParam<ZZC_KnobWithDot19>(Vec(86.087f, 244.197), module, Phasor::FM_PARAM));

  addInput(createInput<ZZC_PJ_Port>(Vec(11.914f, 275.f), module, Phasor::PM_INPUT));
  addInput(createInput<ZZC_PJ_Port>(Vec(47.5f, 275.f), module, Phasor::VOCT_INPUT));
  addInput(createInput<ZZC_PJ_Port>(Vec(83.086f, 275.f), module, Phasor::FM_INPUT));

  addOutput(createOutput<ZZC_PJ_Port>(Vec(11.914f, 320.f), module, Phasor::SIN_OUTPUT));
  addOutput(createOutput<ZZC_PJ_Port>(Vec(47.5f, 320.f), module, Phasor::COS_OUTPUT));
  addOutput(createOutput<ZZC_PJ_Port>(Vec(83.086f, 320.f), module, Phasor::PHASE_OUTPUT));

  addChild(createWidget<ZZC_Screw>(Vec(RACK_GRID_WIDTH, 0)));
  addChild(createWidget<ZZC_Screw>(Vec(box.size.x - 2 * RACK_GRID_WIDTH, 0)));
  addChild(createWidget<ZZC_Screw>(Vec(RACK_GRID_WIDTH, RACK_GRID_HEIGHT - RACK_GRID_WIDTH)));
  addChild(createWidget<ZZC_Screw>(Vec(box.size.x - 2 * RACK_GRID_WIDTH, RACK_GRID_HEIGHT - RACK_GRID_WIDTH)));
}

void PhasorWidget::appendContextMenu(Menu *menu) {
}

Model *modelPhasor = createModel<Phasor, PhasorWidget>("Phasor");