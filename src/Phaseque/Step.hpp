#pragma once

const int NUM_STEPS = 8;

#include "Mutations.hpp"
#include "../ZZC.hpp"
#include "helpers.hpp"


const float baseStepLen = 1.0f / NUM_STEPS;
const float minStepLen = baseStepLen / 64.0f;

enum StepAttr {
    STEP_VALUE,
    STEP_LEN,
    STEP_SHIFT,
    STEP_EXPR_IN,
    STEP_EXPR_CURVE,
    STEP_EXPR_POWER,
    STEP_EXPR_OUT,
    STEP_ATTRS_TOTAL
};

struct AttrDefaults {
    float defaultValue;
    float minValue;
    float maxValue;
    float mutMult;
};

inline AttrDefaults getAttrDefaults(int idx) {
    assert(idx >= 0 && idx < STEP_ATTRS_TOTAL);
    switch (idx) {
    case STEP_VALUE:
        return AttrDefaults{ 0.0f, -2.0f, 2.0f, 0.2f };
    case STEP_LEN:
        return AttrDefaults{ baseStepLen, 0.0f, baseStepLen * 2.0f, 1.0f };
    case STEP_SHIFT:
        return AttrDefaults{ 0.0f, -baseStepLen, baseStepLen, 1.0f };
    case STEP_EXPR_IN:
        return AttrDefaults{ 0.0f, -1.0f, 1.0f, 1.5f };
    case STEP_EXPR_CURVE:
        return AttrDefaults{ 0.0f, -1.0f, 1.0f, 2.0f };
    case STEP_EXPR_POWER:
        return AttrDefaults{ 0.0f, -1.0f, 1.0f, 2.0f };
    case STEP_EXPR_OUT:
        return AttrDefaults{ 0.0f, -1.0f, 1.0f, 1.5f };
    default:
        return AttrDefaults{ 0.0f, 0.0f, 1.0f, 1.0f };
    }
}

struct Step {
    int idx;
    float in_;
    bool isClean = true;
    float mutationStrength = 0.f;

    MutableValue attrs[STEP_ATTRS_TOTAL];

    bool gate;
    float* patternShift = nullptr;
    float* globalShift = nullptr;
    float* globalLen = nullptr;
    rack::engine::Port* exprCurvePort = nullptr;
    rack::engine::Port* exprPowerPort = nullptr;

    Mutator mutator;

    json_t* dataToJson() {
        json_t* stepJ = json_object();
        json_object_set_new(stepJ, "idx", json_integer(idx));
        json_object_set_new(stepJ, "in_", json_real(in_));
        json_object_set_new(stepJ, "gate", json_boolean(gate));
        json_t* attrsJ = json_array();

        for (int i = 0; i < STEP_ATTRS_TOTAL; i++) {
            json_array_append(attrsJ, this->attrs[i].dataToJson());
        }

        json_object_set_new(stepJ, "attrs", attrsJ);
        return stepJ;
    }

    void dataFromJson(json_t* stepJ) {
        idx = json_integer_value(json_object_get(stepJ, "idx"));
        in_ = json_number_value(json_object_get(stepJ, "in_"));
        gate = json_boolean_value(json_object_get(stepJ, "gate"));
        json_t* attrsJ = json_object_get(stepJ, "attrs");

        for (int i = 0; i < STEP_ATTRS_TOTAL; i++) {
            json_t* attrJ = json_array_get(attrsJ, i);
            attrs[i].dataFromJson(attrJ);
        }

        this->updateCleanFlag();
        this->updateMutatedFlag();
    }

    float out() {
        return in() + minLen();
    }

    float out(float patternShift, float globalShift) {
        return in(patternShift, globalShift) + minLen();
    }

    float outBase() {
        return inBase() + minLenBase();
    }

    float outBase(float patternShift, float globalShift) {
        return inBase(patternShift, globalShift) + minLenBase();
    }

    float fastOut(float inCache, float globalLen) {
        return inCache + this->attrs[STEP_LEN].value * globalLen;
    }

    float in() {
        return in_ + attrs[STEP_SHIFT].value + (patternShift ? *patternShift : 0.0f)
               + (globalShift ? *globalShift : 0.0f);
    }

    float in(float patternShift, float globalShift) {
        return in_ + attrs[STEP_SHIFT].value + patternShift + globalShift;
    }

    float inBase() {
        return in_ + attrs[STEP_SHIFT].base + (patternShift ? *patternShift : 0.0f)
               + (globalShift ? *globalShift : 0.0f);
    }

    float inBase(float patternShift, float globalShift) {
        return in_ + attrs[STEP_SHIFT].base + patternShift + globalShift;
    }

    float fastIn(float shift) {
        return in_ + attrs[STEP_SHIFT].value + shift;
    }

    void init(int i) {
        this->idx = i;
        this->in_ = i * baseStepLen;
        this->gate = true;

        for (int i = 0; i < STEP_ATTRS_TOTAL; i++) {
            AttrDefaults defaults = getAttrDefaults(i);
            this->attrs[i].setup(defaults.defaultValue, defaults.minValue, defaults.maxValue, defaults.mutMult);
            this->attrs[i].init();
        }

        this->isClean = true;
        this->mutationStrength = 0.f;
    }

    float minLen() {
        return std::max(minStepLen, this->attrs[STEP_LEN].value * (globalLen ? *globalLen : 1.0f));
    }

    float minLen(float globalLen) {
        return std::max(minStepLen, this->attrs[STEP_LEN].value * globalLen);
    }

    float minLenBase() {
        return std::max(minStepLen, this->attrs[STEP_LEN].base * (globalLen ? *globalLen : 1.0f));
    }

    float minLenBase(float globalLen) {
        return std::max(minStepLen, this->attrs[STEP_LEN].base * globalLen);
    }

    void randomize() {
        // Random uniform with a low threshold used because it's too boring when
        // there is only few notes
        this->gate = random::uniform() > 0.2f;

        for (int i = 0; i < STEP_ATTRS_TOTAL; i++) {
            this->attrs[i].randomize();
        }

        this->isClean = false;
        this->mutationStrength = 0.f;
    }

    inline float expr(float phase) {
        return curve(
            phase,
            clamp(this->attrs[STEP_EXPR_CURVE].value + (exprCurvePort ? exprCurvePort->getVoltage() * 0.2f : 0.0f),
                  -1.0f, 1.0f),
            clamp(this->attrs[STEP_EXPR_POWER].value + (exprPowerPort ? exprPowerPort->getVoltage() * 0.2f : 0.0f),
                  -1.0f, 1.0f),
            this->attrs[STEP_EXPR_IN].value, this->attrs[STEP_EXPR_OUT].value);
    }

    inline float expr(float phase, float exprCurveCV, float exprPowerCV) {
        return curve(phase, clamp(this->attrs[STEP_EXPR_CURVE].value + (exprCurveCV * 0.2f), -1.0f, 1.0f),
                     clamp(this->attrs[STEP_EXPR_POWER].value + (exprPowerCV * 0.2f), -1.0f, 1.0f),
                     this->attrs[STEP_EXPR_IN].value, this->attrs[STEP_EXPR_OUT].value);
    }

    inline float exprBase(float phase) {
        return curve(
            phase,
            clamp(this->attrs[STEP_EXPR_CURVE].base + (exprCurvePort ? exprCurvePort->getVoltage() * 0.2f : 0.0f),
                  -1.0f, 1.0f),
            clamp(this->attrs[STEP_EXPR_POWER].base + (exprPowerPort ? exprPowerPort->getVoltage() * 0.2f : 0.0f),
                  -1.0f, 1.0f),
            this->attrs[STEP_EXPR_IN].base, this->attrs[STEP_EXPR_OUT].base);
    }

    inline float exprBase(float phase, float exprCurveCV, float exprPowerCV) {
        return curve(phase, clamp(this->attrs[STEP_EXPR_CURVE].base + (exprCurveCV * 0.2f), -1.0f, 1.0f),
                     clamp(this->attrs[STEP_EXPR_POWER].base + (exprPowerCV * 0.2f), -1.0f, 1.0f),
                     this->attrs[STEP_EXPR_IN].base, this->attrs[STEP_EXPR_OUT].base);
    }

    float phase(float transportPhase) {
        float eucIn = fastmod(this->in(), 1.0f);
        if (transportPhase < eucIn) {
            return (transportPhase + 1.0f - eucIn) / this->minLen();
        } else {
            return (transportPhase - eucIn) / this->minLen();
        }
    }

    float phaseBase(float transportPhase) {
        float eucIn = fastmod(this->inBase(), 1.0f);
        if (transportPhase < eucIn) {
            return (transportPhase + 1.0f - eucIn) / this->minLenBase();
        } else {
            return (transportPhase - eucIn) / this->minLenBase();
        }
    }

    void quantize() {
        this->attrs[STEP_SHIFT].setValue(roundf(this->attrs[STEP_SHIFT].value * (1.0f / baseStepLen) * 2.0f)
                                         * baseStepLen * 0.5f);
    }

    void resetLength() {
        this->attrs[STEP_LEN].init();
    }

    void mutate(float factor) {
        float mutationAcc = 0.f;
        for (int i = 0; i < STEP_ATTRS_TOTAL; i++) {
            this->attrs[i].mutate(factor);
            mutationAcc += std::abs(this->attrs[i].mutation);
        }
        this->mutationStrength = mutationAcc;
        if (factor > 0.0f) {
            this->isClean = false;
        } else {
            this->updateCleanFlag();
        }
        this->updateMutatedFlag();
    }

    void scaleMutation(float factor) {
        for (int i = 0; i < STEP_ATTRS_TOTAL; i++) {
            this->attrs[i].scaleMutation(factor);
        }
        this->updateCleanFlag();
        this->updateMutatedFlag();
    }

    void resetMutation() {
        for (int i = 0; i < STEP_ATTRS_TOTAL; i++) {
            this->attrs[i].resetMutation();
        }
        this->updateCleanFlag();
        this->mutationStrength = 0.f;
    }

    void updateIn() {
        this->in_ = this->idx * baseStepLen;
    }

    void setAttr(int attr, float factor) {
        this->attrs[attr].adjValue(factor);
        this->isClean = false;
    }

    void setAttrAbs(int attr, float target) {
        this->attrs[attr].setValue(target);
        this->updateCleanFlag();
        this->updateMutatedFlag();
    }

    void setAttrBase(int attr, float target) {
        this->attrs[attr].setBase(target);
        this->updateCleanFlag();
    }

    void updateCleanFlag() {
        for (int i = 0; i < STEP_ATTRS_TOTAL; i++) {
            if (!this->attrs[i].isClean) {
                this->isClean = false;
                return;
            }
        }
        this->isClean = true;
    }

    void updateMutatedFlag() {
        float mutationAcc = 0.f;
        for (int i = 0; i < STEP_ATTRS_TOTAL; i++) {
            mutationAcc += std::abs(this->attrs[i].mutation);
        }
        this->mutationStrength = mutationAcc;
    }

    void resetAttr(int attr) {
        this->attrs[attr].init();
        this->updateCleanFlag();
        this->mutationStrength = 0.f;
    }

    void bakeMutation() {
        for (int i = 0; i < STEP_ATTRS_TOTAL; i++) {
            this->attrs[i].bakeMutation();
        }
        this->updateCleanFlag();
        this->mutationStrength = 0.f;
    }
};
