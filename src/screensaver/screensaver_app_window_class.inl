ATOM RegisterScreensaverWindowClass(HINSTANCE hInstance, const wchar_t* className) {
    WNDCLASSW wc{};
    wc.lpfnWndProc = ScreensaverWndProc;
    wc.hInstance = hInstance;
    wc.lpszClassName = className;
    wc.hbrBackground = reinterpret_cast<HBRUSH>(GetStockObject(BLACK_BRUSH));
    return RegisterClassW(&wc);
}
