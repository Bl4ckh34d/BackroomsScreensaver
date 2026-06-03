// Config dialog preview window procedure.

LRESULT CALLBACK ConfigPreviewWndProc(HWND hwnd, UINT msg, WPARAM wParam, LPARAM lParam) {
    if (msg == WM_NCCREATE) {
        auto* cs = reinterpret_cast<CREATESTRUCTW*>(lParam);
        SetWindowLongPtrW(hwnd, GWLP_USERDATA, reinterpret_cast<LONG_PTR>(cs->lpCreateParams));
        return TRUE;
    }

    ConfigState* state = reinterpret_cast<ConfigState*>(GetWindowLongPtrW(hwnd, GWLP_USERDATA));
    switch (msg) {
    case WM_SIZE:
        if (state && state->previewRenderer) {
            state->previewRenderer->Resize(LOWORD(lParam), HIWORD(lParam));
        }
        return 0;
    case WM_LBUTTONDOWN:
    case WM_RBUTTONDOWN:
        if (state && ConfigActiveTabIsMonsterPreview(state)) {
            BeginConfigPreviewOrbit(state, hwnd, ConfigClientPointToScreen(hwnd, lParam));
            return 0;
        }
        break;
    case WM_LBUTTONUP:
    case WM_RBUTTONUP:
        if (state && state->previewOrbitDragging && GetCapture() == hwnd) {
            EndConfigPreviewOrbit(state, hwnd);
            return 0;
        }
        break;
    case WM_CAPTURECHANGED:
        if (state && reinterpret_cast<HWND>(lParam) != hwnd) {
            state->previewOrbitDragging = false;
        }
        break;
    case WM_MOUSEMOVE:
        if (state && state->previewOrbitDragging && ConfigActiveTabIsMonsterPreview(state)) {
            UpdateConfigPreviewOrbit(state, ConfigClientPointToScreen(hwnd, lParam));
            return 0;
        }
        break;
    case WM_MOUSEWHEEL:
        if (state && ConfigActiveTabIsMonsterPreview(state) && state->previewRenderer) {
            ZoomConfigPreviewOrbit(state, wParam);
            return 0;
        }
        break;
    case WM_ERASEBKGND:
        return 1;
    }
    return DefWindowProcW(hwnd, msg, wParam, lParam);
}
