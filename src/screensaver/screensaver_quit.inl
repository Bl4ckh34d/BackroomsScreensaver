// Screensaver target shutdown helper.
// Included from app_runtime.inl before shared window message routing.

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
