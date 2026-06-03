void ApplyScreensaverDebugGlobals(const ScreensaverRunConfig& config) {
    gEffectDebugViewer = config.bloodDebugMode;
    if (config.bloodDebugMode) {
        gDebugSliceEffect = gStartupDebugSliceEffect;
        gDebugSliceTiles = std::clamp(gDebugSliceTiles, 1, 5);
    }
    gBloodDebugEveryWall = gEffectDebugViewer &&
        (gDebugSliceEffect == DebugSliceEffect::Blood || DebugSliceEffectIsWater(gDebugSliceEffect));
}
