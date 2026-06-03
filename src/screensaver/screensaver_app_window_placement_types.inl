struct ScreensaverWindowPlacement {
    DWORD style = WS_POPUP;
    DWORD exStyle = WS_EX_TOPMOST;
    HWND parent = nullptr;
    int x = 0;
    int y = 0;
    int width = 1;
    int height = 1;
};
