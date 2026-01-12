#pragma once
#include "Phaseque.hpp"

struct PhasequeWidget : ModuleWidget {
    PhasequeWidget(Phaseque* module);
    void appendContextMenu(Menu* menu) override;
};

struct PhasequeCopyToNextItem : MenuItem {
    Phaseque* phaseq;

    void onAction(const event::Action& e) override {
        phaseq->copyToNext();
    }
};

struct PhasequeCopyToPrevItem : MenuItem {
    Phaseque* phaseq;

    void onAction(const event::Action& e) override {
        phaseq->copyToPrev();
    }
};

struct PhasequeCopyResoItem : MenuItem {
    Phaseque* phaseq;

    void onAction(const event::Action& e) override {
        phaseq->copyResoToAll();
    }
};

struct PhasequeRndAllItem : MenuItem {
    Phaseque* phaseq;

    void onAction(const event::Action& e) override {
        phaseq->randomizeAll();
    }
};

struct PhasequeRndAllResoItem : MenuItem {
    Phaseque* phaseq;

    void onAction(const event::Action& e) override {
        phaseq->randomizeAllReso();
    }
};

struct PhasequeBakeMutationItem : MenuItem {
    Phaseque* phaseq;

    void onAction(const event::Action& e) override {
        phaseq->bakeMutation();
    }
};

struct PhasequeClearPatternItem : MenuItem {
    Phaseque* phaseq;

    void onAction(const event::Action& e) override {
        phaseq->pattern.init();
    }
};

struct PolyModeValueItem : MenuItem {
    Phaseque* module;
    PolyphonyModes polyMode;

    void onAction(const event::Action& e) override {
        module->setPolyMode(polyMode);
    }
};

struct PolyModeItem : MenuItem {
    Phaseque* module;

    Menu* createChildMenu() override {
        Menu* menu = new Menu;

        std::vector<std::string> polyModeNames = { "Monophonic", "Polyphonic 8", "Muta-Unison 16" };

        for (int i = 0; i < NUM_POLYPHONY_MODES; i++) {
            PolyphonyModes polyMode = (PolyphonyModes) i;
            PolyModeValueItem* item = new PolyModeValueItem;
            item->text = polyModeNames[i];
            item->rightText = CHECKMARK(module->polyphonyMode == polyMode);
            item->module = module;
            item->polyMode = polyMode;
            menu->addChild(item);
        }
        return menu;
    }
};

struct ExternalCVModeCompatibleOptionItem : MenuItem {
    Phaseque* module;

    void onAction(const event::Action& e) override {
        module->useCompatibleBPMCV = true;
        module->params[Phaseque::USE_COMPATIBLE_BPM_CV_PARAM].setValue(1.f);
    }
};

struct ExternalCVModeVBPSOptionItem : MenuItem {
    Phaseque* module;

    void onAction(const event::Action& e) override {
        module->useCompatibleBPMCV = false;
        module->params[Phaseque::USE_COMPATIBLE_BPM_CV_PARAM].setValue(0.f);
    }
};

struct SnapCVOptionItem : MenuItem {
    Phaseque* module;

    void onAction(const event::Action& e) override {
        module->snapCV ^= true;
    }
};

struct ExternalCVModeItem : MenuItem {
    Phaseque* module;

    Menu* createChildMenu() override {
        Menu* menu = new Menu;

        ExternalCVModeCompatibleOptionItem* item1 = new ExternalCVModeCompatibleOptionItem;
        item1->text = "V/OCT";
        item1->rightText = CHECKMARK(module->useCompatibleBPMCV);
        item1->module = module;
        menu->addChild(item1);

        ExternalCVModeVBPSOptionItem* item2 = new ExternalCVModeVBPSOptionItem;
        item2->text = "V/BPS";
        item2->rightText = CHECKMARK(!module->useCompatibleBPMCV);
        item2->module = module;
        menu->addChild(item2);

        menu->addChild(new MenuSeparator());

        SnapCVOptionItem* item3 = new SnapCVOptionItem;
        item3->text = "Snap CV";
        item3->rightText = CHECKMARK(module->snapCV);
        item3->module = module;
        menu->addChild(item3);

        return menu;
    }
};
