
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
