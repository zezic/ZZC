#pragma once

#define NUM_PATTERNS 32

#include "../ZZC.hpp"

using namespace rack;

struct NegSchmittTrigger : dsp::SchmittTrigger {
    bool process(float in) {
        if (state) {
            if (in >= 0.f) {
                state = false;
            }
        } else {
            if (in <= -1.f) {
                state = true;
                return true;
            }
        }
        return false;
    }
};

struct ChangeTrigger {
    bool lastState;
    bool hasLastState = false;

    ChangeTrigger() {
        reset();
    }

    void reset() {
        lastState = true;
    }

    bool process(bool state) {
        if (!hasLastState) {
            lastState = state;
            hasLastState = true;
            return false;
        }

        bool triggered = lastState != state;
        lastState = state;

        return triggered;
    }
};

struct TempoTracker {
    int ticksTracked = 0;
    float delta = 0.0f;
    bool detected = false;
    float bps = 0.0f;

    void reset() {
        ticksTracked = 0;
        delta = 0.0f;
        detected = false;
        bps = 0.0f;
    }

    void tick(float sampleTime) {
        delta += sampleTime;
        ticksTracked++;
        if (ticksTracked > 1) {
            detected = true;
            bps = 1.0f / delta;
        }
        delta = 0.0f;
    }

    void acc(float sampleTime) {
        delta += sampleTime;
    }
};

struct Limits {
    unsigned int low;
    unsigned int high;
};

inline Limits getRowLimits(int idx) {
    unsigned int rowIdx = (idx) / 4;
    Limits limits;
    limits.low = rowIdx * 4;
    limits.high = (rowIdx + 1) * 4;
    return limits;
}

inline Limits getColumnLimits(int idx) {
    unsigned int baseIdx = idx % 4;
    Limits limits;
    limits.low = baseIdx;
    limits.high = baseIdx + 8 * 4;
    return limits;
}

inline float crossfadePow(float base, float exp) {
    float lowExp = std::floor(exp - 1.0f);
    float low = base;

    for (float i = 0.0f; i < lowExp; i = i + 1.0f) {
        low = low * base;
    }

    float high = low * base;
    float balance = exp - lowExp;

    return crossfade(low, high, balance);
}

inline float fastmod(float value, float base) {
    if (value >= 0.0f && value < base) {
        return value;
    }

    if (value < 0.0f) {
        return value + base;
    }

    return value - base;
}

inline float curve(float phase, float curvature, float power, float in, float out) {
    float range = out - in;

    if (curvature == 0.0f) {
        return in + range * phase;
    }

    float absCurvature = std::abs(curvature);
    bool invCurve = in > out;
    bool invSpline = (curvature > 0.0f) ^ invCurve;
    power = 5.0f + power * (invSpline ? -3.0f : 3.0f);

    // float spline = invSpline ? (1.0f - std::pow(1.0f - phase, power)) :
    // std::pow(phase, power);
    float spline = invSpline ? (1.0f - crossfadePow(1.0f - phase, power)) : crossfadePow(phase, power);

    // float deformed = (phase * (1.0f - absCurvature)) + spline * absCurvature;
    // float deformed = phase + (spline - phase) * absCurvature;
    float deformed = crossfade(phase, spline, absCurvature);

    return in + deformed * range;
}
