        HWND save = CreateWindowW(L"BUTTON", L"Save", WS_CHILD | WS_VISIBLE | BS_DEFPUSHBUTTON,
            12, 660, 96, 30, hwnd, reinterpret_cast<HMENU>(static_cast<INT_PTR>(kConfigSaveId)), nullptr, nullptr);
        HWND reset = CreateWindowW(L"BUTTON", L"Reset defaults", WS_CHILD | WS_VISIBLE,
            116, 660, 126, 30, hwnd, reinterpret_cast<HMENU>(static_cast<INT_PTR>(kConfigResetId)), nullptr, nullptr);
        HWND open = CreateWindowW(L"BUTTON", L"Open INI", WS_CHILD | WS_VISIBLE,
            250, 660, 104, 30, hwnd, reinterpret_cast<HMENU>(static_cast<INT_PTR>(kConfigOpenId)), nullptr, nullptr);
        HWND close = CreateWindowW(L"BUTTON", L"Close", WS_CHILD | WS_VISIBLE,
            628, 660, 104, 30, hwnd, reinterpret_cast<HMENU>(IDCANCEL), nullptr, nullptr);
        SendMessageW(save, WM_SETFONT, reinterpret_cast<WPARAM>(font), TRUE);
        SendMessageW(reset, WM_SETFONT, reinterpret_cast<WPARAM>(font), TRUE);
        SendMessageW(open, WM_SETFONT, reinterpret_cast<WPARAM>(font), TRUE);
        SendMessageW(close, WM_SETFONT, reinterpret_cast<WPARAM>(font), TRUE);
        return 0;
