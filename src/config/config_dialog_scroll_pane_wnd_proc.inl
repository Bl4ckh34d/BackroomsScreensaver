// Config dialog scroll pane window procedure.

LRESULT CALLBACK ConfigScrollPaneWndProc(HWND hwnd, UINT msg, WPARAM wParam, LPARAM lParam) {
    switch (msg) {
    case WM_ERASEBKGND: {
        RECT rc{};
        GetClientRect(hwnd, &rc);
        FillRect(reinterpret_cast<HDC>(wParam), &rc, GetSysColorBrush(COLOR_WINDOW));
        return 1;
    }
    }
    return DefWindowProcW(hwnd, msg, wParam, lParam);
}
