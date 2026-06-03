void InitializeGameHostUi(App& app, HWND hwnd) {
    LayoutGameControls(hwnd);
    SetGameMenuVisible(true);
    SetCustomGameControlsVisible(false);
    UpdateGameMenuLabels();
    SetGameCursorVisible(true);
    app.gameMenuFadeStart = GetTickCount64();
    SetDebugControlsVisible(false);
    ShowWindow(hwnd, SW_SHOW);
    UpdateWindow(hwnd);
    EnsureGameRenderer(hwnd, RendererRuntimeMode::MainMenu);
}
