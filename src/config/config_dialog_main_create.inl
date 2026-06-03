// Config dialog main-window creation.

LRESULT OnConfigCreate(HWND hwnd, LPARAM lParam) {
        HFONT font = reinterpret_cast<HFONT>(GetStockObject(DEFAULT_GUI_FONT));
        ConfigState* state = new ConfigState();
        auto* cs = reinterpret_cast<CREATESTRUCTW*>(lParam);
        auto* params = cs ? reinterpret_cast<ConfigCreateParams*>(cs->lpCreateParams) : nullptr;
        if (params) {
            state->mode = params->mode;
            state->embedded = params->embedded;
        }
        state->hwnd = hwnd;
        BuildConfigModel(state);
        SetWindowLongPtrW(hwnd, GWLP_USERDATA, reinterpret_cast<LONG_PTR>(state));

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

        constexpr int kColumnCount = 2;
        constexpr int kColumnX[kColumnCount] = {26, 376};
        constexpr int kLabelW = 160;
        constexpr int kControlOffset = 170;
        std::array<std::array<int, kColumnCount>, kConfigMaxTabCount> y{};
        std::array<std::array<const wchar_t*, kColumnCount>, kConfigMaxTabCount> group{};
        for (auto& tabY : y) {
            tabY.fill(0);
        }

        for (const auto& def : state->fieldDefs) {
            size_t tabIndex = static_cast<size_t>(def.tab);
            size_t colIndex = static_cast<size_t>(def.column);
            if (tabIndex >= static_cast<size_t>(state->tabCount) || colIndex >= static_cast<size_t>(kColumnCount)) continue;
            if (group[tabIndex][colIndex] != def.group) {
                if (y[tabIndex][colIndex] > 0) y[tabIndex][colIndex] += 10;
                HWND header = CreateWindowW(L"STATIC", def.group, WS_CHILD | SS_LEFT,
                    kColumnX[colIndex], y[tabIndex][colIndex], 320, 22, state->scrollPane, nullptr, nullptr, nullptr);
                SendMessageW(header, WM_SETFONT, reinterpret_cast<WPARAM>(font), TRUE);
                state->headers.push_back({def.tab, header, y[tabIndex][colIndex]});
                group[tabIndex][colIndex] = def.group;
                y[tabIndex][colIndex] += 26;
            }

            int fieldY = y[tabIndex][colIndex];
            y[tabIndex][colIndex] += 30;
            HWND label = CreateWindowW(L"STATIC", def.label, WS_CHILD | SS_RIGHT | SS_NOPREFIX | SS_ENDELLIPSIS,
                kColumnX[colIndex], fieldY + 4, kLabelW, 22, state->scrollPane, nullptr, nullptr, nullptr);
            HWND control = nullptr;
            if (def.kind == ConfigFieldKind::Bool) {
                control = CreateWindowW(L"BUTTON", L"", WS_CHILD | BS_AUTOCHECKBOX,
                    kColumnX[colIndex] + kControlOffset, fieldY + 2, 24, 22, state->scrollPane, reinterpret_cast<HMENU>(static_cast<INT_PTR>(def.id)), nullptr, nullptr);
            } else {
                control = CreateWindowExW(WS_EX_CLIENTEDGE, L"EDIT", L"", WS_CHILD | ES_AUTOHSCROLL,
                    kColumnX[colIndex] + kControlOffset, fieldY, def.width, 24, state->scrollPane, reinterpret_cast<HMENU>(static_cast<INT_PTR>(def.id)), nullptr, nullptr);
            }
            HWND slider = nullptr;
            SendMessageW(label, WM_SETFONT, reinterpret_cast<WPARAM>(font), TRUE);
            SendMessageW(control, WM_SETFONT, reinterpret_cast<WPARAM>(font), TRUE);
            state->fields.push_back({&def, label, control, slider, fieldY + 4, fieldY, fieldY - 1});
        }
        for (int tab = 0; tab < state->tabCount; ++tab) {
            int bottom = std::max(y[static_cast<size_t>(tab)][0], y[static_cast<size_t>(tab)][1]) + 8;
            state->contentHeight[static_cast<size_t>(tab)] = std::max(ConfigVisibleHeight(), bottom);
        }
        state->scrollBar = CreateWindowExW(0, L"SCROLLBAR", L"",
            WS_CHILD | SBS_VERT, 714, kConfigContentTop, 18, ConfigVisibleHeight(),
            hwnd, reinterpret_cast<HMENU>(static_cast<INT_PTR>(kConfigScrollId)), nullptr, nullptr);

        LoadConfigControls(state, false);
        ShowConfigTab(state, 0);

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
}
