#include "osdialog.h"
#include "ZZC.hpp"
#include "WavetablePlayer.hpp"
#include "filetypes/WavSupport.hpp"

WavetablePlayer::WavetablePlayer() {
  this->wt = new Wavetable();
  config(NUM_PARAMS, NUM_INPUTS, NUM_OUTPUTS, NUM_LIGHTS);
  configParam(INDEX_PARAM, 0.f, 1.f, 0.f, "Wave Index");
  configParam(INDEX_CV_ATT_PARAM, -1.f, 1.f, 0.f, "Wave Index CV Attenuverter");
}

json_t *WavetablePlayer::dataToJson() {
  json_t *rootJ = json_object();
  return rootJ;
}

void WavetablePlayer::dataFromJson(json_t *rootJ) {
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

  // if (mipmapLevel != this->lastMipmapLevel) {
    // std::cout << "mipmapLevel: " << mipmapLevel << std::endl;
  // }

  float wave0 = getWTSample(this->wt, index0, this->phase);
  float wave1 = getWTSample(this->wt, index1, this->phase);

  float waveInterpolated = math::crossfade(wave0, wave1, fractpart);

  this->outputs[WAVE_OUTPUT].setVoltage(waveInterpolated * 10.f);

  this->phase = math::eucMod(this->phase + 0.001f, 1.f);
  // this->wave = math::eucMod(this->wave + 0.00001f, 1.f);
  // this->level = math::eucMod(this->level + 0.000003f, 1.f);
}

void WavetablePlayer::selectFolder() {
  std::string dir = asset::user("");
  std::string filename = "Untitled";

  char *path = osdialog_file(OSDIALOG_OPEN, dir.c_str(), filename.c_str(), NULL);
  if (path) {
    this->wtIsReady = false;
    SurgeStorage* ss = new SurgeStorage();
    ss->load_wt(path, this->wt);
    free(ss);
    // module->setPath(path, true);
    free(path);

    std::cout << "size" << ": " << this->wt->size << std::endl;
    std::cout << "n_tables" << ": " << this->wt->n_tables << std::endl;
    std::cout << "size_po2" << ": " << this->wt->size_po2 << std::endl;
    std::cout << "flags" << ": " << this->wt->flags << std::endl;
    std::cout << "dt" << ": " << this->wt->dt << std::endl;

    this->wtIsReady = true;
  }
}

struct WavetablePlayerWidget : ModuleWidget {
  WavetablePlayerWidget(WavetablePlayer *module);
  void appendContextMenu(Menu *menu) override;
};

WavetablePlayerWidget::WavetablePlayerWidget(WavetablePlayer *module) {
  setModule(module);
  setPanel(APP->window->loadSvg(asset::plugin(pluginInstance, "res/panels/WavetablePlayer.svg")));

  addParam(createParam<ZZC_CrossKnob45>(Vec(30.5f, 182.366f), module, WavetablePlayer::INDEX_PARAM));
  addParam(createParam<ZZC_KnobWithDot19>(Vec(50.5f, 245.965f), module, WavetablePlayer::INDEX_CV_ATT_PARAM));

  addInput(createInput<ZZC_PJ_Port>(Vec(47.5f, 275.f), module, WavetablePlayer::INDEX_CV_INPUT));
  addOutput(createOutput<ZZC_PJ_Port>(Vec(47.5f, 320.f), module, WavetablePlayer::WAVE_OUTPUT));

  addChild(createWidget<ZZC_Screw>(Vec(RACK_GRID_WIDTH, 0)));
  addChild(createWidget<ZZC_Screw>(Vec(box.size.x - 2 * RACK_GRID_WIDTH, 0)));
  addChild(createWidget<ZZC_Screw>(Vec(RACK_GRID_WIDTH, RACK_GRID_HEIGHT - RACK_GRID_WIDTH)));
  addChild(createWidget<ZZC_Screw>(Vec(box.size.x - 2 * RACK_GRID_WIDTH, RACK_GRID_HEIGHT - RACK_GRID_WIDTH)));
}

struct SelectFolderItem : MenuItem {
  WavetablePlayer *module;
  void onAction(const event::Action &e) override {
    module->selectFolder();
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
