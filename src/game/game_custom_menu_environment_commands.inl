
    if (!gApp || !gApp->gameCustomMenuOpen) return;
    if (gApp->gameCustomSelectedScare == -2) {
        switch (static_cast<CustomGameMenuControl>(control)) {
        case CustomGameMenuControl::EnvDirtMinus: AdjustCustomGamePercent(gApp->gameCustomSpec.mapDirtPercent, -5); break;
        case CustomGameMenuControl::EnvDirtPlus: AdjustCustomGamePercent(gApp->gameCustomSpec.mapDirtPercent, 5); break;
        case CustomGameMenuControl::EnvPaperMinus: AdjustCustomGamePercent(gApp->gameCustomSpec.paperDensityPercent, -10, 400); break;
        case CustomGameMenuControl::EnvPaperPlus: AdjustCustomGamePercent(gApp->gameCustomSpec.paperDensityPercent, 10, 400); break;
        case CustomGameMenuControl::EnvPropMinus: AdjustCustomGamePercent(gApp->gameCustomSpec.propDensityPercent, -10, 400); break;
        case CustomGameMenuControl::EnvPropPlus: AdjustCustomGamePercent(gApp->gameCustomSpec.propDensityPercent, 10, 400); break;
        case CustomGameMenuControl::EnvLampOnMinus: AdjustCustomGamePercent(gApp->gameCustomSpec.lampOnPercent, -5); break;
        case CustomGameMenuControl::EnvLampOnPlus: AdjustCustomGamePercent(gApp->gameCustomSpec.lampOnPercent, 5); break;
        case CustomGameMenuControl::EnvLampFlickerMinus: AdjustCustomGamePercent(gApp->gameCustomSpec.lampFlickerPercent, -5); break;
        case CustomGameMenuControl::EnvLampFlickerPlus: AdjustCustomGamePercent(gApp->gameCustomSpec.lampFlickerPercent, 5); break;
        case CustomGameMenuControl::EnvLampSparkMinus: AdjustCustomGamePercent(gApp->gameCustomSpec.lampSparkPercent, -5); break;
        case CustomGameMenuControl::EnvLampSparkPlus: AdjustCustomGamePercent(gApp->gameCustomSpec.lampSparkPercent, 5); break;
        case CustomGameMenuControl::EnvFogStartMinus: AdjustCustomGameMeters(gApp->gameCustomSpec.fogStartMeters, -1, 0, 200); break;
        case CustomGameMenuControl::EnvFogStartPlus: AdjustCustomGameMeters(gApp->gameCustomSpec.fogStartMeters, 1, 0, 200); break;
        case CustomGameMenuControl::EnvFogEndMinus: AdjustCustomGameMeters(gApp->gameCustomSpec.fogEndMeters, -1, 1, 300); break;
        case CustomGameMenuControl::EnvFogEndPlus: AdjustCustomGameMeters(gApp->gameCustomSpec.fogEndMeters, 1, 1, 300); break;
        case CustomGameMenuControl::EnvFogDarkMinus: AdjustCustomGamePercent(gApp->gameCustomSpec.fogDarknessPercent, -5); break;
        case CustomGameMenuControl::EnvFogDarkPlus: AdjustCustomGamePercent(gApp->gameCustomSpec.fogDarknessPercent, 5); break;
        case CustomGameMenuControl::ScareDetailBack: gApp->gameCustomSelectedScare = -1; break;
        default: break;
        }
        gApp->gameCustomSpec.fogEndMeters = std::max(gApp->gameCustomSpec.fogEndMeters, gApp->gameCustomSpec.fogStartMeters + 1);
        if (gApp->rendererInitialized) {
            gApp->renderer.SetCustomGameMenuState(gApp->gameCustomSpec, control, gApp->gameCustomSelectedScare);
        }
        InvalidateRect(hwnd, nullptr, FALSE);
        return;
    }
