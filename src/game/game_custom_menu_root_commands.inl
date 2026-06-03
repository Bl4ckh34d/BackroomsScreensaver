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
