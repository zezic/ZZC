#include "PhasequeWidget.hpp"
#include "controls.hpp"

PhasequeWidget::PhasequeWidget(Phaseque* module) {
    setModule(module);
    setPanel(APP->window->loadSvg(asset::plugin(pluginInstance, "res/panels/Phaseque.svg")));

    addParam(createParam<ZZC_VBPSVOCTSwitch>(Vec(49.f, 37.f), module, Phaseque::USE_COMPATIBLE_BPM_CV_PARAM));

    addInput(createInput<ZZC_PJ_Port>(Vec(14, 50), module, Phaseque::CLOCK_INPUT));
    addChild(createLight<TinyLight<GreenLight>>(Vec(36, 50), module, Phaseque::CLOCK_LED));
    addParam(createParam<ZZC_LEDBezelDark>(Vec(15.3f, 82.3f), module, Phaseque::TEMPO_TRACK_SWITCH_PARAM));
    addChild(createLight<LedLight<ZZC_YellowLight>>(Vec(17.1f, 84.0f), module, Phaseque::TEMPO_TRACK_LED));
    addInput(createInput<ZZC_PJ_Port>(Vec(49, 50), module, Phaseque::VBPS_INPUT));
    addChild(createLight<TinyLight<GreenLight>>(Vec(71, 50), module, Phaseque::VBPS_LED));
    addParam(createParam<ZZC_Knob25SnappyNoRand>(Vec(49, 80.8f), module, Phaseque::BPM_PARAM));
    addInput(createInput<ZZC_PJ_Port>(Vec(84, 50), module, Phaseque::PHASE_INPUT));
    addChild(createLight<TinyLight<GreenLight>>(Vec(106, 50), module, Phaseque::PHASE_LED));
    addParam(createParam<ZZC_EncoderKnob>(Vec(258.75f, 49), module, Phaseque::PHASE_PARAM));
    addParam(createParam<ZZC_LEDBezelDark>(Vec(120.3f, 51.3f), module, Phaseque::ABS_MODE_SWITCH_PARAM));
    addChild(createLight<LedLight<ZZC_YellowLight>>(Vec(122.1f, 53.0f), module, Phaseque::ABS_MODE_LED));

    Display32Widget* bpmDisplay = new Display32Widget();
    bpmDisplay->box.pos = Vec(84.0f, 83.0f);
    bpmDisplay->box.size = Vec(58.0f, 21.0f);
    if (module) {
        bpmDisplay->value = &module->bpm;
        bpmDisplay->disabled = &module->bpmDisabled;
    }
    addChild(bpmDisplay);

    addInput(createInput<ZZC_PJ_Port>(Vec(189, 81), module, Phaseque::CLUTCH_INPUT));
    addParam(createParam<ZZC_LEDBezelDark>(Vec(190.3f, 51.3f), module, Phaseque::CLUTCH_SWITCH_PARAM));
    addChild(createLight<LedLight<ZZC_YellowLight>>(Vec(192.1f, 53.0f), module, Phaseque::CLUTCH_LED));

    DisplayIntpartWidget* resolutionDisplay = new DisplayIntpartWidget();
    resolutionDisplay->box.pos = Vec(152, 52);
    resolutionDisplay->box.size = Vec(29, 21);
    if (module) {
        resolutionDisplay->value = &module->resolutionDisplay;
    }
    addChild(resolutionDisplay);

    addParam(createParam<ZZC_PhasequePatternResoKnob>(Vec(154, 80.8f), module, Phaseque::PATTERN_RESO_PARAM));

    addInput(createInput<ZZC_PJ_Port>(Vec(224, 81), module, Phaseque::RESET_INPUT));
    addParam(createParam<ZZC_LEDBezelDark>(Vec(225.3f, 51.3f), module, Phaseque::RESET_SWITCH_PARAM));
    addChild(createLight<LedLight<ZZC_YellowLight>>(Vec(227.1f, 53.0f), module, Phaseque::RESET_LED));

    addInput(createInput<ZZC_PJ_Port>(Vec(14, 126), module, Phaseque::GOTO_INPUT));
    addInput(createInput<ZZC_PJ_Port>(Vec(49, 126), module, Phaseque::PTRN_INPUT));
    addOutput(createOutput<ZZC_PJ_Port>(Vec(14, 174), module, Phaseque::WENT_OUTPUT));
    addOutput(createOutput<ZZC_PJ_Port>(Vec(49, 174), module, Phaseque::PTRN_OUTPUT));

    GridDisplayWidget* patternsDisplayDisplay = new GridDisplayWidget();
    patternsDisplayDisplay->box.pos = Vec(84.0f, 117.0f);
    patternsDisplayDisplay->setupSize(Vec(165.0f, 85.0f));
    if (module) {
        Phaseque* phaseque = dynamic_cast<Phaseque*>(module);
        assert(phaseque);
        phaseque->gridDisplayConsumer = patternsDisplayDisplay->consumer;
        phaseque->gridDisplayProducer = patternsDisplayDisplay->producer;
    }
    addChild(patternsDisplayDisplay);

    addOutput(createOutput<ZZC_PJ_Port>(Vec(259, 126), module, Phaseque::PTRN_PHASE_OUTPUT));
    addOutput(createOutput<ZZC_PJ_Port>(Vec(294, 126), module, Phaseque::PTRN_WRAP_OUTPUT));
    addOutput(createOutput<ZZC_PJ_Port>(Vec(259, 174), module, Phaseque::PTRN_START_OUTPUT));
    addOutput(createOutput<ZZC_PJ_Port>(Vec(294, 174), module, Phaseque::PTRN_END_OUTPUT));

    addInput(createInput<ZZC_PJ_Port>(Vec(14, 220), module, Phaseque::PREV_INPUT));
    addInput(createInput<ZZC_PJ_Port>(Vec(49, 220), module, Phaseque::NEXT_INPUT));

    addInput(createInput<ZZC_PJ_Port>(Vec(84, 220), module, Phaseque::LEFT_INPUT));
    addInput(createInput<ZZC_PJ_Port>(Vec(119, 220), module, Phaseque::DOWN_INPUT));
    addInput(createInput<ZZC_PJ_Port>(Vec(154, 220), module, Phaseque::UP_INPUT));
    addInput(createInput<ZZC_PJ_Port>(Vec(189, 220), module, Phaseque::RIGHT_INPUT));
    addInput(createInput<ZZC_PJ_Port>(Vec(224, 220), module, Phaseque::SEQ_INPUT));

    addInput(createInput<ZZC_PJ_Port>(Vec(259, 220), module, Phaseque::RND_INPUT));

    addParam(createParam<ZZC_LEDBezelDark>(Vec(295.3f, 221.3f), module, Phaseque::WAIT_SWITCH_PARAM));
    addChild(createLight<LedLight<ZZC_RedLight>>(Vec(297.1f, 223.0f), module, Phaseque::WAIT_LED));

    MainDisplayWidget* patternDisplay = new MainDisplayWidget();
    patternDisplay->box.pos = Vec(333.0f, 50.0f);
    patternDisplay->box.size = Vec(233.0f, 233.0f);
    patternDisplay->setupSizes();
    if (module) {
        Phaseque* phaseque = dynamic_cast<Phaseque*>(module);
        assert(phaseque);
        phaseque->mainDisplayConsumer = patternDisplay->consumer;
    }
    addChild(patternDisplay);

    addInput(createInput<ZZC_PJ_Port>(Vec(259, 319.75f), module, Phaseque::GLOBAL_GATE_INPUT));
    addParam(createParam<ZZC_LEDBezelDark>(Vec(260.3f, 278.3f), module, Phaseque::GLOBAL_GATE_SWITCH_PARAM));
    addChild(createLight<LedLight<ZZC_YellowLight>>(Vec(262.1f, 280.0f), module, Phaseque::GLOBAL_GATE_LED));

    addInput(createInput<ZZC_PJ_Port>(Vec(14, 319.75f), module, Phaseque::GLOBAL_SHIFT_INPUT));
    addParam(createParam<ZZC_Knob25NoRand>(Vec(14.05f, 276.9f), module, Phaseque::GLOBAL_SHIFT_PARAM));

    addInput(createInput<ZZC_PJ_Port>(Vec(49, 319.75f), module, Phaseque::GLOBAL_LEN_INPUT));
    addParam(createParam<ZZC_Knob25NoRand>(Vec(49.05f, 276.9f), module, Phaseque::GLOBAL_LEN_PARAM));

    addInput(createInput<ZZC_PJ_Port>(Vec(294, 277), module, Phaseque::GLOBAL_EXPR_CURVE_INPUT));
    addInput(createInput<ZZC_PJ_Port>(Vec(294, 319.75f), module, Phaseque::GLOBAL_EXPR_POWER_INPUT));

    addParam(createParam<ZZC_LEDBezelDark>(Vec(85.3f, 278.3f), module, Phaseque::QNT_SWITCH_PARAM));
    addChild(createLight<LedLight<ZZC_YellowLight>>(Vec(87.1f, 280.0f), module, Phaseque::QNT_LED));

    addParam(createParam<ZZC_LEDBezelDark>(Vec(120.3f, 278.3f), module, Phaseque::SHIFT_LEFT_SWITCH_PARAM));
    addChild(createLight<LedLight<ZZC_YellowLight>>(Vec(122.1f, 280.0f), module, Phaseque::SHIFT_LEFT_LED));

    addParam(createParam<ZZC_PhasequePatternShiftKnob>(Vec(150.5f, 273.5f), module, Phaseque::PATTERN_SHIFT_PARAM));

    addParam(createParam<ZZC_LEDBezelDark>(Vec(190.3f, 278.3f), module, Phaseque::SHIFT_RIGHT_SWITCH_PARAM));
    addChild(createLight<LedLight<ZZC_YellowLight>>(Vec(192.1f, 280.0f), module, Phaseque::SHIFT_RIGHT_LED));

    addParam(createParam<ZZC_LEDBezelDark>(Vec(225.3f, 278.3f), module, Phaseque::LEN_SWITCH_PARAM));
    addChild(createLight<LedLight<ZZC_YellowLight>>(Vec(227.1f, 280.0f), module, Phaseque::LEN_LED));

    addParam(createParam<ZZC_LEDBezelDark>(Vec(85.3f, 320.3f), module, Phaseque::REV_SWITCH_PARAM));
    addChild(createLight<LedLight<ZZC_YellowLight>>(Vec(87.1f, 322.0f), module, Phaseque::REV_LED));

    addInput(createInput<ZZC_PJ_Port>(Vec(119, 319.75f), module, Phaseque::MUTA_DEC_INPUT));

    addParam(createParam<ZZC_PhasequeMutaKnob>(Vec(153, 318), module, Phaseque::PATTERN_MUTA_PARAM));

    addInput(createInput<ZZC_PJ_Port>(Vec(189, 319.75f), module, Phaseque::MUTA_INC_INPUT));

    addParam(createParam<ZZC_LEDBezelDark>(Vec(225.3f, 320.3f), module, Phaseque::FLIP_SWITCH_PARAM));
    addChild(createLight<LedLight<ZZC_YellowLight>>(Vec(227.1f, 322.0f), module, Phaseque::FLIP_LED));

    addInput(createInput<ZZC_PJ_Port>(Vec(579, 319.75f), module, Phaseque::RND_JUMP_INPUT));

    float stepPeriod = 35.0f;
    float stepsAreaX = 614.0;
    for (int i = 0; i < NUM_STEPS; i++) {
        addParam(createParam<ZZC_PhasequeStepAttrKnob23>(Vec(stepsAreaX + stepPeriod * i - 0.5f, 48), module,
                                                         Phaseque::STEP_VALUE_PARAM + i));
        addParam(createParam<ZZC_LEDBezelDark>(Vec(stepsAreaX + 3.3f + stepPeriod * i, 82.3f), module,
                                               Phaseque::GATE_SWITCH_PARAM + i));
        addChild(createLight<LedLight<ZZC_YellowLight>>(Vec(stepsAreaX + 5.1f + stepPeriod * i, 84.0f), module,
                                                        Phaseque::GATE_SWITCH_LED + i));
        addParam(createParam<ZZC_PhasequeStepAttrKnob21>(Vec(stepsAreaX + 0.5f + stepPeriod * i, 110.5f), module,
                                                         Phaseque::STEP_SHIFT_PARAM + i));
        addParam(createParam<ZZC_PhasequeStepAttrKnob21Uni>(Vec(stepsAreaX + 0.5f + stepPeriod * i, 141.5f), module,
                                                            Phaseque::STEP_LEN_PARAM + i));
        addParam(createParam<ZZC_PhasequeStepAttrKnob19>(Vec(stepsAreaX + 1.5f + stepPeriod * i, 176.f), module,
                                                         Phaseque::STEP_EXPR_IN_PARAM + i));

        ZZC_PhasequeXYDisplayWidget* exprDisplay = new ZZC_PhasequeXYDisplayWidget();
        exprDisplay->box.pos = Vec(stepsAreaX + 1.0f + stepPeriod * i, 207);
        exprDisplay->box.size = Vec(27, 27);
        if (module) {
            exprDisplay->paramQuantityX = module->paramQuantities[Phaseque::STEP_EXPR_POWER_PARAM + i];
            exprDisplay->paramQuantityY = module->paramQuantities[Phaseque::STEP_EXPR_CURVE_PARAM + i];
        }
        exprDisplay->setupSize();
        addParam(exprDisplay);

        addParam(createParam<ZZC_PhasequeStepAttrKnob19>(Vec(stepsAreaX + 1.5f + stepPeriod * i, 240.5f), module,
                                                         Phaseque::STEP_EXPR_OUT_PARAM + i));

        addInput(createInput<ZZC_PJ_Port>(Vec(stepsAreaX + 2.0f + stepPeriod * i, 271), module,
                                          Phaseque::STEP_JUMP_INPUT + i));
        addChild(createLight<SmallLight<GreenLight>>(Vec(stepsAreaX + 11.3f + stepPeriod * i, 301.5f), module,
                                                     Phaseque::STEP_GATE_LIGHT + i));
        addOutput(createOutput<ZZC_PJ_Port>(Vec(stepsAreaX + 2.0f + stepPeriod * i, 319.75f), module,
                                            Phaseque::STEP_GATE_OUTPUT + i));
    }

    stepPeriod = 34.0f;
    float outputsAreaX = 335.0f;

    addOutput(createOutput<ZZC_PJ_Port>(Vec(outputsAreaX + stepPeriod * 0, 319.75f), module, Phaseque::GATE_OUTPUT));
    addOutput(createOutput<ZZC_PJ_Port>(Vec(outputsAreaX + stepPeriod * 1, 319.75f), module, Phaseque::V_OUTPUT));
    addOutput(createOutput<ZZC_PJ_Port>(Vec(outputsAreaX + stepPeriod * 2, 319.75f), module, Phaseque::SHIFT_OUTPUT));
    addOutput(createOutput<ZZC_PJ_Port>(Vec(outputsAreaX + stepPeriod * 3, 319.75f), module, Phaseque::LEN_OUTPUT));
    addOutput(createOutput<ZZC_PJ_Port>(Vec(outputsAreaX + stepPeriod * 4, 319.75f), module, Phaseque::EXPR_OUTPUT));
    addOutput(
        createOutput<ZZC_PJ_Port>(Vec(outputsAreaX + stepPeriod * 5, 319.75f), module, Phaseque::EXPR_CURVE_OUTPUT));
    addOutput(createOutput<ZZC_PJ_Port>(Vec(outputsAreaX + stepPeriod * 6, 319.75f), module, Phaseque::PHASE_OUTPUT));

    float outputLedsAreaX = 344.3f;

    addChild(createLight<SmallLight<GreenLight>>(Vec(outputLedsAreaX + stepPeriod * 0, 301.5f), module,
                                                 Phaseque::GATE_LIGHT));
    addChild(createLight<SmallLight<GreenRedLight>>(Vec(outputLedsAreaX + stepPeriod * 1, 301.5f), module,
                                                    Phaseque::V_POS_LIGHT));
    addChild(createLight<SmallLight<GreenRedLight>>(Vec(outputLedsAreaX + stepPeriod * 2, 301.5f), module,
                                                    Phaseque::SHIFT_POS_LIGHT));
    addChild(createLight<SmallLight<GreenRedLight>>(Vec(outputLedsAreaX + stepPeriod * 3, 301.5f), module,
                                                    Phaseque::LEN_POS_LIGHT));
    addChild(createLight<SmallLight<GreenRedLight>>(Vec(outputLedsAreaX + stepPeriod * 4, 301.5f), module,
                                                    Phaseque::EXPR_POS_LIGHT));
    addChild(createLight<SmallLight<GreenRedLight>>(Vec(outputLedsAreaX + stepPeriod * 5, 301.5f), module,
                                                    Phaseque::EXPR_CURVE_POS_LIGHT));
    addChild(createLight<SmallLight<GreenLight>>(Vec(outputLedsAreaX + stepPeriod * 6, 301.5f), module,
                                                 Phaseque::PHASE_LIGHT));

    addChild(createWidget<ZZC_Screw>(Vec(RACK_GRID_WIDTH, 0)));
    addChild(createWidget<ZZC_Screw>(Vec(box.size.x - 2 * RACK_GRID_WIDTH, 0)));
    addChild(createWidget<ZZC_Screw>(Vec(RACK_GRID_WIDTH, RACK_GRID_HEIGHT - RACK_GRID_WIDTH)));
    addChild(createWidget<ZZC_Screw>(Vec(box.size.x - 2 * RACK_GRID_WIDTH, RACK_GRID_HEIGHT - RACK_GRID_WIDTH)));
}

void PhasequeWidget::appendContextMenu(Menu* menu) {
    menu->addChild(new MenuSeparator());

    Phaseque* phaseq = dynamic_cast<Phaseque*>(module);
    assert(phaseq);

    PhasequeCopyToNextItem* phaseqCopyToNextItem = createMenuItem<PhasequeCopyToNextItem>("Copy Steps To Next Pattern");
    PhasequeCopyToPrevItem* phaseqCopyToPrevItem = createMenuItem<PhasequeCopyToPrevItem>("Copy Steps To Prev Pattern");
    PhasequeCopyResoItem* phaseqCopyResoItem = createMenuItem<PhasequeCopyResoItem>("Copy Reso To All Patterns");
    PhasequeRndAllItem* phaseqRndAllItem = createMenuItem<PhasequeRndAllItem>("Randomize All Patterns");
    PhasequeRndAllResoItem* phaseqRndAllResoItem = createMenuItem<PhasequeRndAllResoItem>("Randomize All Resolutions");
    PhasequeBakeMutationItem* phaseqBakeMutationItem = createMenuItem<PhasequeBakeMutationItem>("Bake Mutation");
    PhasequeClearPatternItem* phaseqClearPatternItem = createMenuItem<PhasequeClearPatternItem>("Clear Pattern");
    phaseqCopyToNextItem->phaseq = phaseq;
    phaseqCopyToPrevItem->phaseq = phaseq;
    phaseqCopyResoItem->phaseq = phaseq;
    phaseqRndAllItem->phaseq = phaseq;
    phaseqRndAllResoItem->phaseq = phaseq;
    phaseqBakeMutationItem->phaseq = phaseq;
    phaseqClearPatternItem->phaseq = phaseq;
    menu->addChild(phaseqCopyToNextItem);
    menu->addChild(phaseqCopyToPrevItem);
    menu->addChild(new MenuSeparator());
    menu->addChild(createMenuItem("Copy To All Patterns", "", [=] { phaseq->copyToAll(); }));
    menu->addChild(phaseqCopyResoItem);
    menu->addChild(new MenuSeparator());
    menu->addChild(phaseqRndAllItem);
    menu->addChild(phaseqRndAllResoItem);
    menu->addChild(new MenuSeparator());
    menu->addChild(phaseqBakeMutationItem);
    menu->addChild(new MenuSeparator());
    menu->addChild(phaseqClearPatternItem);
    menu->addChild(new MenuSeparator());

    menu->addChild(createBoolPtrMenuItem("Retrigger Gate between active steps", "", &phaseq->retrigGapGate));
    menu->addChild(new MenuSeparator());

    PolyModeItem* polyModeItem = new PolyModeItem;
    polyModeItem->text = "Polyphony";
    polyModeItem->rightText = RIGHT_ARROW;
    polyModeItem->module = phaseq;
    menu->addChild(polyModeItem);

    menu->addChild(new MenuSeparator());

    ExternalCVModeItem* externalCVModeItem = new ExternalCVModeItem;
    externalCVModeItem->text = "External CV Mode";
    externalCVModeItem->rightText = RIGHT_ARROW;
    externalCVModeItem->module = phaseq;
    menu->addChild(externalCVModeItem);
}
