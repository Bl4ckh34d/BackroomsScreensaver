#include "game_settings_panel_internal.h"

LRESULT CALLBACK GameSettingsPanelWndProc(HWND hwnd, UINT msg, WPARAM wParam, LPARAM lParam) {
    auto* state = reinterpret_cast<GameSettingsPanelState*>(GetWindowLongPtrW(hwnd, GWLP_USERDATA));
    switch (msg) {
    case WM_CREATE: {
        state = new GameSettingsPanelState();
        auto* create = reinterpret_cast<CREATESTRUCTW*>(lParam);
        if (create && create->lpCreateParams) {
            state->host = *reinterpret_cast<GameSettingsPanelHost*>(create->lpCreateParams);
        }
        state->settings = LoadSettings();
        BuildGameResolutionOptions(state);
        SetWindowLongPtrW(hwnd, GWLP_USERDATA, reinterpret_cast<LONG_PTR>(state));
        return 0;
    }
    case WM_GETDLGCODE:
        return DLGC_WANTALLKEYS;
    case WM_PAINT: {
        PAINTSTRUCT ps{};
        HDC dc = BeginPaint(hwnd, &ps);
        PaintGameSettingsPanel(hwnd, dc, state);
        EndPaint(hwnd, &ps);
        return 0;
    }
    case WM_LBUTTONDOWN: {
        if (!state) break;
        HDC hitDc = GetDC(hwnd);
        if (hitDc) {
            PaintGameSettingsPanel(hwnd, hitDc, state);
            ReleaseDC(hwnd, hitDc);
        }
        POINT p{GET_X_LPARAM(lParam), GET_Y_LPARAM(lParam)};
        RECT client{};
        GetClientRect(hwnd, &client);
        RECT saveRect{client.right - 300, client.bottom - 58, client.right - 178, client.bottom - 24};
        RECT closeRect{client.right - 160, client.bottom - 58, client.right - 38, client.bottom - 24};
        if (PtInRectInclusive(saveRect, p)) {
            SaveAndCloseGameSettingsPanel(hwnd, state);
            return 0;
        }
        if (PtInRectInclusive(closeRect, p)) {
            DestroyWindow(hwnd);
            return 0;
        }
        int directId = FindGameSettingsHitId(state, p);
        if (directId == kGameSettingsSave) {
            SaveAndCloseGameSettingsPanel(hwnd, state);
            return 0;
        }
        if (directId == kGameSettingsClose) {
            DestroyWindow(hwnd);
            return 0;
        }
        for (const auto& hit : state->hits) {
            if (!PtInRectInclusive(hit.rect, p)) continue;
            if (hit.kind == GameSettingsControlKind::Tab ||
                (hit.id >= kGameSettingsTabSystem && hit.id <= kGameSettingsTabAudio)) {
                state->tab = std::clamp(hit.id - kGameSettingsTabSystem, 0, 4);
                state->resolutionDropdownOpen = false;
                state->antiAliasingDropdownOpen = false;
                state->anisotropyDropdownOpen = false;
            } else if (hit.kind == GameSettingsControlKind::Check) {
                ToggleGameSetting(state, hit.id);
                state->resolutionDropdownOpen = false;
                state->antiAliasingDropdownOpen = false;
                state->anisotropyDropdownOpen = false;
            } else if (hit.kind == GameSettingsControlKind::Slider) {
                state->draggingSlider = hit.id;
                SetCapture(hwnd);
                ApplyGameSettingsSlider(state, hit.id, p.x);
                state->resolutionDropdownOpen = false;
                state->antiAliasingDropdownOpen = false;
                state->anisotropyDropdownOpen = false;
            } else if (hit.kind == GameSettingsControlKind::Dropdown && hit.id == kGameSettingsResolutionDropdown) {
                state->resolutionDropdownOpen = !state->resolutionDropdownOpen;
                state->antiAliasingDropdownOpen = false;
                state->anisotropyDropdownOpen = false;
            } else if (hit.kind == GameSettingsControlKind::Dropdown && hit.id == kGameSettingsAntiAliasingDropdown) {
                state->antiAliasingDropdownOpen = !state->antiAliasingDropdownOpen;
                state->resolutionDropdownOpen = false;
                state->anisotropyDropdownOpen = false;
            } else if (hit.kind == GameSettingsControlKind::Dropdown && hit.id == kGameSettingsAnisotropyDropdown) {
                state->anisotropyDropdownOpen = !state->anisotropyDropdownOpen;
                state->resolutionDropdownOpen = false;
                state->antiAliasingDropdownOpen = false;
            } else if (hit.kind == GameSettingsControlKind::DropdownOption) {
                int index = hit.id - kGameSettingsResolutionOptionBase;
                if (index >= 0 && index < static_cast<int>(state->resolutionOptions.size())) {
                    POINT res = state->resolutionOptions[static_cast<size_t>(index)];
                    state->settings.gameResolutionWidth = res.x;
                    state->settings.gameResolutionHeight = res.y;
                } else if (hit.id >= kGameSettingsAntiAliasingOptionBase && hit.id < kGameSettingsAntiAliasingOptionBase + 6) {
                    constexpr int values[] = {0, 1, 2, 4, 8, 16};
                    state->settings.antiAliasing = values[hit.id - kGameSettingsAntiAliasingOptionBase];
                    state->settings.fxaaEnabled = AntiAliasingUsesFxaa(state->settings.antiAliasing);
                } else if (hit.id >= kGameSettingsAnisotropyOptionBase && hit.id < kGameSettingsAnisotropyOptionBase + 5) {
                    constexpr int values[] = {1, 2, 4, 8, 16};
                    state->settings.textureAnisotropy = values[hit.id - kGameSettingsAnisotropyOptionBase];
                }
                state->resolutionDropdownOpen = false;
                state->antiAliasingDropdownOpen = false;
                state->anisotropyDropdownOpen = false;
            } else if (hit.id >= kGameSettingsKeybindBase &&
                hit.id < kGameSettingsKeybindBase + kGameInputActionCount) {
                state->capturingKeyAction = hit.id - kGameSettingsKeybindBase;
                state->resolutionDropdownOpen = false;
                state->antiAliasingDropdownOpen = false;
                state->anisotropyDropdownOpen = false;
                NotifyGameSettingsKeyCapture(state, true, false);
                SetFocus(hwnd);
            } else if (hit.id == kGameSettingsSave) {
                SaveAndCloseGameSettingsPanel(hwnd, state);
                return 0;
            } else if (hit.id == kGameSettingsClose) {
                DestroyWindow(hwnd);
                return 0;
            }
            InvalidateRect(hwnd, nullptr, TRUE);
            return 0;
        }
        state->resolutionDropdownOpen = false;
        state->antiAliasingDropdownOpen = false;
        state->anisotropyDropdownOpen = false;
        InvalidateRect(hwnd, nullptr, TRUE);
        return 0;
    }
    case WM_KEYDOWN:
    case WM_SYSKEYDOWN:
        if (state && state->capturingKeyAction >= 0) {
            if (wParam == VK_ESCAPE) {
                state->capturingKeyAction = -1;
                NotifyGameSettingsKeyCapture(state, false, true);
            } else if (wParam > 0 && wParam < 256) {
                AssignGameActionKey(state->settings,
                    static_cast<GameInputAction>(state->capturingKeyAction),
                    static_cast<int>(wParam));
                state->capturingKeyAction = -1;
                NotifyGameSettingsKeyCapture(state, false, false);
            }
            InvalidateRect(hwnd, nullptr, TRUE);
            return 0;
        }
        break;
    case WM_MOUSEMOVE:
        if (state && state->draggingSlider != 0 && (wParam & MK_LBUTTON)) {
            ApplyGameSettingsSlider(state, state->draggingSlider, GET_X_LPARAM(lParam));
            InvalidateRect(hwnd, nullptr, TRUE);
            return 0;
        }
        break;
    case WM_LBUTTONUP:
        if (state && state->draggingSlider != 0) {
            state->draggingSlider = 0;
            if (GetCapture() == hwnd) ReleaseCapture();
            return 0;
        }
        break;
    case WM_ERASEBKGND:
        return 1;
    case WM_DESTROY:
        NotifyGameSettingsKeyCapture(state, false, false);
        {
            GameSettingsPanelHost host = state ? state->host : GameSettingsPanelHost{};
            if (host.onClosed) host.onClosed(host.context, hwnd);
        }
        delete state;
        SetWindowLongPtrW(hwnd, GWLP_USERDATA, 0);
        return 0;
    }
    return DefWindowProcW(hwnd, msg, wParam, lParam);
}
