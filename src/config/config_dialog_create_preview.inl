        HWND previewTitle = CreateWindowW(L"STATIC", L"Live preview", WS_CHILD | WS_VISIBLE | SS_LEFT,
            754, 16, 430, 22, hwnd, nullptr, nullptr, nullptr);
        SendMessageW(previewTitle, WM_SETFONT, reinterpret_cast<WPARAM>(font), TRUE);
        state->preview = CreateWindowExW(WS_EX_CLIENTEDGE, L"BackroomsMazeConfigPreview", L"",
            WS_CHILD | WS_VISIBLE | WS_CLIPSIBLINGS, 754, 44, 430, 300,
            hwnd, reinterpret_cast<HMENU>(static_cast<INT_PTR>(kConfigPreviewId)), nullptr, state);
        state->previewStatus = CreateWindowW(L"STATIC", L"", WS_CHILD | SS_CENTER,
            774, 174, 390, 24, hwnd, nullptr, nullptr, nullptr);
        SendMessageW(state->previewStatus, WM_SETFONT, reinterpret_cast<WPARAM>(font), TRUE);
        state->previewHint = CreateWindowW(L"STATIC", L"Edit several values, then click Update preview to rebuild the embedded screensaver once.",
            WS_CHILD | WS_VISIBLE | SS_LEFT, 754, 354, 430, 42, hwnd, nullptr, nullptr, nullptr);
        SendMessageW(state->previewHint, WM_SETFONT, reinterpret_cast<WPARAM>(font), TRUE);
        state->previewUpdateButton = CreateWindowW(L"BUTTON", L"Update preview", WS_CHILD | WS_VISIBLE,
            754, 404, 128, 30, hwnd, reinterpret_cast<HMENU>(static_cast<INT_PTR>(kConfigPreviewUpdateId)), nullptr, nullptr);
        SendMessageW(state->previewUpdateButton, WM_SETFONT, reinterpret_cast<WPARAM>(font), TRUE);
        SetTimer(hwnd, kConfigPreviewTimerId, 16, nullptr);
        UpdateConfigPreviewHint(state);
        SetConfigPreviewStatus(state, L"Click Update preview to render current settings.");
