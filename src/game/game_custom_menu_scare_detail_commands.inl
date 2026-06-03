    if (gApp->gameCustomSelectedScare >= 0) {
        switch (static_cast<CustomGameMenuControl>(control)) {
        case CustomGameMenuControl::ScareChanceMinus: AdjustSelectedCustomScareChance(-5); break;
        case CustomGameMenuControl::ScareChancePlus: AdjustSelectedCustomScareChance(5); break;
        case CustomGameMenuControl::ScareStartMinMinus: AdjustSelectedCustomScareStartWindow(true, -5); break;
        case CustomGameMenuControl::ScareStartMinPlus: AdjustSelectedCustomScareStartWindow(true, 5); break;
        case CustomGameMenuControl::ScareStartMaxMinus: AdjustSelectedCustomScareStartWindow(false, -5); break;
        case CustomGameMenuControl::ScareStartMaxPlus: AdjustSelectedCustomScareStartWindow(false, 5); break;
        case CustomGameMenuControl::ScareDetailBack: gApp->gameCustomSelectedScare = -1; break;
        default: break;
        }
        if (gApp->rendererInitialized) {
            gApp->renderer.SetCustomGameMenuState(gApp->gameCustomSpec, control, gApp->gameCustomSelectedScare);
        }
        InvalidateRect(hwnd, nullptr, FALSE);
        return;
    }
