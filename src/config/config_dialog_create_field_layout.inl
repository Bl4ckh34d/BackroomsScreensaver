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
