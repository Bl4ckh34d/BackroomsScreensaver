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
