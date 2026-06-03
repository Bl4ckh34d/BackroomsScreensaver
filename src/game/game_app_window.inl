// BackroomsMazeGame.exe window-class registration and launch placement.
// Included from game_app.inl before RunGame.

struct GameWindowLaunchPlacement {
    int x = 0;
    int y = 0;
    int width = 640;
    int height = 360;
    DWORD style = WS_OVERLAPPEDWINDOW | WS_VISIBLE | WS_CLIPCHILDREN | WS_CLIPSIBLINGS;
};

ATOM RegisterGameWindowClass(HINSTANCE hInstance, const wchar_t* className) {
    WNDCLASSW wc{};
    wc.lpfnWndProc = GameWndProc;
    wc.hInstance = hInstance;
    wc.lpszClassName = className;
    wc.hbrBackground = reinterpret_cast<HBRUSH>(GetStockObject(BLACK_BRUSH));
    return RegisterClassW(&wc);
}

GameWindowLaunchPlacement BuildGameWindowLaunchPlacement(const Settings& launchSettings) {
    GameWindowLaunchPlacement placement{};
    placement.width = std::clamp(launchSettings.gameResolutionWidth, 640, 7680);
    placement.height = std::clamp(launchSettings.gameResolutionHeight, 360, 4320);
    placement.x = std::max(0, (GetSystemMetrics(SM_CXSCREEN) - placement.width) / 2);
    placement.y = std::max(0, (GetSystemMetrics(SM_CYSCREEN) - placement.height) / 2);
    placement.style = launchSettings.gameFullscreen
        ? (WS_POPUP | WS_VISIBLE | WS_CLIPCHILDREN | WS_CLIPSIBLINGS)
        : (WS_OVERLAPPEDWINDOW | WS_VISIBLE | WS_CLIPCHILDREN | WS_CLIPSIBLINGS);
    if (launchSettings.gameFullscreen) {
        placement.x = 0;
        placement.y = 0;
        placement.width = GetSystemMetrics(SM_CXSCREEN);
        placement.height = GetSystemMetrics(SM_CYSCREEN);
    }
    return placement;
}

HWND CreateGameHostWindow(HINSTANCE hInstance, const wchar_t* className, const Settings& launchSettings) {
    GameWindowLaunchPlacement placement = BuildGameWindowLaunchPlacement(launchSettings);
    return CreateWindowExW(0, className, L"Backrooms Maze", placement.style,
        placement.x, placement.y, placement.width, placement.height,
        nullptr, nullptr, hInstance, nullptr);
}
