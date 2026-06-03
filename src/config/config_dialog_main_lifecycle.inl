static void HandleConfigDestroy(HWND hwnd, ConfigState* state) {
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
}

static void HandleConfigNcDestroy(HWND hwnd, ConfigState* state) {
    delete state;
    SetWindowLongPtrW(hwnd, GWLP_USERDATA, 0);
}
