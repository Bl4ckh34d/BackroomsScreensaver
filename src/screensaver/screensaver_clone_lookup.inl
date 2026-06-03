// Screensaver clone-output lookup by HWND.
// Included from app_runtime.inl after App is declared.

App::CloneOutput* CloneForWindow(HWND hwnd) {
    if (!gApp) return nullptr;
    for (auto& clone : gApp->clones) {
        if (clone && clone->hwnd == hwnd) return clone.get();
    }
    return nullptr;
}
