// BackroomsMazeGame.exe window procedure.
// Included from app_runtime.inl after game window-procedure helpers.

LRESULT CALLBACK GameWndProc(HWND hwnd, UINT msg, WPARAM wParam, LPARAM lParam) {
    switch (msg) {
    case WM_CREATE:
        return 0;
    case WM_SIZE:
        if (gApp && hwnd == gApp->hwnd) {
            int w = LOWORD(lParam);
            int h = HIWORD(lParam);
            ResizePrimaryHostOutput(hwnd, w, h);
            ResizeGameConfigPanel(w, h);
            if (HandleGameShellResize(hwnd, wParam)) return 0;
        }
        return 0;
    case WM_ERASEBKGND:
        if (HandleGameEraseBackground(hwnd)) return 1;
        break;
    case WM_PAINT:
        if (HandleGamePaint(hwnd)) return 0;
        break;
    case kGameConfigClosedMessage:
        if (HandleGameConfigClosedMessage(hwnd)) return 0;
        break;
    case WM_SETCURSOR:
        if (HandleGameSetCursor()) return TRUE;
        break;
    case WM_MOUSEMOVE:
        if (HandleGameMouseMove(hwnd, lParam)) return 0;
        return 0;
    case WM_MOUSELEAVE:
        if (HandleGameMouseLeave(hwnd)) return 0;
        break;
    case WM_LBUTTONDOWN:
    case WM_MBUTTONDOWN:
    case WM_RBUTTONDOWN:
    case WM_KEYDOWN:
    case WM_SYSKEYDOWN:
        if (HandleGameButtonOrKeyDown(hwnd, msg, lParam)) return 0;
        return 0;
    case WM_COMMAND:
        if (HandleGameCommand(hwnd, wParam)) return 0;
        if (HandleDebugSliceCommand(hwnd, wParam)) return 0;
        break;
    case WM_ACTIVATEAPP:
        if (HandleGameActivateApp(wParam)) return 0;
        return 0;
    case WM_DESTROY:
        if (gApp && gApp->gameShell) ReleaseGameMouse();
        PostQuitMessage(0);
        return 0;
    }
    return DefWindowProcW(hwnd, msg, wParam, lParam);
}
