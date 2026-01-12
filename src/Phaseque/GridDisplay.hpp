#pragma once
#include <bitset>
#include <rack.hpp>
#include "Phaseque.hpp"
#ifndef WIDGETS_H
#    define WIDGETS_H
#    include "../widgets.hpp"
#endif

#include "Phaseque.hpp"

struct GridDisplayConsumer {
    unsigned int currentPattern = 0;
    unsigned int currentPatternGoTo = 1;
    std::bitset<NUM_PATTERNS> dirtyMask;
    bool consumed = false;
    int patternFlashPos = -1;
    int patternFlashNeg = -1;
};

struct GridDisplayProducer {
    unsigned int goToRequest = 0;
    bool hasGoToRequest = false;

    unsigned int nextPatternRequest = 0;
    bool hasNextPatternRequest = false;
    int patternFlashPos = 0;
    int patternFlashNeg = 0;

    unsigned int copyPatternSourceRequest = 0;
    unsigned int copyPatternTargetRequest = 0;
    bool hasCopyPatternRequest = false;

    unsigned int randomizePatternRequest = 0;
    bool hasRandomizePatternRequest = false;
};

struct GridDisplay : BaseDisplayWidget {
    NVGcolor black = nvgRGB(0x00, 0x00, 0x00);
    NVGcolor white = nvgRGB(0xff, 0xff, 0xff);
    NVGcolor lcdActiveColor = nvgRGB(0xff, 0xd4, 0x2a);
    NVGcolor lcdDimmedColor = nvgRGB(0xa0, 0x80, 0x00);
    NVGcolor lcdDisabledColor = nvgRGB(0x36, 0x2b, 0x00);
    NVGcolor negColor = nvgRGB(0xe7, 0x34, 0x2d);
    NVGcolor posColor = nvgRGB(0x9c, 0xd7, 0x43);

    std::shared_ptr<Font> font;

    float patSize = 17.0f;
    float gapSize = 3.0f;
    float padding = 4.0f;
    float flashes[NUM_PATTERNS] = { 0.f };

    std::shared_ptr<GridDisplayConsumer> consumer;

    GridDisplay() {
        font = APP->window->loadFont(asset::plugin(pluginInstance, "res/fonts/Nunito/Nunito-Bold.ttf"));
    }

    void drawPattern(const DrawArgs& args, bool hasCustomSteps, int idx, int currentPatternGoTo, int currentIdxVal) {
        float x = padding + ((idx) % 4) * (patSize + gapSize) + (idx > 15 ? (patSize + gapSize) * 4 : 0.0f);
        float y = box.size.y - ((idx) % 16) / 4 * (patSize + gapSize) - (patSize) -padding;

        bool isClean = !hasCustomSteps;

        if (idx == currentIdxVal) {
            nvgBeginPath(args.vg);
            nvgRoundedRect(args.vg, x, y, patSize, patSize, 1.0);
            nvgFillColor(args.vg, lcdActiveColor);
            nvgFill(args.vg);
        } else if (idx == currentPatternGoTo) {
            nvgBeginPath(args.vg);
            nvgRoundedRect(args.vg, x + 0.5, y + 0.5, patSize - 1, patSize - 1, 1.0);
            nvgFillColor(args.vg, lcdDisabledColor);
            nvgStrokeColor(args.vg, lcdActiveColor);
            if (!isClean) {
                nvgFill(args.vg);
            }
            nvgStroke(args.vg);
        } else if (!isClean) {
            nvgBeginPath(args.vg);
            nvgRoundedRect(args.vg, x, y, patSize, patSize, 1.0);
            nvgFillColor(args.vg, lcdDisabledColor);
            nvgFill(args.vg);
        }

        if (this->flashes[idx] != 0.f) {
            NVGcolor color = this->flashes[idx] > 0.f ? posColor : negColor;
            nvgGlobalCompositeOperation(args.vg, NVG_LIGHTER);
            nvgGlobalAlpha(args.vg, std::min(std::abs(this->flashes[idx]), 1.f));
            nvgBeginPath(args.vg);
            nvgRoundedRect(args.vg, x, y, patSize, patSize, 1.0);
            nvgFillColor(args.vg, color);
            nvgFill(args.vg);
            this->flashes[idx] *= 0.9f;
        }

        nvgGlobalCompositeOperation(args.vg, NVG_SOURCE_OVER);
        nvgGlobalAlpha(args.vg, 1.f);

        Vec textPos = Vec(x + patSize / 2.0f, y + patSize / 2.0f + 2.5f);

        if (idx == currentIdxVal) {
            nvgFillColor(args.vg, black);
        } else if (idx == currentPatternGoTo) {
            nvgFillColor(args.vg, lcdActiveColor);
        } else {
            nvgFillColor(args.vg, isClean ? lcdDimmedColor : lcdActiveColor);
        }

        std::string humanString = std::to_string(idx + 1);
        nvgText(args.vg, textPos.x, textPos.y, humanString.c_str(), NULL);
    }

    void drawLayer(const DrawArgs& args, int layer) override {
        if (layer != 1) {
            this->drawBackground(args);
        }

        if (this->consumer->patternFlashNeg != -1) {
            this->flashes[this->consumer->patternFlashNeg] = -1.f;
        }

        if (this->consumer->patternFlashPos != -1) {
            this->flashes[this->consumer->patternFlashPos] = 1.f;
        }

        nvgFontSize(args.vg, 9);
        nvgFontFaceId(args.vg, font->handle);
        nvgTextAlign(args.vg, NVG_ALIGN_CENTER);
        nvgTextLetterSpacing(args.vg, -1.0);

        for (unsigned int i = 0; i < NUM_PATTERNS; i++) {
            drawPattern(args, this->consumer->dirtyMask.test(i), i, this->consumer->currentPatternGoTo,
                        this->consumer->currentPattern);
        }
    }
};

struct GridDisplayWidget : widget::OpaqueWidget {
    float patSize = 17.0f;
    float gapSize = 3.0f;
    float padding = 4.0f;

    std::shared_ptr<GridDisplayConsumer> consumer;
    std::shared_ptr<GridDisplayProducer> producer;
    unsigned int copyPatternSource;
    bool hasCopyPatternSource = false;

    widget::FramebufferWidget* fb;
    GridDisplay* pd;

    GridDisplayWidget() {
        consumer = std::make_shared<GridDisplayConsumer>();
        producer = std::make_shared<GridDisplayProducer>();

        this->fb = new widget::FramebufferWidget;
        this->addChild(this->fb);

        this->pd = new GridDisplay;
        this->pd->consumer = this->consumer;
        this->fb->addChild(this->pd);
    }

    void setupSize(Vec size) {
        this->pd->setSize(size);
        this->fb->setSize(size);
        this->setSize(size);
    }

    void step() override {
        if (!this->consumer->consumed) {
            this->fb->dirty = true;
        }

        Widget::step();
        this->consumer->consumed = true;

        if (this->consumer->patternFlashNeg != -1) {
            this->producer->patternFlashNeg = -1;
        }

        if (this->consumer->patternFlashPos != -1) {
            this->producer->patternFlashPos = -1;
        }
    }

    void onButton(const event::Button& e) override {
        int button = e.button;

        if ((button == GLFW_MOUSE_BUTTON_LEFT || button == GLFW_MOUSE_BUTTON_RIGHT) && e.action == GLFW_PRESS) {
            e.consume(this);
        } else {
            return;
        }

        float x = e.pos.x;
        float y = e.pos.y;

        float ix = clamp(floorf((x - padding) / (patSize + gapSize)), 0.0f, 7.0f);
        float iy = clamp(floorf((box.size.y - y - padding) / (patSize + gapSize)), 0.0f, 3.0f);

        float targetIdx = fmodf(ix, 4.0f) + (iy * 4.0f) + (ix >= 4.0f ? 16.0f : 0.0f);

        unsigned int targetIdxInt = clamp((unsigned int) targetIdx, 0, NUM_PATTERNS - 1);

        if (button == GLFW_MOUSE_BUTTON_LEFT) {
            if (this->producer->hasGoToRequest) {
                return;
            }
            this->producer->goToRequest = targetIdxInt;
            this->producer->hasGoToRequest = true;
            this->hasCopyPatternSource = false;
        } else if (button == GLFW_MOUSE_BUTTON_RIGHT) {
            createContextMenu(targetIdxInt);
        }
    }

    void createContextMenu(unsigned int targetIdxInt) {
        ui::Menu *menu = createMenu();
        menu->addChild(createMenuLabel(string::f("Pattern %i", targetIdxInt + 1)));
        menu->addChild(new MenuSeparator);

        menu->addChild(createMenuItem("Set As Next Pattern", "", [=] {
            if (this->producer->hasNextPatternRequest) {
                return;
            }
            this->producer->nextPatternRequest = targetIdxInt;
            this->producer->hasNextPatternRequest = true;
        }));

        menu->addChild(new MenuSeparator);

        menu->addChild(createMenuItem(string::f("Copy To Next Pattern %i", this->consumer->currentPatternGoTo + 1), "", [=] {
            if (this->producer->hasCopyPatternRequest) {
                return;
            }
            this->producer->copyPatternSourceRequest = targetIdxInt;
            this->producer->copyPatternTargetRequest = this->consumer->currentPatternGoTo;
            this->producer->hasCopyPatternRequest = true;
        }));

        if ((unsigned int) eucMod(targetIdxInt + 1, NUM_PATTERNS) != this->consumer->currentPatternGoTo) {
            menu->addChild(createMenuItem(string::f("Copy To Pattern %i", eucMod(targetIdxInt + 1, NUM_PATTERNS) + 1), "", [=] {
                if (this->producer->hasCopyPatternRequest) {
                    return;
                }
                this->producer->copyPatternSourceRequest = targetIdxInt;
                this->producer->copyPatternTargetRequest = eucMod(targetIdxInt + 1, NUM_PATTERNS);
                this->producer->hasCopyPatternRequest = true;
            }));
        }

        menu->addChild(createMenuItem("Copy", "", [=] {
            this->copyPatternSource = targetIdxInt;
            this->hasCopyPatternSource = true;
        }));

        if (this->hasCopyPatternSource && this->copyPatternSource != targetIdxInt) {
            menu->addChild(createMenuItem(string::f("Paste From Pattern %i", this->copyPatternSource + 1), "", [=] {
                if (this->producer->hasCopyPatternRequest) {
                    return;
                }
                this->producer->copyPatternSourceRequest = this->copyPatternSource;
                this->producer->copyPatternTargetRequest = targetIdxInt;
                this->producer->hasCopyPatternRequest = true;
            }));
        }

        menu->addChild(new MenuSeparator);

        menu->addChild(createMenuItem("Randomize", "", [=] {
            if (this->producer->hasRandomizePatternRequest) {
                return;
            }
            this->producer->randomizePatternRequest = targetIdxInt;
            this->producer->hasRandomizePatternRequest = true;
        }));
    }
};
