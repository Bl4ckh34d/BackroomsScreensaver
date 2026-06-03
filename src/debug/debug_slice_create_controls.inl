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
