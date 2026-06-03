#pragma once

// App-wide host state and window handles.
// Included from main.cpp after Renderer is defined.

#include "run_mode.h"

struct App {
    Renderer renderer;
    bool preview = false;
    HWND hwnd = nullptr;
    HWND debugPrevEffect = nullptr;
    HWND debugNextEffect = nullptr;
    HWND debugSize = nullptr;
    HWND debugReset = nullptr;
    HWND debugPrevProp = nullptr;
    HWND debugNextProp = nullptr;
    HWND debugSettings = nullptr;
    HWND loadingOverlay = nullptr;

#if defined(BACKROOMS_GAME_EXE)
#include "../game/game_app_state_fields.inl"
#else
#include "../screensaver/screensaver_app_state_fields.inl"
#endif
};

inline App* gApp = nullptr;
