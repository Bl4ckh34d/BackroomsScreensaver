// BackroomsMazeGame.exe Win32 control creation and initial host UI state.
// Included from game_app.inl after game shell helpers are available.

void CreateGameHostControls(App& app, HWND hwnd, HINSTANCE hInstance) {
    app.gameTitle = CreateWindowW(L"STATIC", L"Backrooms Maze", WS_CHILD | WS_VISIBLE | SS_CENTER,
        0, 0, 10, 10, hwnd, nullptr, hInstance, nullptr);
    app.gameSinglePlayer = CreateWindowW(L"BUTTON", L"Single Player", WS_CHILD | WS_VISIBLE | BS_PUSHBUTTON,
        0, 0, 10, 10, hwnd, reinterpret_cast<HMENU>(static_cast<INT_PTR>(kGameSinglePlayerId)), hInstance, nullptr);
    app.gameSettings = CreateWindowW(L"BUTTON", L"Settings", WS_CHILD | WS_VISIBLE | BS_PUSHBUTTON,
        0, 0, 10, 10, hwnd, reinterpret_cast<HMENU>(static_cast<INT_PTR>(kGameSettingsId)), hInstance, nullptr);
    app.gameDebug = CreateWindowW(L"BUTTON", L"Debug", WS_CHILD | WS_VISIBLE | BS_PUSHBUTTON,
        0, 0, 10, 10, hwnd, reinterpret_cast<HMENU>(static_cast<INT_PTR>(kGameDebugId)), hInstance, nullptr);
    app.gameExit = CreateWindowW(L"BUTTON", L"Exit", WS_CHILD | WS_VISIBLE | BS_PUSHBUTTON,
        0, 0, 10, 10, hwnd, reinterpret_cast<HMENU>(static_cast<INT_PTR>(kGameExitId)), hInstance, nullptr);
    app.gameBack = CreateWindowW(L"BUTTON", L"Back", WS_CHILD | BS_PUSHBUTTON,
        0, 0, 10, 10, hwnd, reinterpret_cast<HMENU>(static_cast<INT_PTR>(kGameBackId)), hInstance, nullptr);

    app.customTitle = CreateWindowW(L"STATIC", L"Custom Game", WS_CHILD | SS_CENTER,
        0, 0, 10, 10, hwnd, nullptr, hInstance, nullptr);
    app.customLayerLabel = CreateWindowW(L"STATIC", L"Layer", WS_CHILD,
        0, 0, 10, 10, hwnd, nullptr, hInstance, nullptr);
    app.customLayer = CreateWindowW(L"COMBOBOX", L"", WS_CHILD | CBS_DROPDOWNLIST | WS_VSCROLL,
        0, 0, 10, 10, hwnd, reinterpret_cast<HMENU>(static_cast<INT_PTR>(kGameCustomLayerId)), hInstance, nullptr);
    SendMessageW(app.customLayer, CB_ADDSTRING, 0, reinterpret_cast<LPARAM>(L"Layer 1"));
    SendMessageW(app.customLayer, CB_SETCURSEL, 0, 0);
    app.customScaresLabel = CreateWindowW(L"STATIC", L"Jump scares", WS_CHILD,
        0, 0, 10, 10, hwnd, nullptr, hInstance, nullptr);
    app.customScareBrokenLamp = CreateWindowW(L"BUTTON", L"Broken lamps", WS_CHILD | BS_AUTOCHECKBOX,
        0, 0, 10, 10, hwnd, reinterpret_cast<HMENU>(static_cast<INT_PTR>(kGameCustomScareBrokenLampId)), hInstance, nullptr);
    app.customScareAirVent = CreateWindowW(L"BUTTON", L"Air vents", WS_CHILD | BS_AUTOCHECKBOX,
        0, 0, 10, 10, hwnd, reinterpret_cast<HMENU>(static_cast<INT_PTR>(kGameCustomScareAirVentId)), hInstance, nullptr);
    app.customScareWater = CreateWindowW(L"BUTTON", L"Water damage", WS_CHILD | BS_AUTOCHECKBOX,
        0, 0, 10, 10, hwnd, reinterpret_cast<HMENU>(static_cast<INT_PTR>(kGameCustomScareWaterId)), hInstance, nullptr);
    app.customScareBlood = CreateWindowW(L"BUTTON", L"Blood world", WS_CHILD | BS_AUTOCHECKBOX,
        0, 0, 10, 10, hwnd, reinterpret_cast<HMENU>(static_cast<INT_PTR>(kGameCustomScareBloodId)), hInstance, nullptr);
    app.customScareFlesh = CreateWindowW(L"BUTTON", L"Flesh world", WS_CHILD | BS_AUTOCHECKBOX,
        0, 0, 10, 10, hwnd, reinterpret_cast<HMENU>(static_cast<INT_PTR>(kGameCustomScareFleshId)), hInstance, nullptr);
    app.customBossesLabel = CreateWindowW(L"STATIC", L"Bosses", WS_CHILD,
        0, 0, 10, 10, hwnd, nullptr, hInstance, nullptr);
    app.customBossOmukade = CreateWindowW(L"BUTTON", L"Omukade", WS_CHILD | BS_AUTOCHECKBOX,
        0, 0, 10, 10, hwnd, reinterpret_cast<HMENU>(static_cast<INT_PTR>(kGameCustomBossOmukadeId)), hInstance, nullptr);
    app.customSizeLabel = CreateWindowW(L"STATIC", L"Level size", WS_CHILD,
        0, 0, 10, 10, hwnd, nullptr, hInstance, nullptr);
    app.customSizeXLabel = CreateWindowW(L"STATIC", L"X", WS_CHILD,
        0, 0, 10, 10, hwnd, nullptr, hInstance, nullptr);
    app.customSizeX = CreateWindowW(L"EDIT", L"15", WS_CHILD | WS_BORDER | ES_NUMBER | ES_CENTER,
        0, 0, 10, 10, hwnd, reinterpret_cast<HMENU>(static_cast<INT_PTR>(kGameCustomSizeXId)), hInstance, nullptr);
    app.customSizeYLabel = CreateWindowW(L"STATIC", L"Y", WS_CHILD,
        0, 0, 10, 10, hwnd, nullptr, hInstance, nullptr);
    app.customSizeY = CreateWindowW(L"EDIT", L"15", WS_CHILD | WS_BORDER | ES_NUMBER | ES_CENTER,
        0, 0, 10, 10, hwnd, reinterpret_cast<HMENU>(static_cast<INT_PTR>(kGameCustomSizeYId)), hInstance, nullptr);
    app.customPages = CreateWindowW(L"BUTTON", L"8 pages collectible", WS_CHILD | BS_AUTOCHECKBOX,
        0, 0, 10, 10, hwnd, reinterpret_cast<HMENU>(static_cast<INT_PTR>(kGameCustomPagesId)), hInstance, nullptr);
    app.customStart = CreateWindowW(L"BUTTON", L"Start", WS_CHILD | BS_PUSHBUTTON,
        0, 0, 10, 10, hwnd, reinterpret_cast<HMENU>(static_cast<INT_PTR>(kGameCustomStartId)), hInstance, nullptr);
    app.customBack = CreateWindowW(L"BUTTON", L"Back", WS_CHILD | BS_PUSHBUTTON,
        0, 0, 10, 10, hwnd, reinterpret_cast<HMENU>(static_cast<INT_PTR>(kGameCustomBackId)), hInstance, nullptr);
    HWND defaultChecked[] = {
        app.customScareBrokenLamp,
        app.customScareAirVent,
        app.customScareWater,
        app.customScareBlood,
        app.customScareFlesh,
        app.customBossOmukade,
        app.customPages
    };
    for (HWND control : defaultChecked) SendMessageW(control, BM_SETCHECK, BST_CHECKED, 0);

    CreateDebugSliceControls(app, hwnd, hInstance, true);

    HWND controls[] = {
        app.gameTitle, app.gameSinglePlayer, app.gameSettings, app.gameDebug, app.gameExit, app.gameBack,
        app.customTitle, app.customLayerLabel, app.customLayer, app.customScaresLabel,
        app.customScareBrokenLamp, app.customScareAirVent, app.customScareWater, app.customScareBlood,
        app.customScareFlesh, app.customBossesLabel, app.customBossOmukade, app.customSizeLabel,
        app.customSizeXLabel, app.customSizeX, app.customSizeYLabel, app.customSizeY,
        app.customPages, app.customStart, app.customBack,
        app.debugPrevEffect, app.debugNextEffect, app.debugSize, app.debugReset, app.debugPrevProp, app.debugNextProp,
        app.debugSettings
    };
    for (HWND control : controls) ApplyDefaultGuiFont(control);
}

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
