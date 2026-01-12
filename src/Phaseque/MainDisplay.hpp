#pragma once
#include "Pattern.hpp"
#include "Phaseque.hpp"

#include "../widgets.hpp"

struct MainDisplayConsumer {
    float phase = 0.f;
    int direction = 1;
    Pattern<8> pattern;
    bool globalGate = false;
    int polyphonyMode = PolyphonyModes::MONOPHONIC;
    float globalPower = 0.f;
    float globalCurve = 0.f;

    bool consumed = false;

    MainDisplayConsumer() {
        this->pattern.init();
    }
};

struct MainDisplayWidget : BaseDisplayWidget {
    NVGcolor lcdGhostColor = nvgRGB(0x1e, 0x1f, 0x1d);

    NVGcolor lcdActiveColor = nvgRGB(0xff, 0xd4, 0x2a);
    NVGcolor lcdDimmedColor = nvgRGB(0xa0, 0x80, 0x00);
    NVGcolor lcdDisabledColor = nvgRGB(0x36, 0x2b, 0x00);

    NVGcolor lcdActiveMutColor = nvgRGB(0x2e, 0xc6, 0xff);
    NVGcolor lcdDimmedMutColor = nvgRGB(0x32, 0x95, 0xcc);
    NVGcolor lcdDisabledMutColor = nvgRGB(0x30, 0x4e, 0x5e);

    std::shared_ptr<Font> font;

    std::shared_ptr<MainDisplayConsumer> consumer;

    float padding = 6.0f;
    Vec area;
    float crossLine1;
    float dashLine1;
    float crossLine2;
    float dashLine2;
    float crossLine3;
    float exprHorizont;
    float stepX;
    float stepY;
    bool ready = false;

    MainDisplayWidget() {
        font = APP->window->loadFont(asset::plugin(pluginInstance, "res/fonts/Nunito/Nunito-Black.ttf"));
        consumer = std::make_shared<MainDisplayConsumer>();
    }

    void setupSizes() {
        area = Vec(box.size.x - padding * 2.0f, box.size.y - padding * 2.0f);
        stepX = (area.x - 1) / NUM_STEPS;
        stepY = (area.y) / 6;
        crossLine1 = stepY * 1.0f;
        dashLine1 = stepY * 2.0f;
        crossLine2 = stepY * 3.0f;
        dashLine2 = stepY * 4.0f;
        crossLine3 = stepY * 5.0f;
        exprHorizont = crossLine3 + stepY / 2.0f;
        ready = true;
    }

    void drawDash(const DrawArgs& args, Vec position) {
        nvgStrokeColor(args.vg, lcdGhostColor);
        nvgStrokeWidth(args.vg, 1.0f);

        nvgBeginPath(args.vg);
        nvgMoveTo(args.vg, position.x - 5.5f, position.y);
        nvgLineTo(args.vg, position.x + 5.5f, position.y);
        nvgStroke(args.vg);
    }

    void drawCross(const DrawArgs& args, Vec position) {
        drawDash(args, position);

        nvgBeginPath(args.vg);
        nvgMoveTo(args.vg, position.x, position.y - 5.5f);
        nvgLineTo(args.vg, position.x, position.y + 5.5f);
        nvgStroke(args.vg);
    }

    void drawResolution(const DrawArgs& args) {
        float resolutionVal = this->consumer->pattern.resolution;
        float phaseVal = this->consumer->phase;

        nvgStrokeWidth(args.vg, 1.0f);
        float periodSize = area.x / resolutionVal;

        for (float i = 0.0f; i < resolutionVal; i = i + 1.0f) {
            if (phaseVal * resolutionVal >= i && phaseVal * resolutionVal < i + 1) {
                nvgStrokeColor(args.vg, lcdActiveColor);
            } else {
                nvgStrokeColor(args.vg, lcdDisabledColor);
            }

            nvgBeginPath(args.vg);
            nvgMoveTo(args.vg, padding + periodSize * i, padding + crossLine1);
            nvgLineTo(args.vg, padding + periodSize * (i + 1), padding);
            nvgStroke(args.vg);
        }
    }

    void drawExprCurve(const DrawArgs& args, float x1, float workArea, unsigned int stepIdx, bool mutated) {
        unsigned int blockIdx = stepIdx / this->consumer->pattern.blockSize;
        unsigned int stepInBlockIdx = stepIdx % this->consumer->pattern.blockSize;

        if (mutated) {
            float mutationStrength = 0.f;
            mutationStrength +=
                std::abs(this->consumer->pattern.stepMutas[StepAttr::STEP_EXPR_IN][blockIdx][stepInBlockIdx]);
            mutationStrength +=
                std::abs(this->consumer->pattern.stepMutas[StepAttr::STEP_EXPR_OUT][blockIdx][stepInBlockIdx]);
            mutationStrength +=
                std::abs(this->consumer->pattern.stepMutas[StepAttr::STEP_EXPR_POWER][blockIdx][stepInBlockIdx]);
            mutationStrength +=
                std::abs(this->consumer->pattern.stepMutas[StepAttr::STEP_EXPR_CURVE][blockIdx][stepInBlockIdx]);
            nvgGlobalAlpha(args.vg, std::min(mutationStrength * 0.5f, 1.f));
        }

        float len = workArea
                    * ((mutated ? this->consumer->pattern.stepBasesMutated
                                : this->consumer->pattern.stepBases)[StepAttr::STEP_LEN][blockIdx][stepInBlockIdx])
                    * this->consumer->pattern.globalLen;
        nvgBeginPath(args.vg);

        unsigned int curveReso = 24;
        for (unsigned int i = 0; i <= curveReso; i++) {
            if (i % 2 != 0) {
                continue;
            }

            float phase = float(i) / float(curveReso);
            float cx = x1 + len * phase;
            float expressions[4];

            getBlockExpressions((mutated ? this->consumer->pattern.stepBasesMutated
                                         : this->consumer->pattern.stepBases)[StepAttr::STEP_EXPR_IN][blockIdx],
                                (mutated ? this->consumer->pattern.stepBasesMutated
                                         : this->consumer->pattern.stepBases)[StepAttr::STEP_EXPR_OUT][blockIdx],
                                (mutated ? this->consumer->pattern.stepBasesMutated
                                         : this->consumer->pattern.stepBases)[StepAttr::STEP_EXPR_POWER][blockIdx],
                                (mutated ? this->consumer->pattern.stepBasesMutated
                                         : this->consumer->pattern.stepBases)[StepAttr::STEP_EXPR_CURVE][blockIdx],
                                simd::float_4(phase), this->consumer->globalPower, this->consumer->globalCurve)
                .store(expressions);

            float cy = expressions[stepInBlockIdx];

            if (i == 0) {
                nvgMoveTo(args.vg, cx, exprHorizont + 6 + stepY * cy * -0.5);
                continue;
            }

            float prevPhase = float(i - 1) / float(curveReso);
            float prevCx = x1 + len * prevPhase;
            float prevExpressions[4];

            getBlockExpressions((mutated ? this->consumer->pattern.stepBasesMutated
                                         : this->consumer->pattern.stepBases)[StepAttr::STEP_EXPR_IN][blockIdx],
                                (mutated ? this->consumer->pattern.stepBasesMutated
                                         : this->consumer->pattern.stepBases)[StepAttr::STEP_EXPR_OUT][blockIdx],
                                (mutated ? this->consumer->pattern.stepBasesMutated
                                         : this->consumer->pattern.stepBases)[StepAttr::STEP_EXPR_POWER][blockIdx],
                                (mutated ? this->consumer->pattern.stepBasesMutated
                                         : this->consumer->pattern.stepBases)[StepAttr::STEP_EXPR_CURVE][blockIdx],
                                simd::float_4(prevPhase), this->consumer->globalPower, this->consumer->globalCurve)
                .store(prevExpressions);

            float prevCy = prevExpressions[stepInBlockIdx];

            nvgQuadTo(args.vg, prevCx, exprHorizont + 6 + stepY * prevCy * -0.5, cx,
                      exprHorizont + 6 + stepY * cy * -0.5);
        }

        nvgStroke(args.vg);
    }

    void drawStep(const DrawArgs& args, unsigned int stepIdx, bool mutated) {
        unsigned int blockIdx = stepIdx / this->consumer->pattern.blockSize;
        unsigned int stepInBlockIdx = stepIdx % this->consumer->pattern.blockSize;

        if (mutated) {
            float mutationStrength = 0.f;
            mutationStrength +=
                std::abs(this->consumer->pattern.stepMutas[StepAttr::STEP_VALUE][blockIdx][stepInBlockIdx]);
            mutationStrength +=
                std::abs(this->consumer->pattern.stepMutas[StepAttr::STEP_LEN][blockIdx][stepInBlockIdx]);
            mutationStrength +=
                std::abs(this->consumer->pattern.stepMutas[StepAttr::STEP_SHIFT][blockIdx][stepInBlockIdx]);
            nvgGlobalAlpha(args.vg, std::min(mutationStrength * 3.f, 1.f));
        } else {
            nvgGlobalAlpha(args.vg, 1.f);
        }

        int dir = this->consumer->direction;
        float radius = 2.5f;
        float workArea = area.x - 1;
        float start = padding;
        float end = padding + workArea;

        float x1 = padding
                   + workArea
                         * (mutated ? this->consumer->pattern.stepMutaInsComputed
                                    : this->consumer->pattern.stepInsComputed)[blockIdx][stepInBlockIdx];

        float x2 = padding
                   + workArea
                         * (mutated ? this->consumer->pattern.stepMutaOutsComputed
                                    : this->consumer->pattern.stepOutsComputed)[blockIdx][stepInBlockIdx];

        float lineX1 = x1 + (dir == 1 ? radius * 1.8f : 0.f);
        float lineX2 = x2 - (dir == -1 ? radius * 1.8f : 0.f);

        float y = padding + crossLine2
                  + -stepY
                        * (mutated ? this->consumer->pattern.stepBasesMutated
                                   : this->consumer->pattern.stepBases)[StepAttr::STEP_VALUE][blockIdx][stepInBlockIdx];

        // Step
        if (x2 >= x1) {
            nvgBeginPath(args.vg);
            nvgMoveTo(args.vg, lineX1, y);
            nvgLineTo(args.vg, lineX2, y);
            nvgStroke(args.vg);
        } else {
            nvgBeginPath(args.vg);
            nvgMoveTo(args.vg, lineX1, y);
            nvgLineTo(args.vg, end, y);
            nvgStroke(args.vg);
            nvgBeginPath(args.vg);
            nvgMoveTo(args.vg, start, y);
            nvgLineTo(args.vg, lineX2, y);
            nvgStroke(args.vg);
        }

        // Circle
        nvgBeginPath(args.vg);
        nvgCircle(args.vg, dir == 1 ? x1 + 2 : fastmod(x2 - 2 - padding, area.x) + padding, y, radius);
        nvgFill(args.vg);

        // Expression curve
        this->drawExprCurve(args, x1, workArea, stepIdx, mutated);
        if (x2 < x1) {
            this->drawExprCurve(args, x1 - workArea - 1, workArea, stepIdx, mutated);
        }
    }

    void drawSteps(const DrawArgs& args) {
        nvgStrokeWidth(args.vg, 1.0f);
        nvgScissor(args.vg, padding - 1, padding - 1, area.x + 2, area.y + 2);
        nvgGlobalCompositeOperation(args.vg, NVG_LIGHTER);

        for (unsigned int stepIdx = 0; stepIdx < this->consumer->pattern.size; stepIdx++) {
            unsigned int blockIdx = stepIdx / 4;
            unsigned int stepInBlockIdx = stepIdx % 4;

            bool gate = bool(simd::movemask(this->consumer->pattern.stepGates[blockIdx]) & (1 << stepInBlockIdx))
                        ^ !this->consumer->globalGate;

            bool active = false;

            if (this->consumer->polyphonyMode == PolyphonyModes::MONOPHONIC) {
                active = this->consumer->pattern.hasActiveStep && (this->consumer->pattern.activeStepIdx == stepIdx);
            } else if (this->consumer->polyphonyMode == PolyphonyModes::POLYPHONIC) {
                active = simd::movemask(this->consumer->pattern.hits[blockIdx]) & 1 << stepInBlockIdx;
            } else if (this->consumer->polyphonyMode == PolyphonyModes::UNISON) {
                active = simd::movemask(this->consumer->pattern.hitsClean[blockIdx]) & 1 << stepInBlockIdx;
            }

            if (active) {
                nvgStrokeColor(args.vg, lcdActiveColor);
                nvgFillColor(args.vg, lcdActiveColor);
            } else if (gate) {
                nvgStrokeColor(args.vg, lcdDimmedColor);
                nvgFillColor(args.vg, lcdDimmedColor);
            } else {
                nvgStrokeColor(args.vg, lcdDisabledColor);
                nvgFillColor(args.vg, lcdDisabledColor);
            }

            this->drawStep(args, stepIdx, false);

            if (this->consumer->polyphonyMode == PolyphonyModes::UNISON) {
                active = simd::movemask(this->consumer->pattern.hits[blockIdx]) & 1 << stepInBlockIdx;
            }

            if (active) {
                nvgStrokeColor(args.vg, lcdActiveMutColor);
                nvgFillColor(args.vg, lcdActiveMutColor);
            } else if (gate) {
                nvgStrokeColor(args.vg, lcdDimmedMutColor);
                nvgFillColor(args.vg, lcdDimmedMutColor);
            } else {
                nvgStrokeColor(args.vg, lcdDisabledMutColor);
                nvgFillColor(args.vg, lcdDisabledMutColor);
            }

            this->drawStep(args, stepIdx, true);
        }

        //   int polyphonyMode = this->consumer->polyphonyMode;

        //   for (int i = 0; i < NUM_STEPS; i++) {
        //     Step *step = &this->consumer->pattern.steps[i];

        //     if (!step->gate ^ !globalGate) {
        //       nvgStrokeColor(args.vg, lcdDisabledColor);
        //       nvgFillColor(args.vg, lcdDisabledColor);
        //       nvgGlobalAlpha(args.vg, 1.f);
        //       drawStep(args, step, false);
        //       if (step->mutationStrength != 0.f) {
        //         nvgStrokeColor(args.vg, lcdDisabledMutColor);
        //         nvgFillColor(args.vg, lcdDisabledMutColor);
        //         nvgGlobalAlpha(args.vg,
        //         std::min(step->mutationStrength, 1.f)); drawStep(args, step,
        //         true);
        //       }
        //     }
        //     if (step->gate ^ !globalGate) {
        //       nvgStrokeColor(args.vg, lcdDimmedColor);
        //       nvgFillColor(args.vg, lcdDimmedColor);
        //       nvgGlobalAlpha(args.vg, 1.f);
        //       drawStep(args, step, false);
        //       if (step->mutationStrength != 0.f) {
        //         nvgStrokeColor(args.vg, lcdDimmedMutColor);
        //         nvgFillColor(args.vg, lcdDimmedMutColor);
        //         nvgGlobalAlpha(args.vg,
        //         std::min(step->mutationStrength, 1.f)); drawStep(args, step,
        //         true);
        //       }
        //     }
        //     nvgStrokeColor(args.vg, lcdActiveColor);
        //     nvgFillColor(args.vg, lcdActiveColor);
        //     nvgGlobalAlpha(args.vg, 1.f);
        //     if (polyphonyMode == PolyphonyModes::POLYPHONIC) {
        //       if (this->consumer->stepsStates[i]) {
        //         drawStep(args, step, false);
        //         if (step->mutationStrength != 0.f) {
        //           nvgStrokeColor(args.vg, lcdActiveMutColor);
        //           nvgFillColor(args.vg, lcdActiveMutColor);
        //           nvgGlobalAlpha(args.vg,
        //           std::min(step->mutationStrength, 1.f)); drawStep(args,
        //           step, true);
        //         }
        //       }
        //     } else if (polyphonyMode == PolyphonyModes::UNISON) {
        //       if (this->consumer->unisonStates[i]) {
        //         drawStep(args, step, false);
        //       }
        //       if (this->consumer->stepsStates[i]) {
        //         if (step->mutationStrength != 0.f) {
        //           nvgStrokeColor(args.vg, lcdActiveMutColor);
        //           nvgFillColor(args.vg, lcdActiveMutColor);
        //           nvgGlobalAlpha(args.vg,
        //           std::min(step->mutationStrength, 1.f)); drawStep(args,
        //           step, true);
        //         }
        //       }
        //     }
        //   }
        //   if (polyphonyMode == PolyphonyModes::MONOPHONIC &&
        //   this->consumer->hasActiveStep) {
        //     drawStep(args, &this->consumer->activeStep, false);
        //     if (this->consumer->activeStep.mutationStrength != 0.f) {
        //       nvgStrokeColor(args.vg, lcdActiveMutColor);
        //       nvgFillColor(args.vg, lcdActiveMutColor);
        //       nvgGlobalAlpha(args.vg,
        //       std::min(this->consumer->activeStep.mutationStrength, 1.f));
        //       drawStep(args, &this->consumer->activeStep, true);
        //     }
        //   }

        nvgResetScissor(args.vg);
        nvgGlobalCompositeOperation(args.vg, NVG_SOURCE_OVER);
    }

    void drawLayer(const DrawArgs& args, int layer) override {
        if (layer != 1) {
            drawBackground(args);
        }

        for (int i = 1; i < NUM_STEPS; i++) {
            drawDash(args, Vec(padding + i * stepX, padding + dashLine1));
            drawCross(args, Vec(padding + i * stepX, padding + crossLine1));
            drawDash(args, Vec(padding + i * stepX, padding + dashLine2));
            drawCross(args, Vec(padding + i * stepX, padding + crossLine2));
            drawCross(args, Vec(padding + i * stepX, padding + crossLine3));
        }

        drawResolution(args);
        drawSteps(args);

        this->consumer->consumed = true;
    }
};
