bool HandleDebugSliceCommand(HWND hwnd, WPARAM wParam) {
    if (!gApp || !gEffectDebugViewer || hwnd != gApp->hwnd) return false;
    int id = LOWORD(wParam);
    if (id != kDebugPrevEffectId && id != kDebugNextEffectId && id != kDebugSizeId && id != kDebugResetId &&
        id != kDebugPrevPropId && id != kDebugNextPropId) {
        return false;
    }

    if (id == kDebugPrevEffectId) {
        gDebugSliceEffect = StepDebugSliceEffect(gDebugSliceEffect, -1);
    } else if (id == kDebugNextEffectId) {
        gDebugSliceEffect = StepDebugSliceEffect(gDebugSliceEffect, 1);
    } else if (id == kDebugSizeId) {
        gDebugSliceTiles = gDebugSliceTiles >= 5 ? 1 : gDebugSliceTiles + 1;
    } else if (id == kDebugPrevPropId || id == kDebugNextPropId) {
        if (gDebugSliceEffect != DebugSliceEffect::Props) {
            gDebugSliceEffect = DebugSliceEffect::Props;
        }
        gDebugPropIndex = WrapDebugPropIndex(gDebugPropIndex + (id == kDebugPrevPropId ? -1 : 1));
    } else if (id == kDebugResetId) {
        gApp->renderer.ResetDebugSliceAnimation();
        UpdateDebugSliceControls(hwnd);
        RedrawDebugSliceControls();
        return true;
    }
    gApp->renderer.ConfigureDebugSlice(gDebugSliceEffect, gDebugSliceTiles);
    UpdateDebugSliceControls(hwnd);
    RedrawDebugSliceControls();
    return true;
}
