// BackroomsMaze.scr target-specific window message helpers.
// Included from app_runtime.inl before the shared window procedure.

bool HandleScreensaverCloneResize(HWND hwnd, int width, int height) {
    App::CloneOutput* clone = CloneForWindow(hwnd);
    if (!clone) return false;
    if (clone->loadingOverlay) ResizeLoadingOverlay(hwnd, clone->loadingOverlay);
    clone->renderer.Resize(width, height);
    return true;
}

bool HandleScreensaverSetCursor() {
    if (!gApp || gApp->preview) return false;
    SetCursor(nullptr);
    return true;
}

bool HandleScreensaverMouseMove(HWND hwnd, LPARAM lParam) {
    if (!gApp || gApp->preview) return false;
    POINT p{GET_X_LPARAM(lParam), GET_Y_LPARAM(lParam)};
    if (gApp->firstMouse) {
        gApp->initialMouse = p;
        gApp->firstMouse = false;
    } else {
        int dx = p.x - gApp->initialMouse.x;
        int dy = p.y - gApp->initialMouse.y;
        if (dx * dx + dy * dy > 36) QuitScreensaver(hwnd);
    }
    return true;
}

bool HandleScreensaverQuitInput(HWND hwnd) {
    if (!gApp || gApp->preview) return false;
    QuitScreensaver(hwnd);
    return true;
}

bool HandleScreensaverActivateApp(HWND hwnd, WPARAM wParam) {
    if (!gApp || gApp->preview || wParam != FALSE) return false;
    QuitScreensaver(hwnd);
    return true;
}
