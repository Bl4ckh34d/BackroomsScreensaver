        state->tab = CreateWindowExW(0, WC_TABCONTROLW, L"", WS_CHILD | WS_VISIBLE | WS_CLIPSIBLINGS,
            12, 12, 720, 628, hwnd, reinterpret_cast<HMENU>(static_cast<INT_PTR>(kConfigTabId)), nullptr, nullptr);
        SendMessageW(state->tab, WM_SETFONT, reinterpret_cast<WPARAM>(font), TRUE);
        for (int i = 0; i < state->tabCount; ++i) {
            TCITEMW item{};
            item.mask = TCIF_TEXT;
            item.pszText = const_cast<wchar_t*>(state->tabLabels[static_cast<size_t>(i)].c_str());
            TabCtrl_InsertItem(state->tab, i, &item);
        }

        state->note = CreateWindowW(L"STATIC", state->tabNotes.empty() ? L"" : state->tabNotes[0].c_str(), WS_CHILD | WS_VISIBLE | SS_LEFT,
            30, 50, 660, 34, hwnd, nullptr, nullptr, nullptr);
        SendMessageW(state->note, WM_SETFONT, reinterpret_cast<WPARAM>(font), TRUE);
        state->scrollPane = CreateWindowExW(0, L"BackroomsMazeConfigScrollPane", L"",
            WS_CHILD | WS_VISIBLE | WS_CLIPCHILDREN | WS_CLIPSIBLINGS,
            0, kConfigContentTop, 714, ConfigVisibleHeight(), hwnd, nullptr, nullptr, nullptr);
