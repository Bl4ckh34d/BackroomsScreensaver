HWND CreateScreensaverHostWindow(
    HINSTANCE hInstance,
    const wchar_t* className,
    const ScreensaverWindowPlacement& placement) {
    return CreateWindowExW(placement.exStyle, className, L"Backrooms Maze", placement.style,
        placement.x, placement.y, placement.width, placement.height,
        placement.parent, nullptr, hInstance, nullptr);
}
