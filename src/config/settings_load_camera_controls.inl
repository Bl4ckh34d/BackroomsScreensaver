    s.walkSpeed = std::clamp(IniFloat(L"CameraAI", L"WalkSpeed", s.walkSpeed), 0.1f, 8.0f);
    s.roomSpeed = std::clamp(IniFloat(L"CameraAI", L"RoomSpeed", s.roomSpeed), 0.1f, 8.0f);
    s.runSpeed = std::clamp(IniFloat(L"CameraAI", L"RunSpeed", s.runSpeed), 0.1f, 12.0f);
    s.turnLookAheadTiles = std::clamp(IniFloat(L"CameraAI", L"TurnLookAheadTiles", s.turnLookAheadTiles), 0.0f, 8.0f);
    s.roomLookAheadTiles = std::clamp(IniFloat(L"CameraAI", L"RoomLookAheadTiles", s.roomLookAheadTiles), 0.0f, 10.0f);
    s.roomPauseChance = std::clamp(IniFloat(L"CameraAI", L"RoomPauseChance", s.roomPauseChance), 0.0f, 1.0f);
    s.junctionScanChance = std::clamp(IniFloat(L"CameraAI", L"JunctionScanChance", s.junctionScanChance), 0.0f, 1.0f);
    s.scanAngleDegrees = std::clamp(IniFloat(L"CameraAI", L"ScanAngleDegrees", s.scanAngleDegrees), 0.0f, 160.0f);
    s.lookBackMinSeconds = std::clamp(IniFloat(L"CameraAI", L"LookBackMinSeconds", s.lookBackMinSeconds), 2.0f, 300.0f);
    s.lookBackMaxSeconds = std::max(s.lookBackMinSeconds, std::clamp(IniFloat(L"CameraAI", L"LookBackMaxSeconds", s.lookBackMaxSeconds), 2.0f, 300.0f));
    s.headBobAmount = std::clamp(IniFloat(L"CameraAI", L"HeadBobAmount", s.headBobAmount), 0.0f, 0.4f);
    s.sideSwayAmount = std::clamp(IniFloat(L"CameraAI", L"SideSwayAmount", s.sideSwayAmount), 0.0f, 0.3f);
    s.junctionScanBaseSeconds = std::clamp(IniFloat(L"CameraAI", L"JunctionScanBaseSeconds", s.junctionScanBaseSeconds), 0.0f, 4.0f);
    s.junctionScanBranchSeconds = std::clamp(IniFloat(L"CameraAI", L"JunctionScanBranchSeconds", s.junctionScanBranchSeconds), 0.0f, 3.0f);

    s.flashlightSwayAmount = std::clamp(IniFloat(L"CameraFX", L"FlashlightSwayAmount", s.flashlightSwayAmount), 0.0f, 4.0f);
    s.flashlightFollowSpeed = std::clamp(IniFloat(L"CameraFX", L"FlashlightFollowSpeed", s.flashlightFollowSpeed), 0.1f, 4.0f);
    s.flashlightPanicDartAmount = std::clamp(IniFloat(L"CameraFX", L"FlashlightPanicDartAmount", s.flashlightPanicDartAmount), 0.0f, 4.0f);
    s.exitDoorOpenSeconds = std::clamp(IniFloat(L"CameraFX", L"ExitDoorOpenSeconds", s.exitDoorOpenSeconds), 0.2f, 8.0f);
    s.exitStepSeconds = std::clamp(IniFloat(L"CameraFX", L"ExitStepSeconds", s.exitStepSeconds), 0.2f, 8.0f);
    s.exitFadeSeconds = std::clamp(IniFloat(L"CameraFX", L"ExitFadeSeconds", s.exitFadeSeconds), 0.2f, 8.0f);
    s.exitStepDistance = std::clamp(IniFloat(L"CameraFX", L"ExitStepDistance", s.exitStepDistance), 0.0f, 8.0f);
    s.fadeInSeconds = std::clamp(IniFloat(L"CameraFX", L"FadeInSeconds", s.fadeInSeconds), 0.0f, 8.0f);
    s.mouseSensitivity = std::clamp(IniFloat(L"Controls", L"MouseSensitivity", s.mouseSensitivity), 0.1f, 5.0f);
    s.invertMouseY = IniInt(L"Controls", L"InvertMouseY", s.invertMouseY ? 1 : 0) != 0;
    for (const GameInputBindingDef& binding : kGameInputBindings) {
        int fallback = GameActionKey(s, binding.action);
        SetGameActionKey(s, binding.action, std::clamp(IniInt(L"Controls", binding.iniKey, fallback), 1, 255));
    }
