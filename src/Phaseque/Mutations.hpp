#pragma once
#include "../ZZC.hpp"

using namespace rack;

struct Mutator {
    float factor = 0.0f;

    void init() {
        this->factor = 0.0f;
    }

    float mutate(float value, float force, float low, float high) {
        float range = high - low;

        if ((isNear(value, high) && this->factor > 0.0f) || (isNear(value, low) && this->factor < 0.0f)) {
            this->factor *= -1.0f;
        }

        this->factor = clamp(this->factor + (random::uniform() - 0.5f) * 0.05f, -1.0f, 1.0f);
        return clamp(value + this->factor * range * force, low, high);
    }
};

struct MutableValue {
    float base;
    float mutation;
    float value;
    float defaultValue = 0.0f;
    float minValue;
    float maxValue;
    Mutator mutator;
    bool isClean = true;
    float lastMutFactor = 0.0f;
    float unmutateFrom = 0.0f;
    float mutMult = 1.0f;

    json_t* dataToJson() {
        json_t* mutableValueJ = json_object();
        json_object_set_new(mutableValueJ, "base", json_real(this->base));
        json_object_set_new(mutableValueJ, "mutation", json_real(this->mutation));
        json_object_set_new(mutableValueJ, "isClean", json_boolean(this->isClean));
        return mutableValueJ;
    }

    void dataFromJson(json_t* mutableValueJ) {
        this->base = json_number_value(json_object_get(mutableValueJ, "base"));
        this->mutation = json_number_value(json_object_get(mutableValueJ, "mutation"));
        this->isClean = json_boolean_value(json_object_get(mutableValueJ, "isClean"));
        this->applyMutation();
    }

    void init() {
        this->base = this->defaultValue;
        this->mutation = 0.0f;
        this->mutator.init();
        this->applyMutation();
        this->isClean = true;
    }

    void setup(float defaultVal, float minVal, float maxVal, float mutMulti) {
        this->defaultValue = defaultVal;
        this->minValue = minVal;
        this->maxValue = maxVal;
        this->mutMult = mutMulti;
    }

    void mutate(float factor) {
        if (factor > 0.0f) {
            this->mutation = this->mutator.mutate(this->mutation, factor * this->mutMult, this->minValue - this->base,
                                                  this->maxValue - this->base);
        } else if (factor < 0.0f) {
            if (sgn(factor) != sgn(this->lastMutFactor)) {
                unmutateFrom = fabsf(this->mutation);
            }

            float range = this->maxValue - this->minValue;
            float delta = fabsf(this->unmutateFrom * factor);

            if (this->mutation > 0.0f) {
                this->mutation = fmaxf(this->mutation - delta, 0.0f);
            } else if (this->mutation < 0.0f) {
                this->mutation = fminf(this->mutation + delta, 0.0f);
            }

            if (isNear(this->mutation, 0.0f, range * 0.001f)) {
                this->mutation = 0.0f;
                this->mutator.init();
            }
        }

        this->applyMutation();
        this->lastMutFactor = factor;
        this->isClean = this->mutation == 0.0f && this->base == this->defaultValue;
    }

    void applyMutation() {
        this->value = this->base + this->mutation;
    }

    void clampMutation() {
        if (this->base + this->mutation > this->maxValue) {
            this->mutation = this->maxValue - this->base;
        } else if (this->base + this->mutation < this->minValue) {
            this->mutation = this->minValue - this->base;
        }
    }

    void scaleMutation(float factor) {
        this->mutation *= factor;
        this->clampMutation();
        this->applyMutation();
    }

    void adjValue(float factor) {
        if ((factor > 0.0f && this->base == this->maxValue) || (factor < 0.0f && this->base == this->minValue)) {
            // base is stuck to boundary, try to achieve target value by
            // reducing mutation
            this->mutation += factor;
        } else {
            this->base = clamp(this->base + factor, this->minValue, this->maxValue);
        }

        this->clampMutation();
        this->applyMutation();
        this->isClean = false;
    }

    void setValue(float target) {
        this->base = target;
        this->mutation = 0.0f;
        this->applyMutation();
        this->isClean = false;
    }

    void setBase(float target) {
        this->base = target;
        if (target != this->defaultValue) {
            this->isClean = false;
        }
        this->applyMutation();
    }

    void setMutation(float target) {
        this->mutation = target;
        this->isClean = false;
    }

    void randomize() {
        float range = this->maxValue - this->minValue;
        this->base = this->minValue + random::uniform() * range;
        this->resetMutation();
        this->isClean = false;
    }

    void resetMutation() {
        this->mutation = 0.0f;
        this->mutator.init();
        this->applyMutation();
        this->isClean = this->mutation == 0.0f && this->base == this->defaultValue;
    }

    void bakeMutation() {
        this->base = this->base + this->mutation;
        this->mutation = 0.0f;
        this->isClean = this->base == this->defaultValue;
    }
};
