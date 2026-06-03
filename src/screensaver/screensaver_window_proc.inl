// BackroomsMaze.scr window procedure.
// Included from app_runtime.inl after screensaver window-procedure helpers.

LRESULT CALLBACK ScreensaverWndProc(HWND hwnd, UINT msg, WPARAM wParam, LPARAM lParam) {
    switch (msg) {
    case WM_CREATE:
        return 0;
    case WM_SIZE:
        if (gApp) {
            int w = LOWORD(lParam);
            int h = HIWORD(lParam);
            if (hwnd == gApp->hwnd) {
                ResizePrimaryHostOutput(hwnd, w, h);
            } else {
                HandleScreensaverCloneResize(hwnd, w, h);
            }
        }
        return 0;
    case WM_SETCURSOR:
        if (HandleScreensaverSetCursor()) return TRUE;
        break;
    case WM_MOUSEMOVE:
        HandleScreensaverMouseMove(hwnd, lParam);
        return 0;
    case WM_LBUTTONDOWN:
    case WM_MBUTTONDOWN:
    case WM_RBUTTONDOWN:
    case WM_KEYDOWN:
    case WM_SYSKEYDOWN:
        HandleScreensaverQuitInput(hwnd);
        return 0;
    case WM_COMMAND:
        if (HandleDebugSliceCommand(hwnd, wParam)) return 0;
        break;
    case WM_ACTIVATEAPP:
        HandleScreensaverActivateApp(hwnd, wParam);
        return 0;
    case WM_DESTROY:
        PostQuitMessage(0);
        return 0;
    }
    return DefWindowProcW(hwnd, msg, wParam, lParam);
}
