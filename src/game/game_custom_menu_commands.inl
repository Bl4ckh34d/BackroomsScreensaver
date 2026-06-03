// Custom-game menu commands.

void ActivateCustomGameMenuCommand(HWND hwnd, int control) {
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
    switch (static_cast<CustomGameMenuControl>(control)) {
    case CustomGameMenuControl::BrokenLampScareDetails:
    case CustomGameMenuControl::AirVentScareDetails:
    case CustomGameMenuControl::WaterScareDetails:
    case CustomGameMenuControl::BloodWorldScareDetails:
    case CustomGameMenuControl::FleshWorldScareDetails:
        gApp->gameCustomSelectedScare = CustomGameScareIndexFromControl(static_cast<CustomGameMenuControl>(control));
        break;
    case CustomGameMenuControl::EnvironmentDetails:
        gApp->gameCustomSelectedScare = -2;
        break;
    case CustomGameMenuControl::BrokenLampScares: gApp->gameCustomSpec.brokenLampScares = !gApp->gameCustomSpec.brokenLampScares; break;
    case CustomGameMenuControl::AirVentScares: gApp->gameCustomSpec.airVentScares = !gApp->gameCustomSpec.airVentScares; break;
    case CustomGameMenuControl::WaterScares: gApp->gameCustomSpec.waterScares = !gApp->gameCustomSpec.waterScares; break;
    case CustomGameMenuControl::BloodWorldScares: gApp->gameCustomSpec.bloodWorldScares = !gApp->gameCustomSpec.bloodWorldScares; break;
    case CustomGameMenuControl::FleshWorldScares: gApp->gameCustomSpec.fleshWorldScares = !gApp->gameCustomSpec.fleshWorldScares; break;
    case CustomGameMenuControl::OmukadeBoss: gApp->gameCustomSpec.omukadeBoss = !gApp->gameCustomSpec.omukadeBoss; break;
    case CustomGameMenuControl::SizeXMinus: AdjustCustomGameSize(gApp->gameCustomSpec.mazeWidth, -2); break;
    case CustomGameMenuControl::SizeXPlus: AdjustCustomGameSize(gApp->gameCustomSpec.mazeWidth, 2); break;
    case CustomGameMenuControl::SizeYMinus: AdjustCustomGameSize(gApp->gameCustomSpec.mazeHeight, -2); break;
    case CustomGameMenuControl::SizeYPlus: AdjustCustomGameSize(gApp->gameCustomSpec.mazeHeight, 2); break;
    case CustomGameMenuControl::RoomCountMinus: AdjustCustomGameRoomCount(gApp->gameCustomSpec.roomCount, -1); break;
    case CustomGameMenuControl::RoomCountPlus: AdjustCustomGameRoomCount(gApp->gameCustomSpec.roomCount, 1); break;
    case CustomGameMenuControl::ScareChanceMinus: AdjustCustomGameScareChance(gApp->gameCustomSpec.jumpscareChancePercent, -5); break;
    case CustomGameMenuControl::ScareChancePlus: AdjustCustomGameScareChance(gApp->gameCustomSpec.jumpscareChancePercent, 5); break;
    case CustomGameMenuControl::ScareStartMinMinus: AdjustCustomGameScareStartWindow(gApp->gameCustomSpec, true, -5); break;
    case CustomGameMenuControl::ScareStartMinPlus: AdjustCustomGameScareStartWindow(gApp->gameCustomSpec, true, 5); break;
    case CustomGameMenuControl::ScareStartMaxMinus: AdjustCustomGameScareStartWindow(gApp->gameCustomSpec, false, -5); break;
    case CustomGameMenuControl::ScareStartMaxPlus: AdjustCustomGameScareStartWindow(gApp->gameCustomSpec, false, 5); break;
    case CustomGameMenuControl::EightPages: gApp->gameCustomSpec.eightPages = !gApp->gameCustomSpec.eightPages; break;
    case CustomGameMenuControl::Start: StartCustomGameFromMenu(hwnd); return;
    case CustomGameMenuControl::Back: ExitGameCustomMenu(hwnd); return;
    default: break;
    }
    if (gApp->rendererInitialized) {
        gApp->renderer.SetCustomGameMenuState(gApp->gameCustomSpec, control, gApp->gameCustomSelectedScare);
    }
    InvalidateRect(hwnd, nullptr, FALSE);
}
