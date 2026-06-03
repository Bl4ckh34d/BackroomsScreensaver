// Config dialog top-level and embedded host creation.

void ShowConfig(HWND owner, ConfigDialogMode mode) {
    EnsureSettingsFile();
    INITCOMMONCONTROLSEX commonControls{sizeof(commonControls), ICC_TAB_CLASSES | ICC_STANDARD_CLASSES | ICC_BAR_CLASSES};
    InitCommonControlsEx(&commonControls);

    HINSTANCE hInstance = GetModuleHandleW(nullptr);
    const wchar_t* cls = L"BackroomsMazeConfigWindow";
    const wchar_t* previewCls = L"BackroomsMazeConfigPreview";
    const wchar_t* scrollPaneCls = L"BackroomsMazeConfigScrollPane";

    WNDCLASSW previewWc{};
    previewWc.lpfnWndProc = ConfigPreviewWndProc;
    previewWc.hInstance = hInstance;
    previewWc.lpszClassName = previewCls;
    previewWc.hCursor = LoadCursorW(nullptr, IDC_ARROW);
    previewWc.hbrBackground = reinterpret_cast<HBRUSH>(GetStockObject(BLACK_BRUSH));
    RegisterClassW(&previewWc);

    WNDCLASSW scrollPaneWc{};
    scrollPaneWc.lpfnWndProc = ConfigScrollPaneWndProc;
    scrollPaneWc.hInstance = hInstance;
    scrollPaneWc.lpszClassName = scrollPaneCls;
    scrollPaneWc.hCursor = LoadCursorW(nullptr, IDC_ARROW);
    scrollPaneWc.hbrBackground = reinterpret_cast<HBRUSH>(COLOR_WINDOW + 1);
    RegisterClassW(&scrollPaneWc);

    WNDCLASSW wc{};
    wc.lpfnWndProc = ConfigWndProc;
    wc.hInstance = hInstance;
    wc.lpszClassName = cls;
    wc.hCursor = LoadCursorW(nullptr, IDC_ARROW);
    wc.hbrBackground = reinterpret_cast<HBRUSH>(COLOR_WINDOW + 1);
    RegisterClassW(&wc);

    const wchar_t* title = mode == ConfigDialogMode::Game
        ? L"Backrooms Maze Game Settings"
        : (mode == ConfigDialogMode::Debug ? L"Backrooms Maze Debug Settings" : L"Backrooms Maze Configuration");
    ConfigCreateParams params{mode, false};
    HWND hwnd = CreateWindowExW(WS_EX_DLGMODALFRAME, cls, title,
        WS_OVERLAPPED | WS_CAPTION | WS_SYSMENU | WS_MINIMIZEBOX | WS_VISIBLE, CW_USEDEFAULT, CW_USEDEFAULT, 1220, 735,
        owner, nullptr, hInstance, &params);
    if (!hwnd) return;
    MSG msg{};
    while (GetMessageW(&msg, nullptr, 0, 0) > 0) {
        ConfigState* state = reinterpret_cast<ConfigState*>(GetWindowLongPtrW(hwnd, GWLP_USERDATA));
        if (state && msg.message == WM_MOUSEWHEEL) {
            POINT screenPoint{GET_X_LPARAM(msg.lParam), GET_Y_LPARAM(msg.lParam)};
            if (ConfigPointOverPreview(state, screenPoint)) {
                ZoomConfigPreviewOrbit(state, msg.wParam);
                continue;
            }
        }
        TranslateMessage(&msg);
        DispatchMessageW(&msg);
    }
}

HWND CreateEmbeddedConfig(HWND parent, ConfigDialogMode mode) {
    EnsureSettingsFile();
    INITCOMMONCONTROLSEX commonControls{sizeof(commonControls), ICC_TAB_CLASSES | ICC_STANDARD_CLASSES | ICC_BAR_CLASSES};
    InitCommonControlsEx(&commonControls);

    HINSTANCE hInstance = GetModuleHandleW(nullptr);
    const wchar_t* cls = L"BackroomsMazeConfigWindow";
    const wchar_t* previewCls = L"BackroomsMazeConfigPreview";
    const wchar_t* scrollPaneCls = L"BackroomsMazeConfigScrollPane";

    WNDCLASSW previewWc{};
    previewWc.lpfnWndProc = ConfigPreviewWndProc;
    previewWc.hInstance = hInstance;
    previewWc.lpszClassName = previewCls;
    previewWc.hCursor = LoadCursorW(nullptr, IDC_ARROW);
    previewWc.hbrBackground = reinterpret_cast<HBRUSH>(GetStockObject(BLACK_BRUSH));
    RegisterClassW(&previewWc);

    WNDCLASSW scrollPaneWc{};
    scrollPaneWc.lpfnWndProc = ConfigScrollPaneWndProc;
    scrollPaneWc.hInstance = hInstance;
    scrollPaneWc.lpszClassName = scrollPaneCls;
    scrollPaneWc.hCursor = LoadCursorW(nullptr, IDC_ARROW);
    scrollPaneWc.hbrBackground = reinterpret_cast<HBRUSH>(COLOR_WINDOW + 1);
    RegisterClassW(&scrollPaneWc);

    WNDCLASSW wc{};
    wc.lpfnWndProc = ConfigWndProc;
    wc.hInstance = hInstance;
    wc.lpszClassName = cls;
    wc.hCursor = LoadCursorW(nullptr, IDC_ARROW);
    wc.hbrBackground = reinterpret_cast<HBRUSH>(COLOR_WINDOW + 1);
    RegisterClassW(&wc);

    ConfigCreateParams params{mode, true};
    return CreateWindowExW(0, cls, L"",
        WS_CHILD | WS_VISIBLE | WS_CLIPSIBLINGS | WS_CLIPCHILDREN,
        0, 0, 10, 10, parent, nullptr, hInstance, &params);
}
