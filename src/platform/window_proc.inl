// Main window procedure, screensaver quit handling, and command-line mode parsing.
// Included from main.cpp after app state and loading overlay helpers.

void QuitScreensaver(HWND hwnd) {
    if (gApp && !gApp->preview && !gApp->quitting) {
        gApp->quitting = true;
        for (auto& clone : gApp->clones) {
            if (clone && clone->hwnd && IsWindow(clone->hwnd)) {
                DestroyWindow(clone->hwnd);
            }
        }
        if (gApp->hwnd && IsWindow(gApp->hwnd)) {
            DestroyWindow(gApp->hwnd);
        } else if (hwnd) {
            DestroyWindow(hwnd);
        }
        return;
    }
    DestroyWindow(hwnd);
}

LRESULT CALLBACK WndProc(HWND hwnd, UINT msg, WPARAM wParam, LPARAM lParam) {
    switch (msg) {
    case WM_CREATE:
        return 0;
    case WM_SIZE:
        if (gApp) {
            int w = LOWORD(lParam);
            int h = HIWORD(lParam);
            if (hwnd == gApp->hwnd) {
                if (gApp->loadingOverlay) ResizeLoadingOverlay(hwnd, gApp->loadingOverlay);
#if defined(BACKROOMS_GAME_EXE)
                if (gApp->gameConfig) MoveWindow(gApp->gameConfig, 0, 0, std::max(1, w), std::max(1, h), TRUE);
#endif
                gApp->renderer.Resize(w, h);
#if defined(BACKROOMS_GAME_EXE)
                if (gApp->gameShell) {
                    LayoutGameControls(hwnd);
                    if (gApp->gameMouseCaptured) CaptureGameMouse(hwnd);
                    if (gApp->gameState == GameState::MainMenu) InvalidateRect(hwnd, nullptr, TRUE);
                }
#endif
            } else if (App::CloneOutput* clone = CloneForWindow(hwnd)) {
                if (clone->loadingOverlay) ResizeLoadingOverlay(hwnd, clone->loadingOverlay);
                clone->renderer.Resize(w, h);
            }
        }
        return 0;
#if defined(BACKROOMS_GAME_EXE)
    case WM_ERASEBKGND:
        if (gApp && gApp->gameShell && gApp->gameState == GameState::MainMenu && hwnd == gApp->hwnd) {
            return 1;
        }
        break;
    case WM_PAINT:
        if (gApp && gApp->gameShell && gApp->gameState == GameState::MainMenu && hwnd == gApp->hwnd) {
            PAINTSTRUCT ps{};
            HDC dc = BeginPaint(hwnd, &ps);
            PaintGameMainMenu(hwnd, dc);
            EndPaint(hwnd, &ps);
            return 0;
        }
        break;
#endif
#if defined(BACKROOMS_GAME_EXE)
    case kGameConfigClosedMessage:
        if (gApp && gApp->gameShell && hwnd == gApp->hwnd) {
            if (gApp->gameSettingsReturnState == GameState::DebugScene) {
                EnterGameDebug(hwnd);
            } else {
                EnterGameMainMenu(hwnd);
            }
            return 0;
        }
        break;
#endif
    case WM_SETCURSOR:
#if defined(BACKROOMS_GAME_EXE)
        if (gApp && gApp->gameShell && gApp->gameState == GameState::PlayGame) {
            SetCursor(nullptr);
            return TRUE;
        }
        if (gApp && gApp->gameShell) {
            SetCursor(LoadCursorW(nullptr, IDC_ARROW));
            return TRUE;
        }
#endif
        if (gApp && !gApp->preview) {
            SetCursor(nullptr);
            return TRUE;
        }
        break;
    case WM_MOUSEMOVE:
#if defined(BACKROOMS_GAME_EXE)
        if (gApp && gApp->gameShell && gApp->gameState == GameState::PlayGame && hwnd == gApp->hwnd) {
            POINT p{GET_X_LPARAM(lParam), GET_Y_LPARAM(lParam)};
            if (gApp->gameRecenteringMouse) {
                gApp->gameRecenteringMouse = false;
                return 0;
            }
            gApp->gameMouseDeltaX += static_cast<float>(p.x - gApp->gameMouseCenter.x);
            gApp->gameMouseDeltaY += static_cast<float>(p.y - gApp->gameMouseCenter.y);
            POINT center = gApp->gameMouseCenter;
            ClientToScreen(hwnd, &center);
            gApp->gameRecenteringMouse = true;
            SetCursorPos(center.x, center.y);
            return 0;
        }
        if (gApp && gApp->gameShell && gApp->gameState == GameState::MainMenu && hwnd == gApp->hwnd) {
            POINT p{GET_X_LPARAM(lParam), GET_Y_LPARAM(lParam)};
            int hover = HitTestGameMenu(hwnd, p);
            gApp->gameMenuMouse = p;
            gApp->gameMenuHasMouse = true;
            if (hover == kGameSinglePlayerId && gApp->gameMenuBloodStart == 0) {
                gApp->gameMenuBloodStart = GetTickCount64();
            }
            if (hover != gApp->gameMenuHoverId) {
                gApp->gameMenuHoverId = hover;
                InvalidateRect(hwnd, nullptr, FALSE);
            }
            InvalidateRect(hwnd, nullptr, FALSE);
            return 0;
        }
#endif
        if (gApp && !gApp->preview) {
            POINT p{GET_X_LPARAM(lParam), GET_Y_LPARAM(lParam)};
            if (gApp->firstMouse) {
                gApp->initialMouse = p;
                gApp->firstMouse = false;
            } else {
                int dx = p.x - gApp->initialMouse.x;
                int dy = p.y - gApp->initialMouse.y;
                if (dx * dx + dy * dy > 36) QuitScreensaver(hwnd);
            }
        }
        return 0;
    case WM_LBUTTONDOWN:
    case WM_MBUTTONDOWN:
    case WM_RBUTTONDOWN:
    case WM_KEYDOWN:
    case WM_SYSKEYDOWN:
#if defined(BACKROOMS_GAME_EXE)
        if (gApp && gApp->gameShell) {
            if (msg == WM_LBUTTONDOWN && gApp->gameState == GameState::MainMenu && hwnd == gApp->hwnd) {
                POINT p{GET_X_LPARAM(lParam), GET_Y_LPARAM(lParam)};
                int id = HitTestGameMenu(hwnd, p);
                if (id != 0) ActivateGameMenuCommand(hwnd, id);
            }
            return 0;
        }
#endif
        if (gApp && !gApp->preview) QuitScreensaver(hwnd);
        return 0;
    case WM_COMMAND:
#if defined(BACKROOMS_GAME_EXE)
        if (gApp && gApp->gameShell && hwnd == gApp->hwnd) {
            int id = LOWORD(wParam);
            if (id == kGameSinglePlayerId) {
                ActivateGameMenuCommand(hwnd, id);
                return 0;
            }
            if (id == kGameSettingsId) {
                ActivateGameMenuCommand(hwnd, id);
                return 0;
            }
            if (id == kGameDebugId) {
                ActivateGameMenuCommand(hwnd, id);
                return 0;
            }
            if (id == kGameBackId) {
                if (gApp->gameState == GameState::Settings && gApp->gameConfig) {
                    DestroyWindow(gApp->gameConfig);
                    return 0;
                }
                EnterGameMainMenu(hwnd);
                return 0;
            }
            if (id == kDebugSettingsId) {
                EnterGameSettings(hwnd, ConfigDialogMode::Debug, GameState::DebugScene);
                return 0;
            }
            if (id == kGameExitId) {
                ActivateGameMenuCommand(hwnd, id);
                return 0;
            }
        }
#endif
        if (gApp && gEffectDebugViewer && hwnd == gApp->hwnd) {
            int id = LOWORD(wParam);
            if (id == kDebugPrevEffectId || id == kDebugNextEffectId || id == kDebugSizeId || id == kDebugResetId ||
                id == kDebugPrevPropId || id == kDebugNextPropId) {
                if (id == kDebugPrevEffectId) {
                    gDebugSliceEffect = StepDebugSliceEffect(gDebugSliceEffect, -1);
                } else if (id == kDebugNextEffectId) {
                    gDebugSliceEffect = StepDebugSliceEffect(gDebugSliceEffect, 1);
                } else if (id == kDebugSizeId) {
                    gDebugSliceTiles = gDebugSliceTiles >= 5 ? 1 : gDebugSliceTiles + 1;
                } else if (id == kDebugPrevPropId || id == kDebugNextPropId) {
                    if (gDebugSliceEffect != DebugSliceEffect::Props) {
                        gDebugSliceEffect = DebugSliceEffect::Props;
                    }
                    gDebugPropIndex = WrapDebugPropIndex(gDebugPropIndex + (id == kDebugPrevPropId ? -1 : 1));
                } else if (id == kDebugResetId) {
                    gApp->renderer.ResetDebugSliceAnimation();
                    UpdateDebugSliceControls(hwnd);
                    RedrawDebugSliceControls();
                    return 0;
                }
                gApp->renderer.ConfigureDebugSlice(gDebugSliceEffect, gDebugSliceTiles);
                UpdateDebugSliceControls(hwnd);
                RedrawDebugSliceControls();
                return 0;
            }
        }
        break;
    case WM_ACTIVATEAPP:
#if defined(BACKROOMS_GAME_EXE)
        if (gApp && gApp->gameShell) {
            if (wParam == FALSE) ReleaseGameMouse();
            else if (gApp->gameState == GameState::PlayGame) CaptureGameMouse(gApp->hwnd);
            return 0;
        }
#endif
        if (gApp && !gApp->preview && wParam == FALSE) QuitScreensaver(hwnd);
        return 0;
    case WM_DESTROY:
#if defined(BACKROOMS_GAME_EXE)
        if (gApp && gApp->gameShell) ReleaseGameMouse();
#endif
        PostQuitMessage(0);
        return 0;
    }
    return DefWindowProcW(hwnd, msg, wParam, lParam);
}

std::wstring Lower(std::wstring s) {
    std::transform(s.begin(), s.end(), s.begin(), [](wchar_t c) { return static_cast<wchar_t>(towlower(c)); });
    return s;
}

uintptr_t ParseHandle(const wchar_t* s) {
    if (!s) return 0;
    while (*s == L':' || *s == L' ') ++s;
    return static_cast<uintptr_t>(_wcstoui64(s, nullptr, 0));
}

RunMode ParseMode(int argc, wchar_t** argv, HWND& previewParent) {
    for (int i = 1; i < argc; ++i) {
        std::wstring arg = Lower(argv[i]);
        if (arg.rfind(L"/c", 0) == 0 || arg.rfind(L"-c", 0) == 0) return RunMode::Configure;
        if (arg == L"/selftest" || arg == L"-selftest") return RunMode::SelfTest;
        if (arg == L"/makeini" || arg == L"-makeini") return RunMode::GenerateIni;
        if (arg == L"/monsterpreviewfront" || arg == L"-monsterpreviewfront") return RunMode::MonsterPreviewFront;
        if (arg == L"/monsterpreviewside" || arg == L"-monsterpreviewside") return RunMode::MonsterPreviewSide;
        if (arg == L"/monsterpreviewleft" || arg == L"-monsterpreviewleft") return RunMode::MonsterPreviewLeftSide;
        if (arg == L"/monsterpreviewtop" || arg == L"-monsterpreviewtop") return RunMode::MonsterPreviewTop;
        if (arg == L"/monsterpreview" || arg == L"-monsterpreview") return RunMode::MonsterPreview;
        if (arg == L"/blooddebug" || arg == L"-blooddebug" ||
            arg == L"/effectdebug" || arg == L"-effectdebug") return RunMode::BloodDebug;
        if (arg.rfind(L"/p", 0) == 0 || arg.rfind(L"-p", 0) == 0) {
            uintptr_t handle = 0;
            if (arg.size() > 2) handle = ParseHandle(arg.c_str() + 2);
            if (!handle && i + 1 < argc) handle = ParseHandle(argv[++i]);
            previewParent = reinterpret_cast<HWND>(handle);
            return RunMode::Preview;
        }
        if (arg.rfind(L"/s", 0) == 0 || arg.rfind(L"-s", 0) == 0) return RunMode::Fullscreen;
    }
    return RunMode::Configure;
}
