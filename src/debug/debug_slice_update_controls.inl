void UpdateDebugSliceControls(HWND hwnd) {
    if (!gApp || !gEffectDebugViewer) return;
    wchar_t title[160]{};
    if (gDebugSliceEffect == DebugSliceEffect::Props) {
        swprintf_s(title, L"Backrooms Maze Effect Slice Debug - Props - %s (%d/%d)",
            DebugPropName(gDebugPropIndex), WrapDebugPropIndex(gDebugPropIndex) + 1, kDebugPropCount);
    } else {
        swprintf_s(title, L"Backrooms Maze Effect Slice Debug - %s - %dx%d",
            DebugSliceEffectName(gDebugSliceEffect), gDebugSliceTiles, gDebugSliceTiles);
    }
    SetWindowTextW(hwnd, title);
    if (gApp->debugSize) {
        wchar_t sizeText[48]{};
        swprintf_s(sizeText, L"Grid: %dx%d", gDebugSliceTiles, gDebugSliceTiles);
        SetWindowTextW(gApp->debugSize, sizeText);
    }
    if (gApp->debugPrevProp) EnableWindow(gApp->debugPrevProp, TRUE);
    if (gApp->debugNextProp) EnableWindow(gApp->debugNextProp, TRUE);
}
