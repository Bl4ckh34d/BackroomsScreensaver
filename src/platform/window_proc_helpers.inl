// Shared Win32 window-procedure helpers.
// Included before target-specific window procedures.

void ResizePrimaryHostOutput(HWND hwnd, int width, int height) {
    if (!gApp || hwnd != gApp->hwnd) return;
    if (gApp->loadingOverlay) ResizeLoadingOverlay(hwnd, gApp->loadingOverlay);
    gApp->renderer.Resize(width, height);
}
