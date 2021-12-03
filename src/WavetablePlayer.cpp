#include "osdialog.h"
#include "ZZC.hpp"
#include "WavetablePlayer.hpp"
#include "filetypes/WavSupport.hpp"

WavetablePlayer::WavetablePlayer() {
  this->wtPtr = std::make_shared<Wavetable>();
  this->debugDivider.setDivision(1000);
  config(NUM_PARAMS, NUM_INPUTS, NUM_OUTPUTS, NUM_LIGHTS);
  configParam(INDEX_PARAM, 0.f, 1.f, 0.f, "Wave Index");
  configParam(INDEX_CV_ATT_PARAM, -1.f, 1.f, 0.f, "Wave Index CV Attenuverter");
  // configParam(XTRA_PARAM, 0.f, 10.f, 5.0f, "Extrapolation");
  configParam(MIPMAP_PARAM, 0.f, 1.f, 1.0f, "MIP-mapping");
  configParam(INDEX_INTER_PARAM, 0.f, 1.f, 1.0f, "Index Interpolation");
}

json_t *WavetablePlayer::dataToJson() {
  json_t *rootJ = json_object();
  json_object_set_new(rootJ, "filename", json_string(filename.c_str()));
  return rootJ;
}

void WavetablePlayer::dataFromJson(json_t *rootJ) {
  json_t *filenameJ = json_object_get(rootJ, "filename");
  if (filenameJ) {
    std::string newFilename = json_string_value(filenameJ);
    if (newFilename != this->filename) {
      this->tryToLoadWT(newFilename);
    }
  }
}

float getWTSample(Wavetable* wt, int wave, float phase) {
  int targetWaveSize = wt->size;

  float intpart;
  float fractpart = std::modf(phase * targetWaveSize, &intpart);

  int index0 = intpart;
  int index1 = math::eucMod(index0 + 1, targetWaveSize);

  int waveOffset = targetWaveSize * wave;

  float sample0 = wt->TableF32Data[waveOffset + index0];
  float sample1 = wt->TableF32Data[waveOffset + index1];

  return math::crossfade(sample0, sample1, fractpart);
}

float getWTMipmapSample(Wavetable* wt, int mipmapLevel, int wave, float phase) {
  int targetWaveSize = wt->size >> mipmapLevel;

  float intpart;
  float fractpart = std::modf(phase * targetWaveSize, &intpart);

  int index0 = intpart;
  int index1 = math::eucMod(index0 + 1, targetWaveSize);

  float sample0 = wt->TableF32WeakPointers[mipmapLevel][wave][index0];
  float sample1 = wt->TableF32WeakPointers[mipmapLevel][wave][index1];

  return math::crossfade(sample0, sample1, fractpart);
}

void WavetablePlayer::process(const ProcessArgs &args) {
  if (!this->wtIsReady) { return; }

  Wavetable* wt = this->wtPtr.get();

  float targetIndex = this->params[INDEX_PARAM].getValue();

  if (this->inputs[INDEX_CV_INPUT].isConnected()) {
    float indexModulation = this->inputs[INDEX_CV_INPUT].getVoltage() * 0.1f * this->params[INDEX_CV_ATT_PARAM].getValue();
    targetIndex = math::clamp(targetIndex + indexModulation, 0.f, 1.f);
  }


  float intpart;
  float fractpart = std::modf(targetIndex * (wt->n_tables - 1), &intpart);

  int index0 = intpart;
  this->indexIntpart = index0;
  int index1 = math::eucMod(index0 + 1, wt->n_tables);

  // int mipmapLevel = std::min(wt->size_po2 - 2, (int)(this->level * 10));

  float phase = math::eucMod(this->inputs[PHASE_INPUT].getVoltage() * 0.1f, 1.f);

  // int targetMipmapLevel = this->inputs[MIPMAP_INPUT].isConnected() ? std::max(0, (int)std::floor(this->inputs[MIPMAP_INPUT].getVoltage())) : 0;

  float junk;
  float phaseDelta = std::modf((phase + 1.f) - lastPhase, &junk);

  int targetMipmapLevel = -1;
  float mipmapInterpol = 0.f;

  if (this->params[MIPMAP_PARAM].getValue() > 0.f) {
    if (phaseDelta > 0.f) {
      float samplesPerCycle = 1.f / phaseDelta;
      float idealSizePo2 = std::log2f(samplesPerCycle);
      float brightnessModifier = 0.0f;
      float possibleSizePo2 = math::clamp(idealSizePo2 + brightnessModifier, 3.f, (float)wt->size_po2);

      float referenceMipmapLevel = (float)wt->size_po2 - possibleSizePo2;

      float targetMipmapLevelFloat;
      mipmapInterpol = std::modf(referenceMipmapLevel, &targetMipmapLevelFloat);
      targetMipmapLevel = targetMipmapLevelFloat;
      mipmapInterpol = mipmapInterpol * mipmapInterpol;

      // targetMipmapLevel = wt->size_po2 - (int)nearestSizePo2Int;

      // if (this->debugDivider.process()) {
      //   std::cout << targetMipmapLevel << std::endl;
      // }
    }
  }

  float wave0 = targetMipmapLevel >= 0 ? math::crossfade(
    getWTMipmapSample(wt, targetMipmapLevel, index0, phase),
    getWTMipmapSample(wt, targetMipmapLevel + 1, index0, phase),
    mipmapInterpol
  ) : getWTSample(wt, index0, phase);
  float wave1 = targetMipmapLevel >= 0 ? math::crossfade(
    getWTMipmapSample(wt, targetMipmapLevel, index1, phase),
    getWTMipmapSample(wt, targetMipmapLevel + 1, index1, phase),
    mipmapInterpol
  ) : getWTSample(wt, index1, phase);

  float waveInterpolated = math::crossfade(wave0, wave1, fractpart);
  this->interpolation = fractpart;

  this->outputs[WAVE_OUTPUT].setVoltage(waveInterpolated * 5.f);
  this->index = targetIndex;

  this->lastPhase = phase;
}

bool WavetablePlayer::tryToLoadWT(std::string path) {
  if (!system::isFile(path)) { return false; }
  this->wtIsReady = false;
  SurgeStorage* ss = new SurgeStorage();
  bool loaded = ss->load_wt(path, this->wtPtr.get());
  free(ss);
  this->wtIsReady = true;
  if (loaded) {
    this->filename = path;
  }
  return loaded;
}

void WavetablePlayer::selectFile() {
  std::string dir = asset::user("");

  if (this->filename != "") {
    std::cout << "Filename: " << this->filename << std::endl;
    dir = system::getDirectory(this->filename);
  }

  std::cout << "Opening directory: " << dir << std::endl;

  char *path = osdialog_file(OSDIALOG_OPEN, dir.c_str(), NULL, NULL);
  if (path) {
    this->tryToLoadWT(std::string(path));
  }
  free(path);
}

void WavetablePlayer::switchFile(int delta) {
  if (this->filename == "") { return; }

  std::string dir = system::getDirectory(this->filename);
  std::vector<std::string> entries = {};

  for (std::string wtPath : system::getEntries(dir)) {
    if (!system::isFile(wtPath)) { continue; }
    entries.push_back(wtPath);
  }

  std::vector<std::string>::iterator it = std::find(entries.begin(), entries.end(), this->filename);

  if (it != entries.end()) {
    int currentIdx = std::distance(entries.begin(), it);
    int targetIdx = math::eucMod(currentIdx + delta, entries.size());
    std::string targetPath = entries.at(targetIdx);
    this->tryToLoadWT(targetPath);
  } else if (!entries.empty()) {
    this->tryToLoadWT(entries.front());
  }
}

struct WaveformDimensions {
  Vec pos;
  Vec waveSize;
  Vec depth;
  float skew;
};

void drawWave(
  const Widget::DrawArgs &args,
  Vec pos, Vec size, float skew,
  int reso, int dataSize,
  float* data, bool interpolate, float interpolation = 0.f
) {
  float smpl = interpolate ? math::crossfade(data[0], data[dataSize], interpolation) : data[0];
  int stepSize = dataSize / reso;

  nvgBeginPath(args.vg);
  nvgMoveTo(args.vg, pos.x, pos.y + smpl * size.y);

  for (int step = 1; step < reso + 1; step++) {
    int smplIdx = std::min(dataSize - 1, step * stepSize);
    float smpl = interpolate ? math::crossfade(data[smplIdx], data[smplIdx + dataSize], interpolation) : data[smplIdx];
    float smplPhase = (float)step / (float)reso;
    float smplX = smplPhase * size.x;
    float smplSkew = smplPhase * skew;
    nvgLineTo(args.vg, pos.x + smplX, pos.y - smpl * size.y + smplSkew);
  }

  nvgLineJoin(args.vg, NVG_ROUND);
  nvgLineCap(args.vg, NVG_ROUND);
  nvgMiterLimit(args.vg, 2.f);
  nvgStrokeWidth(args.vg, 0.72f);
  nvgStroke(args.vg);
}

struct WavetableWidget : TransparentWidget {
  std::shared_ptr<Wavetable> wtPtr;
  float lineWidth = 0.72f;
  int waveReso = 256;
  NVGcolor dimmedColor = nvgRGBA(0xfe, 0xc3, 0x00, 0x40);
  NVGcolor brightColor = nvgRGB(0xff, 0xd4, 0x2a);
  NVGcolor graphColor = nvgRGBA(0xfe, 0xc3, 0x00, 0x40);
  WaveformDimensions wd;
  std::string* filename = nullptr;

  void draw(const DrawArgs &args) override {

    if (!this->wtPtr) { return; }

    std::shared_ptr<Font> font = APP->window->loadFont(asset::plugin(pluginInstance, "res/fonts/SKODANext/SKODANext-Regular.ttf"));
    if (!font) { return; }

    Wavetable* wt = this->wtPtr.get();

    nvgStrokeColor(args.vg, this->graphColor);

    for (int waveIdx = 0; waveIdx < wt->n_tables; waveIdx++) {
      Vec pos = this->wd.pos.plus(this->wd.depth.mult((float)waveIdx / (float)(wt->n_tables - 1)));
      drawWave(args, pos, this->wd.waveSize, this->wd.skew, this->waveReso, wt->size, wt->TableF32Data + waveIdx * wt->size, false);
    }

    nvgFontSize(args.vg, 8.5f);
    nvgFontFaceId(args.vg, font->handle);
    nvgTextLetterSpacing(args.vg, 0.1f);
    nvgTextAlign(args.vg, NVG_ALIGN_CENTER);
    Vec textPos = Vec(box.size.x / 2.f, box.size.y * 0.13f);
    nvgFillColor(args.vg, dimmedColor);
    nvgText(args.vg, textPos.x, textPos.y, string::f("%d x %d", wt->size, wt->n_tables).c_str(), nullptr);

    textPos = Vec(box.size.x / 2.f, box.size.y * 0.89f);
    nvgFillColor(args.vg, brightColor);

    std::string fullFilename = system::getFilename(*this->filename);
    std::string finalFilename = fullFilename;

    size_t maxLength = 16;
    if (fullFilename.length() > maxLength) {
      if (fullFilename.find('-') != std::string::npos) {
        // Has delimiter, let's split first part
        finalFilename = string::ellipsize(
          string::trim(
            fullFilename.substr(
              fullFilename.find('-') + 1,
              fullFilename.length() - fullFilename.find('-') - 1
            )
          ),
          maxLength
        );
      } else {
        finalFilename = string::ellipsize(fullFilename, maxLength);
      }
    }

    nvgText(args.vg, textPos.x, textPos.y, finalFilename.c_str(), nullptr);
    // nvgText(args.vg, textPos.x, textPos.y, "Filename", NULL);
  }
};

struct WaveformWidget : TransparentWidget {
  std::shared_ptr<Wavetable> wtPtr;
  float* index = nullptr;
  int* indexIntpart = nullptr;
  float* interpolation = nullptr;
  bool* inter = nullptr;
  int waveReso = 256;
  WaveformDimensions wd;
  NVGcolor graphColor = nvgRGB(0xff, 0xd4, 0x2a);

  void draw(const DrawArgs &args) override {
    if (!this->wtPtr) { return; }

    Wavetable* wt = this->wtPtr.get();

    nvgStrokeColor(args.vg, this->graphColor);

    Vec pos = this->wd.pos.plus(this->wd.depth.mult(*this->index));
    drawWave(args, pos, this->wd.waveSize, this->wd.skew, this->waveReso, wt->size, wt->TableF32Data + *this->indexIntpart * wt->size, true, *this->interpolation);
  }
};

struct WavetableOpenButtonWidget : SvgButton {
  Module* module = nullptr;

  WavetableOpenButtonWidget() {
    this->addFrame(APP->window->loadSvg(asset::plugin(pluginInstance, "res/switches/ZZC-Open-Button.svg")));
    this->shadow->opacity = 0.f;
  }

  void onAction(const event::Action& e) override {
    if (!this->module) { return; }
    WavetablePlayer* wtPlayer = static_cast<WavetablePlayer*>(this->module);
    wtPlayer->selectFile();
  }
};

struct WavetablePrevButtonWidget : SvgButton {
  Module* module = nullptr;

  WavetablePrevButtonWidget() {
    this->addFrame(APP->window->loadSvg(asset::plugin(pluginInstance, "res/switches/ZZC-Prev-Button.svg")));
    this->shadow->opacity = 0.f;
  }

  void onAction(const event::Action& e) override {
    if (!this->module) { return; }
    WavetablePlayer* wtPlayer = static_cast<WavetablePlayer*>(this->module);
    wtPlayer->switchFile(-1);
  }
};

struct WavetableNextButtonWidget : SvgButton {
  Module* module = nullptr;

  WavetableNextButtonWidget() {
    this->addFrame(APP->window->loadSvg(asset::plugin(pluginInstance, "res/switches/ZZC-Next-Button.svg")));
    this->shadow->opacity = 0.f;
  }

  void onAction(const event::Action& e) override {
    if (!this->module) { return; }
    WavetablePlayer* wtPlayer = static_cast<WavetablePlayer*>(this->module);
    wtPlayer->switchFile(1);
  }
};

struct WavetableDisplayWidget : Widget {
  std::shared_ptr<Wavetable> wtPtr;
  NVGcolor monoColor = nvgRGB(0xff, 0xd4, 0x2a);
  NVGcolor polyColor = nvgRGB(0x29, 0xb2, 0xef);

  widget::FramebufferWidget* fbw;
  WavetableWidget *wtw;
  WaveformWidget *wfw;
  WaveformDimensions wd;
  WavetableOpenButtonWidget *obw;
  WavetablePrevButtonWidget *pbw;
  WavetableNextButtonWidget *nbw;

  WavetableDisplayWidget() {
    this->fbw = new widget::FramebufferWidget;
    this->addChild(this->fbw);

    this->wtw = new WavetableWidget;
    this->fbw->addChild(this->wtw);

    this->wfw = new WaveformWidget;
    this->addChild(this->wfw);

    this->obw = new WavetableOpenButtonWidget;
    this->addChild(this->obw);

    this->pbw = new WavetablePrevButtonWidget;
    this->addChild(this->pbw);

    this->nbw = new WavetableNextButtonWidget;
    this->addChild(this->nbw);
  }

  NVGcolor calcColor(float step, float lineWidth) {
    float potentialOverlap = std::max(0.f, lineWidth * 1.5f - step);
    float alphaMultiplier = std::max(0.05f, 1.f - potentialOverlap);
    std::cout << "alphaMultiplier: " << alphaMultiplier << std::endl;
    return nvgRGBA(
      0xfe, 0xc3, 0x00,
      (int)((float)0x40 * alphaMultiplier)
    );
  }

  void setupSizes() {
    this->wtw->setSize(this->box.size);
    this->fbw->setSize(this->box.size);

    float tableWidth = this->box.size.y * 0.8f;
    Vec depth(tableWidth * 0.2f, this->box.size.y * -0.25f);
    float skew = tableWidth * 0.1f;
    float waveWidth = tableWidth - depth.x;
    float waveHeight = waveWidth * 0.2;
    Vec waveSize(waveWidth, waveHeight);
    Vec pos(
      (this->box.size.y - tableWidth) * 0.5f,
      this->box.size.y * 0.5f - depth.y * 0.5f - skew * 0.5f
    );

    this->wd.depth = depth;
    this->wd.pos = pos;
    this->wd.skew = skew;
    this->wd.waveSize = waveSize;

    this->wtw->wd = this->wd;
    this->wfw->wd = this->wd;

    this->obw->box.pos.y = this->box.size.y - this->obw->box.size.y;

    this->nbw->box.pos.x = this->box.size.x - this->nbw->box.size.x;
    this->nbw->box.pos.y = this->box.size.y - this->nbw->box.size.y;

    this->pbw->box.pos.x = this->nbw->box.pos.x - this->pbw->box.size.x;
    this->pbw->box.pos.y = this->box.size.y - this->pbw->box.size.y;
  }

  void step() override {
    if (this->wtPtr) {
      Wavetable* wt = this->wtPtr.get();
      if (wt->refresh_display) {
        wt->refresh_display = false;
        float verticalStep = (-this->wd.depth.y) / (wt->n_tables - 1);
        this->wtw->graphColor = calcColor(verticalStep, this->wtw->lineWidth);
        this->fbw->dirty = true;
      }
    }
    Widget::step();
  }
};

struct WavetablePlayerWidget : ModuleWidget {
  WavetablePlayerWidget(WavetablePlayer *module);
  void appendContextMenu(Menu *menu) override;
};

WavetablePlayerWidget::WavetablePlayerWidget(WavetablePlayer *module) {
  setModule(module);
  setPanel(APP->window->loadSvg(asset::plugin(pluginInstance, "res/panels/WavetablePlayer.svg")));

  WavetableDisplayWidget *display = new WavetableDisplayWidget();
  display->box.pos = Vec(0.f, 40.f);
  display->box.size = Vec(this->box.size.x, this->box.size.x);
  display->setupSizes();
  if (module) {
    display->wtPtr = module->wtPtr;

    display->wtw->wtPtr = module->wtPtr;
    display->wtw->filename = &module->filename;

    display->wfw->wtPtr = module->wtPtr;
    display->wfw->index = &module->index;
    display->wfw->indexIntpart = &module->indexIntpart;
    display->wfw->interpolation = &module->interpolation;
    display->wfw->inter = &module->indexInter;

    display->obw->module = module;
    display->pbw->module = module;
    display->nbw->module = module;
  }
  addChild(display);

  addParam(createParam<ZZC_CrossKnob45>(Vec(30.5f, 182.366f), module, WavetablePlayer::INDEX_PARAM));
  addParam(createParam<ZZC_KnobWithDot19>(Vec(50.5f, 245.965f), module, WavetablePlayer::INDEX_CV_ATT_PARAM));
  // addParam(createParam<ZZC_Knob25>(Vec(83.084f, 274.03f), module, WavetablePlayer::XTRA_PARAM));

  addParam(createParam<ZZC_Switch2Vertical>(Vec(12.f, 200.f), module, WavetablePlayer::MIPMAP_PARAM));
  addParam(createParam<ZZC_Switch2Vertical>(Vec(93.f, 200.f), module, WavetablePlayer::INDEX_INTER_PARAM));

  addInput(createInput<ZZC_PJ_Port>(Vec(11.914f, 275.f), module, WavetablePlayer::PHASE_INPUT));
  addInput(createInput<ZZC_PJ_Port>(Vec(47.5f, 275.f), module, WavetablePlayer::INDEX_CV_INPUT));
  // addOutput(createOutput<ZZC_PJ_Port>(Vec(11.914f, 320.f), module, WavetablePlayer::INTER_OUTPUT));
  addOutput(createOutput<ZZC_PJ_Port>(Vec(47.5f, 320.f), module, WavetablePlayer::WAVE_OUTPUT));
  // addOutput(createOutput<ZZC_PJ_Port>(Vec(83.086f, 320.f), module, WavetablePlayer::XTRA_OUTPUT));

  addChild(createWidget<ZZC_Screw>(Vec(RACK_GRID_WIDTH, 0)));
  addChild(createWidget<ZZC_Screw>(Vec(box.size.x - 2 * RACK_GRID_WIDTH, 0)));
  addChild(createWidget<ZZC_Screw>(Vec(RACK_GRID_WIDTH, RACK_GRID_HEIGHT - RACK_GRID_WIDTH)));
  addChild(createWidget<ZZC_Screw>(Vec(box.size.x - 2 * RACK_GRID_WIDTH, RACK_GRID_HEIGHT - RACK_GRID_WIDTH)));
}

struct SelectFolderItem : MenuItem {
  WavetablePlayer *module;
  void onAction(const event::Action &e) override {
    module->selectFile();
  }
};

void WavetablePlayerWidget::appendContextMenu(Menu *menu) {

  WavetablePlayer *wavetablePlayer = dynamic_cast<WavetablePlayer*>(module);
  assert(wavetablePlayer);

  menu->addChild(new MenuSeparator());

  SelectFolderItem *selectFolderItem = new SelectFolderItem;
  selectFolderItem->text = "Select Wavetables Folder...";
  selectFolderItem->module = wavetablePlayer;
  menu->addChild(selectFolderItem);
}

Model *modelWavetablePlayer = createModel<WavetablePlayer, WavetablePlayerWidget>("WavetablePlayer");
