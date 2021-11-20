using namespace rack;

extern Plugin *pluginInstance;

struct ZZC_SmallSlider : SvgSlider {
  ZZC_SmallSlider() {
    Vec margin = Vec(3.5, 3.5);
    maxHandlePos = Vec(2, 2).plus(margin);
    minHandlePos = Vec(2, 24).plus(margin);
    setBackgroundSvg(APP->window->loadSvg(asset::plugin(pluginInstance, "res/sliders/ZZC-Small-Slider_BG.svg")));
    setHandleSvg(APP->window->loadSvg(asset::plugin(pluginInstance, "res/sliders/ZZC-Small-Slider_Handle.svg")));
    background->box.pos = margin;
    box.size = background->box.size.plus(margin.mult(2));
  }
};
