#include "osdialog.h"
#include "ZZC.hpp"
#include "WavetablePlayer.hpp"
#include "filetypes/WavSupport.hpp"

WavetablePlayer::WavetablePlayer() {
  this->wt = new Wavetable();
  this->wtPtr = std::shared_ptr<Wavetable>(this->wt);
  config(NUM_PARAMS, NUM_INPUTS, NUM_OUTPUTS, NUM_LIGHTS);
  configParam(INDEX_PARAM, 0.f, 1.f, 0.f, "Wave Index");
  configParam(INDEX_CV_ATT_PARAM, -1.f, 1.f, 0.f, "Wave Index CV Attenuverter");
  configParam(XTRA_PARAM, 0.f, 10.f, 5.0f, "Extrapolation");
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
      this->filename = newFilename;
      this->tryToLoadWT(this->filename);
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

  float targetIndex = this->params[INDEX_PARAM].getValue();

  if (this->inputs[INDEX_CV_INPUT].isConnected()) {
    float indexModulation = this->inputs[INDEX_CV_INPUT].getVoltage() * 0.1f * this->params[INDEX_CV_ATT_PARAM].getValue();
    targetIndex = math::clamp(targetIndex + indexModulation, 0.f, 1.f);
  }

  float intpart;
  float fractpart = std::modf(targetIndex * (this->wt->n_tables - 1), &intpart);

  int index0 = intpart;
  int index1 = math::eucMod(index0 + 1, this->wt->n_tables);

  // int mipmapLevel = std::min(this->wt->size_po2 - 2, (int)(this->level * 10));

  float phase = math::eucMod(this->inputs[PHASE_INPUT].getVoltage() * 0.1f, 1.f);

  float wave0 = getWTSample(this->wt, index0, phase);
  float wave1 = getWTSample(this->wt, index1, phase);

  float waveInterpolated = math::crossfade(wave0, wave1, fractpart);

  this->outputs[WAVE_OUTPUT].setVoltage(waveInterpolated * 5.f);
}

bool WavetablePlayer::tryToLoadWT(std::string path) {
  this->wtIsReady = false;
  SurgeStorage* ss = new SurgeStorage();
  bool loaded = ss->load_wt(path, this->wt);
  free(ss);
  this->wtIsReady = true;
  return loaded;
}

void WavetablePlayer::selectFile() {
  std::string dir = asset::user("");

  if (this->filename != "") {
    std::cout << "Filename: " << this->filename << std::endl;
    dir = string::directory(this->filename);
  }

  std::cout << "Opening directory: " << dir << std::endl;

  char *path = osdialog_file(OSDIALOG_OPEN, dir.c_str(), NULL, NULL);
  if (path) {
    bool loaded = this->tryToLoadWT(path);
    if (loaded) {
      this->filename = path;
    }
  }
  free(path);
}

struct WavetableDisplayWidget : BaseDisplayWidget {
  std::shared_ptr<Wavetable> wtPtr;
  NVGcolor monoColor = nvgRGB(0xff, 0xd4, 0x2a);
  NVGcolor polyColor = nvgRGB(0x29, 0xb2, 0xef);

  void draw(const DrawArgs &args) override {
    if (!this->wtPtr) { return; }
    Wavetable* wt = this->wtPtr.get();

    int waveReso = 128;
    int sampleStep = wt->size / waveReso;

    float lineWidth = 0.72f;
    float verticalDepth = this->box.size.y * 0.25f;
    float verticalStep = verticalDepth / (wt->n_tables - 1);
    float potentialOverlap = std::min(0.75f, std::max(0.f, (wt->n_tables * lineWidth - verticalDepth) / verticalDepth));

    NVGcolor graphColor = nvgRGBA(0xfe, 0xc3, 0x00, (int)((float)0x40 * (1.f - potentialOverlap)));

    float diagramWidth = this->box.size.y * 0.8f;
    float diagramTiltX = diagramWidth * 0.25f;
    float diagramTiltY = diagramWidth * 0.1f;
    float waveWidth = diagramWidth - diagramTiltX;
    float waveHeightMultiplier = waveWidth * 0.2;
    float horizontalOffset = (this->box.size.y - diagramWidth) * 0.5f;

    float verticalOffset = this->box.size.y * 0.5f + verticalDepth * 0.5f - diagramTiltY * 0.5f;

    nvgStrokeColor(args.vg, graphColor);

    for (int waveIdx = 0; waveIdx < wt->n_tables; waveIdx++) {
      float waveOffsetY = verticalOffset - verticalStep * waveIdx;
      nvgBeginPath(args.vg);


      float smpl = wt->TableF32Data[waveIdx * wt->size];
      float tiltOffsetX = (float)waveIdx / (float)(std::max(1, wt->n_tables - 1)) * diagramTiltX;
      nvgMoveTo(args.vg, horizontalOffset + tiltOffsetX, waveOffsetY - smpl * waveHeightMultiplier);

      for (int wavePos = 1; wavePos < waveReso + 1; wavePos++) {

        float tiltOffsetY = (float)wavePos / (float)(waveReso) * diagramTiltY;

        float smpl = wt->TableF32Data[waveIdx * wt->size + std::min(wt->size - 1, wavePos * sampleStep)];
        float waveX = (float)wavePos / (float)waveReso * waveWidth;
        nvgLineTo(args.vg, horizontalOffset + tiltOffsetX + waveX, waveOffsetY + tiltOffsetY - smpl * waveHeightMultiplier);
      }

      nvgLineJoin(args.vg, NVG_ROUND);
      nvgLineCap(args.vg, NVG_ROUND);
      nvgMiterLimit(args.vg, 2.f);
      nvgStrokeWidth(args.vg, 0.72f);
      // nvgGlobalCompositeOperation(args.vg, NVG_LIGHTER);
      nvgStroke(args.vg);
    }
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
  if (module) {
    display->wtPtr = std::shared_ptr<Wavetable>(module->wt);
  }
  addChild(display);

  addParam(createParam<ZZC_CrossKnob45>(Vec(30.5f, 182.366f), module, WavetablePlayer::INDEX_PARAM));
  addParam(createParam<ZZC_KnobWithDot19>(Vec(50.5f, 245.965f), module, WavetablePlayer::INDEX_CV_ATT_PARAM));
  addParam(createParam<ZZC_Knob25>(Vec(83.084f, 274.03f), module, WavetablePlayer::XTRA_PARAM));

  addParam(createParam<ZZC_Switch2Vertical>(Vec(12.f, 200.f), module, WavetablePlayer::MIPMAP_PARAM));
  addParam(createParam<ZZC_Switch2Vertical>(Vec(93.f, 200.f), module, WavetablePlayer::INDEX_INTER_PARAM));

  addInput(createInput<ZZC_PJ_Port>(Vec(11.914f, 275.f), module, WavetablePlayer::PHASE_INPUT));
  addInput(createInput<ZZC_PJ_Port>(Vec(47.5f, 275.f), module, WavetablePlayer::INDEX_CV_INPUT));
  addOutput(createOutput<ZZC_PJ_Port>(Vec(11.914f, 320.f), module, WavetablePlayer::INTER_OUTPUT));
  addOutput(createOutput<ZZC_PJ_Port>(Vec(47.5f, 320.f), module, WavetablePlayer::WAVE_OUTPUT));
  addOutput(createOutput<ZZC_PJ_Port>(Vec(83.086f, 320.f), module, WavetablePlayer::XTRA_OUTPUT));

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
