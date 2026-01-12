#pragma once
#include <bitset>
#include <iomanip>
#include "Step.hpp"
#include "../ZZC.hpp"

// #include "simd/functions.hpp"
// #include "simd/vector.hpp"

inline simd::float_4 eucMod(simd::float_4 a, simd::float_4 b) {
    simd::float_4 mod = simd::fmod(a, b);
    return simd::ifelse(mod < 0.f, mod + b, mod);
}

inline simd::float_4 createMask(int x) {
    __m128i msk8421 = _mm_set_epi32(8, 4, 2, 1);
    __m128i x_bc = _mm_set1_epi32(x);
    __m128i t = _mm_and_si128(x_bc, msk8421);
    return simd::float_4(_mm_castsi128_ps(_mm_cmpeq_epi32(msk8421, t)));
}

simd::float_4 getBlockExpressions(simd::float_4 exprIn, simd::float_4 exprOut, simd::float_4 exprPower,
                                  simd::float_4 exprCurve, simd::float_4 phase, float globalPower, float globalCurve);

#define BLOCK_SIZE 4

template<unsigned int SIZE> struct Pattern {
    /* Pattern settings */
    unsigned int resolution = SIZE;
    unsigned int goTo = 0;
    float shift = 0.f;
    float globalShift = 0.f;

    unsigned int size = SIZE;
    unsigned int blockSize = BLOCK_SIZE;
    float baseStepLen = 1.f / SIZE;
    float globalLen = 1.f;

    unsigned int activeStepIdx = 0;
    unsigned int lastActiveStepIdx = 0;
    unsigned int activeBlockIdx = -1;
    unsigned int activeStepInBlockIdx = 0;
    int activeBlockMask = 0;
    bool hasActiveStep = false;

    simd::float_4 hits[SIZE / BLOCK_SIZE] = { 0.f };
    simd::float_4 hitsClean[SIZE / BLOCK_SIZE] = { 0.f };

    /* Step attributes, mutations and gates */
    simd::float_4 stepBases[STEP_ATTRS_TOTAL][SIZE / BLOCK_SIZE] = { { 0.f } };
    simd::float_4 stepMutas[STEP_ATTRS_TOTAL][SIZE / BLOCK_SIZE] = { { simd::float_4(0.f) } };
    simd::float_4 stepGates[SIZE / BLOCK_SIZE] = { 0.f };
    simd::float_4 stepIns[SIZE / BLOCK_SIZE] = { 0.f };

    simd::float_4 stepMutaVectors[STEP_ATTRS_TOTAL][SIZE / BLOCK_SIZE] = { { simd::float_4(0.f) } };
    simd::float_4 stepBasesMutated[STEP_ATTRS_TOTAL][SIZE / BLOCK_SIZE] = { { 0.f } };

    simd::float_4 stepInsComputed[SIZE / BLOCK_SIZE] = { 0.f };
    simd::float_4 stepOutsComputed[SIZE / BLOCK_SIZE] = { 0.f };
    simd::float_4 stepMutaInsComputed[SIZE / BLOCK_SIZE] = { 0.f };
    simd::float_4 stepMutaOutsComputed[SIZE / BLOCK_SIZE] = { 0.f };

    simd::float_4 stepAttrDefaults[STEP_ATTRS_TOTAL] = {
        simd::float_4(0.f), simd::float_4(1.f / SIZE), simd::float_4(0.f), simd::float_4(0.f),
        simd::float_4(0.f), simd::float_4(0.f),        simd::float_4(0.f),
    };

    simd::float_4 attrMutaMultipliers[STEP_ATTRS_TOTAL] = { simd::float_4(0.5f),  simd::float_4(0.1f),
                                                            simd::float_4(0.02f), simd::float_4(1.0f),
                                                            simd::float_4(1.0f),  simd::float_4(1.0f),
                                                            simd::float_4(1.0f) };

    std::pair<float, float> attrBounds[STEP_ATTRS_TOTAL] = { { -2.0f, 2.0f },
                                                             { 0.0f, baseStepLen * 2.0f },
                                                             { -baseStepLen, baseStepLen },
                                                             { -1.0f, 1.0f },
                                                             { -1.0f, 1.0f },
                                                             { -1.0f, 1.0f },
                                                             { -1.0f, 1.0f } };

    void findStepsForPhase(float phase, bool globalGate) {
        // Search for phase-step intersection
        for (unsigned int blockIdx = 0; blockIdx < SIZE / BLOCK_SIZE; blockIdx++) {
            simd::float_4 inMod = this->stepMutaInsComputed[blockIdx];
            simd::float_4 outMod = this->stepMutaOutsComputed[blockIdx];
            this->hits[blockIdx] = ((inMod <= phase) ^ (phase < outMod) ^ (inMod < outMod))
                                   & (this->stepGates[blockIdx] ^ createMask(globalGate ? 0b0000 : 0b1111));
        }
    }

    void findCleanStepsForPhase(float phase, bool globalGate) {
        // Search mutated steps for phase-step intersection
        for (unsigned int blockIdx = 0; blockIdx < SIZE / BLOCK_SIZE; blockIdx++) {
            simd::float_4 inMod = this->stepInsComputed[blockIdx];
            simd::float_4 outMod = this->stepOutsComputed[blockIdx];
            this->hitsClean[blockIdx] = ((inMod <= phase) ^ (phase < outMod) ^ (inMod < outMod))
                                        & (this->stepGates[blockIdx] ^ createMask(globalGate ? 0b0000 : 0b1111));
        }
    }

    void findMonoStep() {
        this->lastActiveStepIdx = this->activeStepIdx;
        for (unsigned int blockOffset = 0; blockOffset < SIZE / BLOCK_SIZE; blockOffset++) {
            unsigned int blockIdx = SIZE / BLOCK_SIZE - blockOffset - 1;
            int blockMask = simd::movemask(this->hits[blockIdx]);

            if (blockMask == 0) {
                // No hits in this block
                continue;
            }

            for (unsigned int stepInBlockOffset = 0; stepInBlockOffset < BLOCK_SIZE; stepInBlockOffset++) {
                unsigned int stepInBlockIdx = BLOCK_SIZE - stepInBlockOffset - 1;
                int maskToCompare = 1 << stepInBlockIdx;
                if (blockMask & maskToCompare) {
                    this->hasActiveStep = true;
                    this->activeStepIdx = blockIdx * BLOCK_SIZE + stepInBlockIdx;
                    this->activeStepInBlockIdx = stepInBlockIdx;
                    this->activeBlockMask = 1 << stepInBlockIdx;
                    this->activeBlockIdx = blockIdx;
                    return;
                }
            }
        }
        this->hasActiveStep = false;
    }

    void applyMutations(unsigned int attrIdx, unsigned int blockIdx) {
        std::pair<float, float> bounds = this->attrBounds[attrIdx];
        this->stepBasesMutated[attrIdx][blockIdx] = simd::clamp(
            this->stepBases[attrIdx][blockIdx] + this->stepMutas[attrIdx][blockIdx], bounds.first, bounds.second);
    }

    void recalcInOuts(unsigned int blockIdx) {
        this->stepInsComputed[blockIdx] = eucMod(
            this->stepIns[blockIdx] + this->stepBases[StepAttr::STEP_SHIFT][blockIdx] + this->shift + this->globalShift,
            1.f);

        this->stepOutsComputed[blockIdx] = eucMod(
            this->stepInsComputed[blockIdx] + this->stepBases[StepAttr::STEP_LEN][blockIdx] * this->globalLen, 1.f);

        this->stepMutaInsComputed[blockIdx] =
            eucMod(this->stepIns[blockIdx] + this->stepBasesMutated[StepAttr::STEP_SHIFT][blockIdx] + this->shift
                       + this->globalShift,
                   1.f);

        this->stepMutaOutsComputed[blockIdx] =
            eucMod(this->stepMutaInsComputed[blockIdx]
                       + this->stepBasesMutated[StepAttr::STEP_LEN][blockIdx] * this->globalLen,
                   1.f);
    }

    json_t* dataToJson() {
        json_t* patternJ = json_object();
        json_object_set_new(patternJ, "resolution", json_integer(resolution));
        json_object_set_new(patternJ, "goTo",
                            json_integer(goTo + 1)); // +1 for compatibility with v1.1.2
        json_object_set_new(patternJ, "shift", json_real(shift));
        json_t* stepsJ = json_array();

        for (unsigned int stepIdx = 0; stepIdx < SIZE; stepIdx++) {
            unsigned int blockIdx = stepIdx / BLOCK_SIZE;
            unsigned int stepInBlockIdx = stepIdx % BLOCK_SIZE;

            json_t* stepJ = json_object();

            json_object_set_new(stepJ, "gate",
                                json_boolean(bool(simd::movemask(this->stepGates[blockIdx]) & (1 << stepInBlockIdx))));

            json_t* attrsJ = json_array();

            for (unsigned int attrIdx = 0; attrIdx < STEP_ATTRS_TOTAL; attrIdx++) {
                json_t* attrJ = json_object();
                json_object_set_new(attrJ, "base", json_real(this->stepBases[attrIdx][blockIdx][stepInBlockIdx]));
                json_object_set_new(attrJ, "mutation", json_real(this->stepMutas[attrIdx][blockIdx][stepInBlockIdx]));
                json_array_append(attrsJ, attrJ);
            }

            // idx and in_ are for compatibility with v1.1.2
            json_object_set_new(stepJ, "idx", json_integer(stepIdx));
            json_object_set_new(stepJ, "in_", json_real(stepIdx * this->baseStepLen));

            json_object_set_new(stepJ, "attrs", attrsJ);
            json_array_append(stepsJ, stepJ);
        }

        json_object_set_new(patternJ, "steps", stepsJ);
        return patternJ;
    }

    void dataFromJson(json_t* patternJ, bool compatibilityMode) {
        this->resolution = json_number_value(json_object_get(patternJ, "resolution"));
        this->goTo = json_number_value(json_object_get(patternJ, "goTo"));

        if (compatibilityMode) {
            // -1 for compatibility with v1.1.2
            this->goTo -= 1;
        }

        this->shift = json_number_value(json_object_get(patternJ, "shift"));
        json_t* stepsJ = json_object_get(patternJ, "steps");

        for (unsigned int stepIdx = 0; stepIdx < SIZE; stepIdx++) {
            unsigned int blockIdx = stepIdx / BLOCK_SIZE;
            unsigned int stepInBlockIdx = stepIdx % BLOCK_SIZE;
            json_t* stepJ = json_array_get(stepsJ, stepIdx);
            bool gate = json_boolean_value(json_object_get(stepJ, "gate"));

            if (!gate) {
                this->stepGates[blockIdx] ^= createMask(1 << stepInBlockIdx);
            }

            json_t* attrsJ = json_object_get(stepJ, "attrs");

            for (unsigned int attrIdx = 0; attrIdx < STEP_ATTRS_TOTAL; attrIdx++) {
                json_t* attrJ = json_array_get(attrsJ, attrIdx);
                this->stepBases[attrIdx][blockIdx][stepInBlockIdx] = json_number_value(json_object_get(attrJ, "base"));
                this->stepMutas[attrIdx][blockIdx][stepInBlockIdx] =
                    json_number_value(json_object_get(attrJ, "mutation"));
                this->applyMutations(attrIdx, blockIdx);
            }
        }

        for (unsigned int blockIdx = 0; blockIdx < SIZE / BLOCK_SIZE; blockIdx++) {
            this->recalcInOuts(blockIdx);
        }
    }

    bool isClean() {
        if (resolution != 8 || shift != 0.f) {
            return false;
        }

        for (unsigned int attrIdx = 0; attrIdx < STEP_ATTRS_TOTAL; attrIdx++) {
            for (unsigned int blockIdx = 0; blockIdx < SIZE / BLOCK_SIZE; blockIdx++) {
                if (simd::movemask(this->stepGates[blockIdx]) != 0b1111) {
                    return false;
                }

                if (simd::movemask(this->stepBases[attrIdx][blockIdx] != this->stepAttrDefaults[attrIdx])
                    || simd::movemask(this->stepMutas[attrIdx][blockIdx] != 0.f)) {
                    return false;
                }
            }
        }

        return true;
    }

    bool hasCustomSteps() {
        for (unsigned int attrIdx = 0; attrIdx < STEP_ATTRS_TOTAL; attrIdx++) {
            for (unsigned int blockIdx = 0; blockIdx < SIZE / BLOCK_SIZE; blockIdx++) {
                if (simd::movemask(this->stepBases[attrIdx][blockIdx] != this->stepAttrDefaults[attrIdx])
                    || simd::movemask(this->stepMutas[attrIdx][blockIdx] != 0.f)) {
                    return true;
                }
            }
        }
        return false;
    }

    Pattern() {
        this->init();
    }

    void init() {
        this->resolution = SIZE;
        this->shift = 0.f;

        for (unsigned int attrIdx = 0; attrIdx < STEP_ATTRS_TOTAL; attrIdx++) {
            for (unsigned int blockIdx = 0; blockIdx < SIZE / BLOCK_SIZE; blockIdx++) {
                this->stepBases[attrIdx][blockIdx] = this->stepAttrDefaults[attrIdx];
                this->stepMutas[attrIdx][blockIdx] = 0.f;
                this->applyMutations(attrIdx, blockIdx);
            }
        }

        for (unsigned int stepIdx = 0; stepIdx < SIZE; stepIdx++) {
            unsigned int blockIdx = stepIdx / BLOCK_SIZE;
            unsigned int stepInBlockIdx = stepIdx % BLOCK_SIZE;
            this->stepIns[blockIdx][stepInBlockIdx] = 1.f / SIZE * stepIdx;
        }

        for (unsigned int blockIdx = 0; blockIdx < SIZE / BLOCK_SIZE; blockIdx++) {
            this->stepGates[blockIdx] = simd::float_4::mask();
            this->recalcInOuts(blockIdx);
        }
    }

    void randomize() {
        for (unsigned int attrIdx = 0; attrIdx < STEP_ATTRS_TOTAL; attrIdx++) {
            std::pair<float, float> bounds = this->attrBounds[attrIdx];
            float range = bounds.second - bounds.first;

            for (unsigned int stepIdx = 0; stepIdx < SIZE; stepIdx++) {
                unsigned int blockIdx = stepIdx / BLOCK_SIZE;
                unsigned int stepInBlockIdx = stepIdx % BLOCK_SIZE;
                float newValue = bounds.first + random::uniform() * range;
                this->stepBases[attrIdx][blockIdx][stepInBlockIdx] = newValue;
                this->stepMutas[attrIdx][blockIdx][stepInBlockIdx] = 0.f;
            }

            for (unsigned int blockIdx = 0; blockIdx < SIZE / BLOCK_SIZE; blockIdx++) {
                this->applyMutations(attrIdx, blockIdx);
            }
        }
        for (unsigned int blockIdx = 0; blockIdx < SIZE / BLOCK_SIZE; blockIdx++) {
            this->recalcInOuts(blockIdx);
            int randomMask = 0;

            for (unsigned int stepInBlockIdx = 0; stepInBlockIdx < BLOCK_SIZE; stepInBlockIdx++) {
                // 10% chance for disabled steps
                if (random::uniform() > 0.1f) {
                    randomMask += 1 << stepInBlockIdx;
                }
            }
            this->stepGates[blockIdx] = createMask(randomMask);
        }
    }

    void randomizeReso() {
        this->resolution = 1 + std::round(99 * random::uniform());
    }

    void resetResolution() {
        this->resolution = SIZE;
    }

    void quantize() {
        for (unsigned int blockIdx = 0; blockIdx < SIZE / BLOCK_SIZE; blockIdx++) {
            this->stepBases[StepAttr::STEP_SHIFT][blockIdx] =
                simd::trunc(this->stepBases[StepAttr::STEP_SHIFT][blockIdx] * SIZE * 2) / SIZE * 2;
            this->stepMutas[StepAttr::STEP_SHIFT][blockIdx] =
                simd::trunc(this->stepMutas[StepAttr::STEP_SHIFT][blockIdx] * SIZE * 2) / SIZE * 2;
            this->applyMutations(StepAttr::STEP_SHIFT, blockIdx);
            this->recalcInOuts(blockIdx);
        }

        this->shift = 0.f;
    }

private:
    void shiftBy(int delta) {
        int targetShift = (SIZE + SIZE + delta) % SIZE;

        for (unsigned int attrIdx = 0; attrIdx < STEP_ATTRS_TOTAL; attrIdx++) {
            float bases[SIZE] = { 0.f };
            for (unsigned int blockIdx = 0; blockIdx < SIZE / BLOCK_SIZE; blockIdx++) {
                std::copy(this->stepBases[attrIdx][blockIdx].s, this->stepBases[attrIdx][blockIdx].s + BLOCK_SIZE,
                          bases + blockIdx * BLOCK_SIZE);
            }

            std::rotate(&bases[0], &bases[targetShift], &bases[SIZE]);
            for (unsigned int blockIdx = 0; blockIdx < SIZE / BLOCK_SIZE; blockIdx++) {
                std::copy(bases + blockIdx * BLOCK_SIZE, bases + (blockIdx + 1) * BLOCK_SIZE,
                          this->stepBases[attrIdx][blockIdx].s);
            }

            float mutas[SIZE] = { 0.f };
            for (unsigned int blockIdx = 0; blockIdx < SIZE / BLOCK_SIZE; blockIdx++) {
                std::copy(this->stepMutas[attrIdx][blockIdx].s, this->stepMutas[attrIdx][blockIdx].s + BLOCK_SIZE,
                          mutas + blockIdx * BLOCK_SIZE);
            }

            std::rotate(&mutas[0], &mutas[targetShift], &mutas[SIZE]);
            for (unsigned int blockIdx = 0; blockIdx < SIZE / BLOCK_SIZE; blockIdx++) {
                std::copy(mutas + blockIdx * BLOCK_SIZE, mutas + (blockIdx + 1) * BLOCK_SIZE,
                          this->stepMutas[attrIdx][blockIdx].s);
                this->applyMutations(attrIdx, blockIdx);
            }
        }

        float gates[SIZE] = { 0.f };
        for (unsigned int blockIdx = 0; blockIdx < SIZE / BLOCK_SIZE; blockIdx++) {
            std::copy(this->stepGates[blockIdx].s, this->stepGates[blockIdx].s + BLOCK_SIZE,
                      gates + blockIdx * BLOCK_SIZE);
        }

        std::rotate(&gates[0], &gates[targetShift], &gates[SIZE]);
        for (unsigned int blockIdx = 0; blockIdx < SIZE / BLOCK_SIZE; blockIdx++) {
            std::copy(gates + blockIdx * BLOCK_SIZE, gates + (blockIdx + 1) * BLOCK_SIZE, this->stepGates[blockIdx].s);
            this->recalcInOuts(blockIdx);
        }
    }

public:
    void shiftLeft() {
        this->shiftBy(1);
    }

    void shiftRight() {
        this->shiftBy(-1);
    }

    float getShift() {
        return this->shift / this->baseStepLen;
    }

    void setShift(float newShift) {
        this->shift = newShift * this->baseStepLen;
        for (unsigned int blockIdx = 0; blockIdx < SIZE / BLOCK_SIZE; blockIdx++) {
            this->recalcInOuts(blockIdx);
        }
    }

    void applyGlobalShift(float newShift) {
        this->globalShift = newShift;
        for (unsigned int blockIdx = 0; blockIdx < SIZE / BLOCK_SIZE; blockIdx++) {
            this->recalcInOuts(blockIdx);
        }
    }

    void applyGlobalLen(float newLen) {
        this->globalLen = newLen;
        for (unsigned int blockIdx = 0; blockIdx < SIZE / BLOCK_SIZE; blockIdx++) {
            this->recalcInOuts(blockIdx);
        }
    }

    void resetLenghts() {
        for (unsigned int blockIdx = 0; blockIdx < SIZE / BLOCK_SIZE; blockIdx++) {
            this->stepBases[StepAttr::STEP_LEN][blockIdx] = this->stepAttrDefaults[StepAttr::STEP_LEN];
            this->applyMutations(StepAttr::STEP_LEN, blockIdx);
            this->recalcInOuts(blockIdx);
        }
    }

    void mutateBlock(unsigned int blockIdx, simd::float_4 mask, float factorScalar) {
        for (unsigned int attrIdx = 0; attrIdx < STEP_ATTRS_TOTAL; attrIdx++) {
            std::pair<float, float> bounds = this->attrBounds[attrIdx];
            simd::float_4 factor = factorScalar;
            simd::float_4 bases = this->stepBases[attrIdx][blockIdx];
            simd::float_4 mutas = this->stepMutas[attrIdx][blockIdx];
            simd::float_4 basesMutated = this->stepBasesMutated[attrIdx][blockIdx];
            simd::float_4* vctrs = &this->stepMutaVectors[attrIdx][blockIdx];
            simd::float_4 mults = this->attrMutaMultipliers[attrIdx];

            float rnds[BLOCK_SIZE] = { 0.f };
            for (unsigned int i = 0; i < BLOCK_SIZE; i++) {
                rnds[i] = random::uniform();
            }

            simd::float_4 range = bounds.second - bounds.first;
            simd::float_4 mutatedNormalized = (basesMutated - bounds.first) / range * 2.f - 1.f;
            simd::float_4 overflowCaution = simd::trunc(mutatedNormalized * 2.f);
            simd::float_4 antiOverflowFactor = 0.25f;

            simd::float_4 rands = simd::float_4::load(rnds) * 2.f - 1.f;
            simd::float_4 vectorLifeFactor = 0.25f;

            *vctrs = simd::ifelse(
                mask,
                simd::clamp(*vctrs + (rands * vectorLifeFactor) - (overflowCaution * antiOverflowFactor), -1.f, 1.f),
                *vctrs * 0.98f);

            simd::float_4 newMutas =
                simd::ifelse(factor < 0.f, mutas + mutas * factor * 2.f, mutas + *vctrs * factor * mults);
            newMutas = simd::clamp(newMutas, bounds.first - bases, bounds.second - bases);

            this->stepMutas[attrIdx][blockIdx] = simd::ifelse(mask, newMutas, mutas);
            this->applyMutations(attrIdx, blockIdx);
            this->recalcInOuts(blockIdx);
        }
    }

    void mutate(float factor) {
        for (unsigned int blockIdx = 0; blockIdx < SIZE / BLOCK_SIZE; blockIdx++) {
            this->mutateBlock(blockIdx, createMask(0b1111), factor);
        }
    }

    void resetBlockMutation(unsigned int blockIdx, simd::float_4 mask) {
        for (unsigned int attrIdx = 0; attrIdx < STEP_ATTRS_TOTAL; attrIdx++) {
            this->stepMutas[attrIdx][blockIdx] = simd::ifelse(mask, 0.f, this->stepMutas[attrIdx][blockIdx]);
            this->applyMutations(attrIdx, blockIdx);
        }
        this->recalcInOuts(blockIdx);
    }

    void resetMutation() {
        for (unsigned int blockIdx = 0; blockIdx < SIZE / BLOCK_SIZE; blockIdx++) {
            this->resetBlockMutation(blockIdx, createMask(0b1111));
        }
    }

    void reverse() {
        this->shift = -this->shift;
        for (unsigned int attrIdx = 0; attrIdx < STEP_ATTRS_TOTAL; attrIdx++) {
            float newBases[SIZE];
            for (unsigned int blockIdx = 0; blockIdx < SIZE / BLOCK_SIZE; blockIdx++) {
                std::copy(this->stepBases[attrIdx][blockIdx].s, this->stepBases[attrIdx][blockIdx].s + BLOCK_SIZE,
                          newBases + blockIdx * BLOCK_SIZE);
            }

            std::reverse(newBases, newBases + SIZE);
            for (unsigned int blockIdx = 0; blockIdx < SIZE / BLOCK_SIZE; blockIdx++) {
                std::copy(newBases + blockIdx * BLOCK_SIZE, newBases + (blockIdx + 1) * BLOCK_SIZE,
                          this->stepBases[attrIdx][blockIdx].s);
            }

            float newMutas[SIZE];
            for (unsigned int blockIdx = 0; blockIdx < SIZE / BLOCK_SIZE; blockIdx++) {
                std::copy(this->stepMutas[attrIdx][blockIdx].s, this->stepMutas[attrIdx][blockIdx].s + BLOCK_SIZE,
                          newMutas + blockIdx * BLOCK_SIZE);
            }

            std::reverse(newMutas, newMutas + SIZE);
            for (unsigned int blockIdx = 0; blockIdx < SIZE / BLOCK_SIZE; blockIdx++) {
                std::copy(newMutas + blockIdx * BLOCK_SIZE, newMutas + (blockIdx + 1) * BLOCK_SIZE,
                          this->stepMutas[attrIdx][blockIdx].s);
            }
        }
        for (unsigned int blockIdx = 0; blockIdx < SIZE / BLOCK_SIZE; blockIdx++) {
            std::swap(this->stepBases[StepAttr::STEP_EXPR_IN][blockIdx],
                      this->stepBases[StepAttr::STEP_EXPR_OUT][blockIdx]);
            this->stepBases[StepAttr::STEP_SHIFT][blockIdx] = this->baseStepLen
                                                              - this->stepBases[StepAttr::STEP_SHIFT][blockIdx]
                                                              - this->stepBases[StepAttr::STEP_LEN][blockIdx];
            this->stepMutas[StepAttr::STEP_SHIFT][blockIdx] = -this->stepMutas[StepAttr::STEP_SHIFT][blockIdx];
            this->stepBases[StepAttr::STEP_EXPR_POWER][blockIdx] =
                -this->stepBases[StepAttr::STEP_EXPR_POWER][blockIdx];
            this->stepMutas[StepAttr::STEP_EXPR_POWER][blockIdx] =
                -this->stepMutas[StepAttr::STEP_EXPR_POWER][blockIdx];
        }

        for (unsigned int attrIdx = 0; attrIdx < STEP_ATTRS_TOTAL; attrIdx++) {
            for (unsigned int blockIdx = 0; blockIdx < SIZE / BLOCK_SIZE; blockIdx++) {
                this->applyMutations(attrIdx, blockIdx);
            }
        }

        for (unsigned int blockIdx = 0; blockIdx < SIZE / BLOCK_SIZE; blockIdx++) {
            this->recalcInOuts(blockIdx);
        }
    }

    void flip() {
        for (unsigned int blockIdx = 0; blockIdx < SIZE / BLOCK_SIZE; blockIdx++) {
            this->stepBases[StepAttr::STEP_VALUE][blockIdx] = -this->stepBases[StepAttr::STEP_VALUE][blockIdx];
            this->stepMutas[StepAttr::STEP_VALUE][blockIdx] = -this->stepMutas[StepAttr::STEP_VALUE][blockIdx];
            this->applyMutations(StepAttr::STEP_VALUE, blockIdx);
        }
    }

    void bakeMutation() {
        for (unsigned int attrIdx = 0; attrIdx < STEP_ATTRS_TOTAL; attrIdx++) {
            for (unsigned int blockIdx = 0; blockIdx < SIZE / BLOCK_SIZE; blockIdx++) {
                this->stepBases[attrIdx][blockIdx] = this->stepBasesMutated[attrIdx][blockIdx];
                this->stepMutas[attrIdx][blockIdx] = 0.f;
                this->applyMutations(attrIdx, blockIdx);
            }
        }

        for (unsigned int blockIdx = 0; blockIdx < SIZE / BLOCK_SIZE; blockIdx++) {
            this->recalcInOuts(blockIdx);
        }
    }
};
