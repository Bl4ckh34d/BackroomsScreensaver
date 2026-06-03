// Effect-slice debug toolbar state and Win32 control updates.
// Included from app_runtime.inl after App and gApp are declared.

DebugSliceEffect StepDebugSliceEffect(DebugSliceEffect effect, int delta) {
    int count = static_cast<int>(DebugSliceEffect::Count);
    int index = (static_cast<int>(effect) + delta) % count;
    if (index < 0) index += count;
    return static_cast<DebugSliceEffect>(index);
}

void CreateDebugSliceControls(App& app, HWND hwnd, HINSTANCE hInstance, bool includeSettingsButton, DWORD extraStyle = 0) {
    DWORD style = WS_CHILD | extraStyle | BS_PUSHBUTTON;
    app.debugPrevEffect = CreateWindowW(L"BUTTON", L"< Effect", style,
        12, 10, 92, 28, hwnd, reinterpret_cast<HMENU>(static_cast<INT_PTR>(kDebugPrevEffectId)), hInstance, nullptr);
    app.debugNextEffect = CreateWindowW(L"BUTTON", L"Effect >", style,
        110, 10, 92, 28, hwnd, reinterpret_cast<HMENU>(static_cast<INT_PTR>(kDebugNextEffectId)), hInstance, nullptr);
    app.debugSize = CreateWindowW(L"BUTTON", L"Grid: 3x3", style,
        210, 10, 104, 28, hwnd, reinterpret_cast<HMENU>(static_cast<INT_PTR>(kDebugSizeId)), hInstance, nullptr);
    app.debugReset = CreateWindowW(L"BUTTON", L"Reset anim", style,
        322, 10, 104, 28, hwnd, reinterpret_cast<HMENU>(static_cast<INT_PTR>(kDebugResetId)), hInstance, nullptr);
    app.debugPrevProp = CreateWindowW(L"BUTTON", L"< Prop", style,
        434, 10, 84, 28, hwnd, reinterpret_cast<HMENU>(static_cast<INT_PTR>(kDebugPrevPropId)), hInstance, nullptr);
    app.debugNextProp = CreateWindowW(L"BUTTON", L"Prop >", style,
        526, 10, 84, 28, hwnd, reinterpret_cast<HMENU>(static_cast<INT_PTR>(kDebugNextPropId)), hInstance, nullptr);
    if (includeSettingsButton) {
        app.debugSettings = CreateWindowW(L"BUTTON", L"Debug settings", style,
            618, 10, 126, 28, hwnd, reinterpret_cast<HMENU>(static_cast<INT_PTR>(kDebugSettingsId)), hInstance, nullptr);
    }
}

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

void RedrawDebugSliceControls() {
    if (!gApp || !gEffectDebugViewer) return;
    HWND controls[] = {
        gApp->debugPrevEffect,
        gApp->debugNextEffect,
        gApp->debugSize,
        gApp->debugReset,
        gApp->debugPrevProp,
        gApp->debugNextProp,
        gApp->debugSettings
    };
    for (HWND control : controls) {
        if (!control) continue;
        SetWindowPos(control, HWND_TOP, 0, 0, 0, 0,
            SWP_NOMOVE | SWP_NOSIZE | SWP_NOACTIVATE);
        RedrawWindow(control, nullptr, nullptr,
            RDW_INVALIDATE | RDW_UPDATENOW | RDW_ERASE | RDW_ALLCHILDREN);
    }
}

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
