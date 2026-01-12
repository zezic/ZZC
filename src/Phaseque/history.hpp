#pragma once
#include "Phaseque.hpp"
#include "../ZZC.hpp"

struct PhasequePatternResoChange : history::ModuleAction {
    int paramId;
    unsigned int patternNum;
    float oldValue;
    float newValue;

    void undo() override {
        app::ModuleWidget* mw = APP->scene->rack->getModule(moduleId);
        assert(mw);
        Phaseque* phaseq = static_cast<Phaseque*>(mw->module);
        phaseq->patterns[patternNum].resolution = oldValue;
        if (phaseq->patternIdx == patternNum) {
            phaseq->pattern.resolution = oldValue;
            mw->module->params[paramId].value = oldValue;
        } else {
            phaseq->patternFlashNeg = patternNum;
        }
    }

    void redo() override {
        app::ModuleWidget* mw = APP->scene->rack->getModule(moduleId);
        assert(mw);
        Phaseque* phaseq = static_cast<Phaseque*>(mw->module);
        phaseq->patterns[patternNum].resolution = newValue;
        if (phaseq->patternIdx == patternNum) {
            phaseq->pattern.resolution = newValue;
            mw->module->params[paramId].value = newValue;
        } else {
            phaseq->patternFlashPos = patternNum;
        }
    }

    PhasequePatternResoChange() {
        name = "change pattern resolution";
    }
};

struct PhasequePatternShiftChange : history::ModuleAction {
    int paramId;
    unsigned int patternNum;
    float oldValue;
    float newValue;

    void undo() override {
        app::ModuleWidget* mw = APP->scene->rack->getModule(moduleId);
        assert(mw);
        Phaseque* phaseq = static_cast<Phaseque*>(mw->module);
        phaseq->patterns[patternNum].setShift(oldValue);
        if (phaseq->patternIdx == patternNum) {
            phaseq->pattern.setShift(oldValue);
            mw->module->params[paramId].value = oldValue;
        } else {
            phaseq->patternFlashNeg = patternNum;
        }
    }

    void redo() override {
        app::ModuleWidget* mw = APP->scene->rack->getModule(moduleId);
        assert(mw);
        Phaseque* phaseq = static_cast<Phaseque*>(mw->module);
        phaseq->patterns[patternNum].setShift(newValue);
        if (phaseq->patternIdx == patternNum) {
            phaseq->pattern.setShift(newValue);
            mw->module->params[paramId].value = newValue;
        } else {
            phaseq->patternFlashPos = patternNum;
        }
    }

    PhasequePatternShiftChange() {
        name = "change pattern shift";
    }
};

struct PhasequeStepAttrChange : history::ModuleAction {
    int paramId;
    unsigned int patternNum;
    int step;
    int attr;
    float oldValue;
    float newValue;

    void undo() override {
        app::ModuleWidget* mw = APP->scene->rack->getModule(moduleId);
        assert(mw);
        Phaseque* phaseq = static_cast<Phaseque*>(mw->module);

        unsigned int blockIdx = step / phaseq->pattern.blockSize;
        unsigned int stepInBlockIdx = step % phaseq->pattern.blockSize;

        phaseq->patterns[patternNum].stepBases[attr][blockIdx][stepInBlockIdx] = oldValue;
        phaseq->patterns[patternNum].applyMutations(attr, blockIdx);
        phaseq->patterns[patternNum].recalcInOuts(blockIdx);

        if (phaseq->patternIdx == patternNum) {
            phaseq->pattern.stepBases[attr][blockIdx][stepInBlockIdx] = oldValue;
            phaseq->pattern.applyMutations(attr, blockIdx);
            phaseq->pattern.recalcInOuts(blockIdx);
            mw->module->params[paramId].value = oldValue;
            phaseq->renderParamQuantities();
        }
        phaseq->patternFlashNeg = patternNum;
    }

    void redo() override {
        app::ModuleWidget* mw = APP->scene->rack->getModule(moduleId);
        assert(mw);
        Phaseque* phaseq = static_cast<Phaseque*>(mw->module);

        unsigned int blockIdx = step / phaseq->pattern.blockSize;
        unsigned int stepInBlockIdx = step % phaseq->pattern.blockSize;

        phaseq->patterns[patternNum].stepBases[attr][blockIdx][stepInBlockIdx] = newValue;
        phaseq->patterns[patternNum].applyMutations(attr, blockIdx);
        phaseq->patterns[patternNum].recalcInOuts(blockIdx);

        if (phaseq->patternIdx == patternNum) {
            phaseq->pattern.stepBases[attr][blockIdx][stepInBlockIdx] = newValue;
            phaseq->pattern.applyMutations(attr, blockIdx);
            phaseq->pattern.recalcInOuts(blockIdx);
            mw->module->params[paramId].value = newValue;
            phaseq->renderParamQuantities();
        }
        phaseq->patternFlashPos = patternNum;
    }

    PhasequeStepAttrChange() {
        name = "change step attribute";
    }
};
