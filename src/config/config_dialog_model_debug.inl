void BuildDebugConfigModel(ConfigState* state) {
    state->tabLabels = {L"Debug View", L"Effects", L"Autopilot", L"Monster"};
    state->tabNotes = {
        L"Debug-only overlays and forced study views.",
        L"Effect loop and stress-test tuning used by the debug scene.",
        L"Screensaver/autopilot camera behavior, useful for reusing movement logic in monster AI.",
        L"Monster mesh and orientation. Right-drag the preview to orbit and use the wheel to zoom."
    };

    auto& f = state->fieldDefs;
    AddConfigFieldCopy(f, L"Maze", L"MapOverlay", 0, 0, L"Overlays");
    AddConfigFieldCopy(f, L"Dread", L"DebugMeter", 0, 0, L"Overlays");
    AddCustomConfigField(f, 0, 0, kConfigFieldBaseId + 176, L"Overlays", L"Debug", L"AiMapOverlay", L"AI minimap overlay", L"0", ConfigFieldKind::Bool, 0);
    AddCustomConfigField(f, 0, 0, kConfigFieldBaseId + 177, L"Cheats", L"Debug", L"InfiniteStamina", L"Infinite stamina", L"0", ConfigFieldKind::Bool, 0);
    AddCustomConfigField(f, 0, 0, kConfigFieldBaseId + 178, L"Cheats", L"Debug", L"Invincible", L"Invincible", L"0", ConfigFieldKind::Bool, 0);
    AddConfigFieldCopy(f, L"Atmosphere", L"BloodStudyView", 0, 1, L"Forced Views");

    const wchar_t* effectKeys[] = {L"BloodLoopSeconds", L"BloodFullSpreadAge", L"WaterLoopSeconds", L"AirVentLoopSeconds",
        L"BrokenLampLoopSeconds", L"StaticLoopSeconds", L"BrokenLampSparkIntensityMin", L"BrokenLampSparkIntensityMax",
        L"BrokenLampChainIntensityScale", L"BrokenLampChainBurstsMin", L"BrokenLampChainBurstsMax",
        L"AirVentSteamIntensityMin", L"AirVentSteamIntensityMax", L"AirVentPanelDropEvery", L"AirVentPanelDropChance"};
    for (const wchar_t* key : effectKeys) AddConfigFieldCopy(f, L"EffectTuning", key, 1,
        std::wcsstr(key, L"AirVent") ? 1 : 0, L"Effect Tuning");

    const wchar_t* autopilotKeys[] = {L"RoomSpeed", L"TurnLookAheadTiles", L"RoomLookAheadTiles", L"RoomPauseChance",
        L"JunctionScanChance", L"ScanAngleDegrees", L"LookBackMinSeconds", L"LookBackMaxSeconds",
        L"JunctionScanBaseSeconds", L"JunctionScanBranchSeconds"};
    for (const wchar_t* key : autopilotKeys) AddConfigFieldCopy(f, L"CameraAI", key, 2,
        std::wcsstr(key, L"Look") || std::wcsstr(key, L"Scan") || std::wcsstr(key, L"Junction") ? 1 : 0, L"Autopilot");

    const wchar_t* monsterKeys[] = {L"SkullMesh", L"AlternateSkullMesh", L"AlternateSkullChance", L"SkullMaxTriangles",
        L"SkullYawDegrees", L"SkullPitchDegrees", L"SkullRollDegrees",
        L"AlternateSkullYawDegrees", L"AlternateSkullPitchDegrees", L"AlternateSkullRollDegrees"};
    for (const wchar_t* key : monsterKeys) {
        AddConfigFieldCopy(f, L"Monster", key, 3, 0, L"Meshes / Orientation");
    }
}
