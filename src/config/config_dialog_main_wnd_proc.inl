#include "config_dialog_main_create.inl"

// Config dialog main window procedure.

LRESULT CALLBACK ConfigWndProc(HWND hwnd, UINT msg, WPARAM wParam, LPARAM lParam) {
    ConfigState* state = reinterpret_cast<ConfigState*>(GetWindowLongPtrW(hwnd, GWLP_USERDATA));
    switch (msg) {
    case WM_CREATE:
        return OnConfigCreate(hwnd, lParam);
    case WM_NOTIFY:
        if (state && reinterpret_cast<NMHDR*>(lParam)->idFrom == kConfigTabId &&
            reinterpret_cast<NMHDR*>(lParam)->code == TCN_SELCHANGE) {
            ShowConfigTab(state, TabCtrl_GetCurSel(state->tab));
        }
        return 0;
    case WM_TIMER:
        if (state && wParam == kConfigPreviewTimerId) {
            if (state->previewPending && GetTickCount64() >= state->previewApplyAt) {
                RestartConfigPreview(state);
            }
            if (state->previewRenderer) state->previewRenderer->Tick();
            return 0;
        }
        break;
    case WM_VSCROLL:
        if (state && reinterpret_cast<HWND>(lParam) == state->scrollBar) {
            int offset = state->scrollOffset[static_cast<size_t>(state->activeTab)];
            int maxScroll = ConfigMaxScroll(state, state->activeTab);
            switch (LOWORD(wParam)) {
            case SB_LINEUP: offset -= kConfigScrollStep; break;
            case SB_LINEDOWN: offset += kConfigScrollStep; break;
            case SB_PAGEUP: offset -= ConfigVisibleHeight() - kConfigScrollStep; break;
            case SB_PAGEDOWN: offset += ConfigVisibleHeight() - kConfigScrollStep; break;
            case SB_THUMBTRACK:
            case SB_THUMBPOSITION: {
                SCROLLINFO si{};
                si.cbSize = sizeof(si);
                si.fMask = SIF_TRACKPOS;
                GetScrollInfo(state->scrollBar, SB_CTL, &si);
                offset = si.nTrackPos;
                break;
            }
            case SB_TOP: offset = 0; break;
            case SB_BOTTOM: offset = maxScroll; break;
            default: break;
            }
            SetConfigScrollOffset(state, offset);
            ApplyConfigScroll(state);
            return 0;
        }
        break;
    case WM_MOUSEWHEEL:
        if (state && ConfigActiveTabIsMonsterPreview(state) && state->previewRenderer) {
            POINT screenPoint{GET_X_LPARAM(lParam), GET_Y_LPARAM(lParam)};
            if (ConfigPointOverPreview(state, screenPoint)) {
                ZoomConfigPreviewOrbit(state, wParam);
                return 0;
            }
        }
        if (state && ConfigMaxScroll(state, state->activeTab) > 0) {
            int detents = GET_WHEEL_DELTA_WPARAM(wParam) / WHEEL_DELTA;
            int offset = state->scrollOffset[static_cast<size_t>(state->activeTab)] - detents * kConfigScrollStep;
            SetConfigScrollOffset(state, offset);
            ApplyConfigScroll(state);
            return 0;
        }
        break;
    case WM_LBUTTONDOWN:
    case WM_RBUTTONDOWN:
        if (state && ConfigActiveTabIsMonsterPreview(state)) {
            POINT screenPoint = ConfigClientPointToScreen(hwnd, lParam);
            if (ConfigPointOverPreview(state, screenPoint)) {
                BeginConfigPreviewOrbit(state, hwnd, screenPoint);
                return 0;
            }
        }
        break;
    case WM_LBUTTONUP:
    case WM_RBUTTONUP:
        if (state && state->previewOrbitDragging && GetCapture() == hwnd) {
            EndConfigPreviewOrbit(state, hwnd);
            return 0;
        }
        break;
    case WM_CAPTURECHANGED:
        if (state && state->previewOrbitDragging && reinterpret_cast<HWND>(lParam) != hwnd && GetCapture() != state->preview) {
            state->previewOrbitDragging = false;
        }
        break;
    case WM_MOUSEMOVE:
        if (state && state->previewOrbitDragging && ConfigActiveTabIsMonsterPreview(state) && GetCapture() == hwnd) {
            UpdateConfigPreviewOrbit(state, ConfigClientPointToScreen(hwnd, lParam));
            return 0;
        }
        break;
    case WM_COMMAND: {
        int id = LOWORD(wParam);
        if (id == kConfigSaveId && state) {
            SaveConfigControls(state);
            if (state->mode == ConfigDialogMode::Debug && gApp
#if defined(BACKROOMS_GAME_EXE)
                && gApp->rendererInitialized
#endif
                ) {
                gApp->renderer.ApplyGameSettings(LoadSettings());
            }
            const wchar_t* message = state->mode == ConfigDialogMode::Game
                ? L"Game settings saved. Display settings apply next launch; start a new run to reload level-generation settings."
                : (state->mode == ConfigDialogMode::Debug
                    ? L"Debug settings saved. Re-enter Debug or update the preview to reload scene-generation settings."
                    : L"Settings saved. Restart the screensaver to use changed values.");
            MessageBoxW(hwnd, message, L"Backrooms Maze", MB_OK | MB_ICONINFORMATION);
            return 0;
        }
        if (id == kConfigResetId && state) {
            const wchar_t* prompt = state->mode == ConfigDialogMode::Full
                ? L"Reset all settings to defaults?"
                : L"Reset the visible settings in this view to defaults?";
            if (MessageBoxW(hwnd, prompt, L"Backrooms Maze", MB_YESNO | MB_ICONQUESTION) == IDYES) {
                if (state->mode == ConfigDialogMode::Full) {
                    WriteTextFile(SettingsPath(), DefaultConfigText());
                    LoadConfigControls(state, true);
                } else {
                    ResetVisibleConfigControls(state);
                }
                ScheduleConfigPreview(state, 0);
            }
            return 0;
        }
        if (id == kConfigOpenId) {
            std::wstring arg = L"\"" + SettingsPath().wstring() + L"\"";
            ShellExecuteW(hwnd, L"open", L"notepad.exe", arg.c_str(), nullptr, SW_SHOWNORMAL);
            return 0;
        }
        if (id == kConfigPreviewUpdateId && state) {
            RestartConfigPreview(state);
            return 0;
        }
        if (id == IDCANCEL) {
            DestroyWindow(hwnd);
            return 0;
        }
        if (state) {
            ConfigFieldUi* field = FindConfigFieldById(state, id);
            WORD code = HIWORD(wParam);
            if (field && (code == EN_CHANGE || code == BN_CLICKED)) {
                ScheduleConfigPreview(state);
                return 0;
            }
        }
        break;
    }
    case WM_CLOSE:
        DestroyWindow(hwnd);
        return 0;
    case WM_DESTROY:
        if (state) {
            KillTimer(hwnd, kConfigPreviewTimerId);
            state->previewRenderer.reset();
        }
        if (state && state->embedded) {
            if (gApp
#if defined(BACKROOMS_GAME_EXE)
                && gApp->gameConfig == hwnd
#else
                && false
#endif
                ) {
#if defined(BACKROOMS_GAME_EXE)
                gApp->gameConfig = nullptr;
                PostMessageW(gApp->hwnd, kGameConfigClosedMessage, 0, 0);
#endif
            }
        } else {
            PostQuitMessage(0);
        }
        return 0;
    case WM_NCDESTROY:
        delete state;
        SetWindowLongPtrW(hwnd, GWLP_USERDATA, 0);
        return 0;
    }
    return DefWindowProcW(hwnd, msg, wParam, lParam);
}
