#pragma once

#include "Phaseque.hpp"
#include "history.hpp"

std::vector<std::string> StepAttrNames = { "value",         "length",           "shift",
                                           "expression in", "expression curve", "expression power",
                                           "expression out" };

struct ZZC_PhasequePatternResoKnob : SvgKnob {
    float oldValue = 0.0;

    ZZC_PhasequePatternResoKnob() {
        setSvg(APP->window->loadSvg(asset::plugin(pluginInstance, "res/knobs/ZZC-Knob-25-Encoder.svg")));
        shadow->box.size = Vec(29, 29);
        shadow->box.pos = Vec(-2, 2);
        shadow->blurRadius = 15.0f;
        shadow->opacity = 1.0f;
        smooth = false;
        snap = true;
        maxAngle = M_PI * 4;
    }

    void onDragStart(const DragStartEvent& e) override {
        if (e.button != GLFW_MOUSE_BUTTON_LEFT) {
            return;
        }

        engine::ParamQuantity* pq = getParamQuantity();
        if (pq) {
            this->oldValue = pq->getValue();
        }

        SvgKnob::onDragStart(e);
    }

    void onDragEnd(const event::DragEnd& e) override {
        if (e.button != GLFW_MOUSE_BUTTON_LEFT) {
            return;
        }

        APP->window->cursorUnlock();

        engine::ParamQuantity* paramQuantity = this->getParamQuantity();
        if (paramQuantity) {
            float newValue = paramQuantity->getValue();
            if (this->oldValue != newValue) {
                PhasequePatternResoChange* h = new PhasequePatternResoChange;
                h->moduleId = paramQuantity->module->id;
                h->paramId = paramQuantity->paramId;
                Phaseque* phaseq = static_cast<Phaseque*>(paramQuantity->module);
                h->patternNum = phaseq->patternIdx;
                h->oldValue = oldValue;
                h->newValue = newValue;
                h->name = "change pattern " + std::to_string(phaseq->patternIdx) + " resolution";
                APP->history->push(h);
            }
        }
    }
};

struct ZZC_DisplayKnob : SvgKnob {
    ZZC_DirectKnobDisplay* disp = nullptr;

    float strokeWidth = 1.5f;
    bool unipolar = false;
    float oldValue = 0.0;
    float lastDrawnValue = 0.0;
    float valueToDraw = 0.0;

    ZZC_DisplayKnob() {
        smooth = false;
        disp = new ZZC_DirectKnobDisplay();
        fb->addChild(disp);
        disp->box.pos = math::Vec(0, 0);
        disp->strokeWidth = strokeWidth;
        disp->box.size = fb->box.size;
    }

    void onDragMove(const DragMoveEvent& e) override {
        SvgKnob::onDragMove(e);
        engine::ParamQuantity* pq = getParamQuantity();
        if (pq) {
            auto val = pq->getValue();
            pq->setValue(val);
        }
    }

    void onDragStart(const DragStartEvent& e) override {
        if (e.button != GLFW_MOUSE_BUTTON_LEFT) {
            return;
        }

        engine::ParamQuantity* pq = getParamQuantity();
        if (pq) {
            float oldVal = pq->getValue();
            this->oldValue = oldVal;
        }

        SvgKnob::onDragStart(e);
    }

    void recalcSizes() {
        float padding = strokeWidth + 2.f;
        sw->box.pos = math::Vec(padding, padding);
        tw->box.size = sw->box.size;
        math::Vec size = math::Vec(padding * 2, padding * 2).plus(sw->box.size);
        setSize(size);
        fb->box.size = size;
        shadow->box.size = sw->box.size;
        // Move shadow downward by 20% and take value display into account
        shadow->box.pos = math::Vec(padding, padding).plus(math::Vec(0, sw->box.size.y * 0.2));
        disp->strokeWidth = strokeWidth;
        disp->box.size = fb->box.size;
    }

    void onChange(const event::Change& e) override {
        engine::ParamQuantity* paramQuantity = this->getParamQuantity();
        if (paramQuantity) {
            disp->setLimits(paramQuantity->getMinValue(), paramQuantity->getMaxValue());
            this->valueToDraw = paramQuantity->getValue();
        }
        SvgKnob::onChange(e);
    }

    void onDoubleClick(const event::DoubleClick& e) override {
        engine::ParamQuantity* paramQuantity = this->getParamQuantity();
        if (paramQuantity) {
            paramQuantity->setValue(paramQuantity->defaultValue);
        }
    }

    void step() override {
        engine::ParamQuantity* paramQuantity = this->getParamQuantity();

        if (paramQuantity) {
            this->valueToDraw = paramQuantity->getValue();
        }

        if (this->valueToDraw != this->lastDrawnValue) {
            this->fb->setDirty();
        }

        SvgKnob::step();
    }

    void draw(const DrawArgs& args) override {
        disp->value = this->valueToDraw;
        SvgKnob::draw(args);
        this->lastDrawnValue = this->valueToDraw;
    }
};

struct ZZC_PhasequePatternShiftKnob : ZZC_DisplayKnob {
    ZZC_PhasequePatternShiftKnob() {
        setSvg(APP->window->loadSvg(asset::plugin(pluginInstance, "res/knobs/ZZC-Knob-25-Encoder.svg")));
        shadow->box.size = Vec(31, 31);
        shadow->box.pos = Vec(0, 2);
        shadow->blurRadius = 15.0f;
        shadow->opacity = 1.0f;
        maxAngle = M_PI * 1.5;
        recalcSizes();
    }

    void onDragEnd(const event::DragEnd& e) override {
        if (e.button != GLFW_MOUSE_BUTTON_LEFT) {
            return;
        }

        APP->window->cursorUnlock();

        engine::ParamQuantity* paramQuantity = this->getParamQuantity();
        if (paramQuantity) {
            float newValue = paramQuantity->getValue();
            if (this->oldValue != newValue) {
                PhasequePatternShiftChange* h = new PhasequePatternShiftChange;
                h->moduleId = paramQuantity->module->id;
                h->paramId = paramQuantity->paramId;
                Phaseque* phaseq = static_cast<Phaseque*>(paramQuantity->module);
                h->patternNum = phaseq->patternIdx;
                h->oldValue = oldValue;
                h->newValue = newValue;
                h->name = "change pattern " + std::to_string(phaseq->patternIdx) + " shift";
                APP->history->push(h);
            }
        }
    }

    void onDoubleClick(const event::DoubleClick& e) override {
        Phaseque* phaseq = static_cast<Phaseque*>(module);
        phaseq->resetPatternShift();
    }

    void step() override {
        engine::ParamQuantity* paramQuantity = this->getParamQuantity();
        if (paramQuantity) {
            this->valueToDraw = paramQuantity->getValue();
        }
        if (this->valueToDraw != this->lastDrawnValue) {
            this->fb->setDirty();
        }
        SvgKnob::step();
    }
};

struct ZZC_PhasequeMutaKnob : SvgKnob {
    ZZC_PhasequeMutaKnob() {
        setSvg(APP->window->loadSvg(asset::plugin(pluginInstance, "res/knobs/ZZC-Knob-27-Encoder.svg")));
        shadow->box.size = Vec(33, 33);
        shadow->box.pos = Vec(-3, 2);
        shadow->blurRadius = 15.0f;
        shadow->opacity = 1.0f;
        smooth = false;
    }

    void onDragEnd(const event::DragEnd& e) override {
        if (e.button != GLFW_MOUSE_BUTTON_LEFT) {
            return;
        }

        APP->window->cursorUnlock();
    }

    void onDoubleClick(const event::DoubleClick& e) override {
        Phaseque* phaseq = static_cast<Phaseque*>(module);
        phaseq->resetMutation();
    }
};

struct ZZC_PhasequeAttrKnob : ZZC_DisplayKnob {
    ZZC_PhasequeAttrKnob() {}

    void onDragEnd(const event::DragEnd& e) override {
        if (e.button != GLFW_MOUSE_BUTTON_LEFT) {
            return;
        }

        APP->window->cursorUnlock();

        engine::ParamQuantity* paramQuantity = this->getParamQuantity();
        if (paramQuantity) {
            float newValue = paramQuantity->getValue();
            if (oldValue != newValue) {
                PhasequeStepAttrChange* h = new PhasequeStepAttrChange;
                h->moduleId = paramQuantity->module->id;
                h->paramId = paramQuantity->paramId;
                Phaseque* phaseq = static_cast<Phaseque*>(paramQuantity->module);
                h->patternNum = phaseq->patternIdx;
                h->oldValue = oldValue;
                h->newValue = newValue;
                StepAttrParamQuantityBase* q = static_cast<StepAttrParamQuantityBase*>(paramQuantity);
                h->step = q->item;
                h->name = "change step " + StepAttrNames[q->attr];
                h->attr = q->attr;
                APP->history->push(h);
            }
        }
    }
};

struct ZZC_PhasequeXYDisplayWidget : XYDisplayWidget {
    ZZC_PhasequeXYDisplayWidget() {}

    void onDragEnd(const event::DragEnd& e) override {
        if (e.button != GLFW_MOUSE_BUTTON_LEFT) {
            return;
        }

        APP->window->cursorUnlock();

        if (this->paramQuantityX) {
            float newValueX = paramQuantityX->getValue();
            if (this->oldValueX != newValueX) {
                PhasequeStepAttrChange* h = new PhasequeStepAttrChange;
                h->moduleId = paramQuantityX->module->id;
                h->paramId = paramQuantityX->paramId;
                Phaseque* phaseq = static_cast<Phaseque*>(paramQuantityX->module);
                h->patternNum = phaseq->patternIdx;
                h->oldValue = oldValueX;
                h->newValue = newValueX;
                StepAttrParamQuantityBase* q = static_cast<StepAttrParamQuantityBase*>(paramQuantityX);
                h->step = q->item;
                h->name = "change step " + StepAttrNames[q->attr];
                h->attr = q->attr;
                APP->history->push(h);
            }
        }
        if (this->paramQuantityY) {
            float newValueY = paramQuantityY->getValue();
            if (this->oldValueY != newValueY) {
                PhasequeStepAttrChange* h = new PhasequeStepAttrChange;
                h->moduleId = paramQuantityY->module->id;
                h->paramId = paramQuantityY->paramId;
                Phaseque* phaseq = static_cast<Phaseque*>(paramQuantityY->module);
                h->patternNum = phaseq->patternIdx;
                h->oldValue = oldValueY;
                h->newValue = newValueY;
                StepAttrParamQuantityBase* q = static_cast<StepAttrParamQuantityBase*>(paramQuantityY);
                h->step = q->item;
                h->name = "change step " + StepAttrNames[q->attr];
                h->attr = q->attr;
                APP->history->push(h);
            }
        }
    }

    void onDoubleClick(const event::DoubleClick& e) override {
        if (this->paramQuantityX) {
            this->paramQuantityX->setValue(this->paramQuantityX->defaultValue);
        }
        if (this->paramQuantityY) {
            this->paramQuantityY->setValue(this->paramQuantityY->defaultValue);
        }
    }
};

struct ZZC_PhasequeStepAttrKnob23 : ZZC_PhasequeAttrKnob {
    ZZC_PhasequeStepAttrKnob23() {
        setSvg(APP->window->loadSvg(asset::plugin(pluginInstance, "res/knobs/ZZC-Knob-27-23-Encoder.svg")));
        shadow->box.size = Vec(33, 33);
        shadow->box.pos = Vec(-1.5, 2);
        shadow->blurRadius = 15.0f;
        shadow->opacity = 1.0f;
        recalcSizes();
    }
};

struct ZZC_PhasequeStepAttrKnob21 : ZZC_PhasequeAttrKnob {
    ZZC_PhasequeStepAttrKnob21() {
        setSvg(APP->window->loadSvg(asset::plugin(pluginInstance, "res/knobs/ZZC-Knob-27-21-Encoder.svg")));
        shadow->box.size = Vec(29, 29);
        shadow->box.pos = Vec(-0.5, 2);
        shadow->blurRadius = 15.0f;
        shadow->opacity = 1.0f;
        recalcSizes();
    }
};

struct ZZC_PhasequeStepAttrKnob21Uni : ZZC_PhasequeStepAttrKnob21 {
    ZZC_PhasequeStepAttrKnob21Uni() {
        unipolar = true;
    }
};

struct ZZC_PhasequeStepAttrKnob19 : ZZC_PhasequeAttrKnob {
    ZZC_PhasequeStepAttrKnob19() {
        setSvg(APP->window->loadSvg(asset::plugin(pluginInstance, "res/knobs/ZZC-Knob-27-19-Encoder.svg")));
        shadow->box.size = Vec(25, 25);
        shadow->box.pos = Vec(0.5, 2);
        shadow->blurRadius = 15.0f;
        shadow->opacity = 1.0f;
        recalcSizes();
        disp->enableColor();
    }
};
