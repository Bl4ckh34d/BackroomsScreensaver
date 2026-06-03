// Screensaver run-mode, debug-global, and launch-window setup helpers.
// Included from screensaver_app.inl before RunScreensaver.

struct ScreensaverRunConfig {
    bool monsterPreviewMode = false;
    bool bloodDebugMode = false;
    bool diagnosticWindowMode = false;
    MonsterPreviewView monsterPreviewView = MonsterPreviewView::Orbit;
};

ScreensaverRunConfig BuildScreensaverRunConfig(RunMode mode) {
    ScreensaverRunConfig config{};
    config.monsterPreviewMode =
        mode == RunMode::MonsterPreview ||
        mode == RunMode::MonsterPreviewFront ||
        mode == RunMode::MonsterPreviewSide ||
        mode == RunMode::MonsterPreviewLeftSide ||
        mode == RunMode::MonsterPreviewTop;
    config.bloodDebugMode = mode == RunMode::BloodDebug;
    config.diagnosticWindowMode = config.monsterPreviewMode || config.bloodDebugMode;
    if (mode == RunMode::MonsterPreviewFront) config.monsterPreviewView = MonsterPreviewView::Front;
    else if (mode == RunMode::MonsterPreviewSide) config.monsterPreviewView = MonsterPreviewView::Side;
    else if (mode == RunMode::MonsterPreviewLeftSide) config.monsterPreviewView = MonsterPreviewView::LeftSide;
    else if (mode == RunMode::MonsterPreviewTop) config.monsterPreviewView = MonsterPreviewView::Top;
    return config;
}

void ApplyScreensaverDebugGlobals(const ScreensaverRunConfig& config) {
    gEffectDebugViewer = config.bloodDebugMode;
    if (config.bloodDebugMode) {
        gDebugSliceEffect = gStartupDebugSliceEffect;
        gDebugSliceTiles = std::clamp(gDebugSliceTiles, 1, 5);
    }
    gBloodDebugEveryWall = gEffectDebugViewer &&
        (gDebugSliceEffect == DebugSliceEffect::Blood || DebugSliceEffectIsWater(gDebugSliceEffect));
}

ATOM RegisterScreensaverWindowClass(HINSTANCE hInstance, const wchar_t* className) {
    WNDCLASSW wc{};
    wc.lpfnWndProc = ScreensaverWndProc;
    wc.hInstance = hInstance;
    wc.lpszClassName = className;
    wc.hbrBackground = reinterpret_cast<HBRUSH>(GetStockObject(BLACK_BRUSH));
    return RegisterClassW(&wc);
}

struct ScreensaverWindowPlacement {
    DWORD style = WS_POPUP;
    DWORD exStyle = WS_EX_TOPMOST;
    HWND parent = nullptr;
    int x = 0;
    int y = 0;
    int width = 1;
    int height = 1;
};

ScreensaverWindowPlacement BuildScreensaverWindowPlacement(
    RunMode mode,
    HWND previewParent,
    bool diagnosticWindowMode,
    const std::vector<PlaybackMonitorRect>& playbackMonitors) {
    ScreensaverWindowPlacement placement{};
    placement.x = GetSystemMetrics(SM_XVIRTUALSCREEN);
    placement.y = GetSystemMetrics(SM_YVIRTUALSCREEN);
    placement.width = GetSystemMetrics(SM_CXVIRTUALSCREEN);
    placement.height = GetSystemMetrics(SM_CYVIRTUALSCREEN);
    if (!playbackMonitors.empty()) {
        const RECT& rc = playbackMonitors.front().rc;
        placement.x = rc.left;
        placement.y = rc.top;
        placement.width = std::max(1L, rc.right - rc.left);
        placement.height = std::max(1L, rc.bottom - rc.top);
    }

    if (diagnosticWindowMode) {
        placement.style = WS_OVERLAPPEDWINDOW | WS_VISIBLE | WS_CLIPCHILDREN | WS_CLIPSIBLINGS;
        placement.exStyle = 0;
        placement.parent = nullptr;
        placement.x = 80;
        placement.y = 80;
        placement.width = 1100;
        placement.height = 820;
    } else if (mode == RunMode::Preview && previewParent) {
        RECT rc{};
        GetClientRect(previewParent, &rc);
        placement.x = 0;
        placement.y = 0;
        placement.width = std::max(1L, rc.right - rc.left);
        placement.height = std::max(1L, rc.bottom - rc.top);
        placement.style = WS_CHILD | WS_VISIBLE;
        placement.exStyle = 0;
        placement.parent = previewParent;
    }
    return placement;
}

HWND CreateScreensaverHostWindow(
    HINSTANCE hInstance,
    const wchar_t* className,
    const ScreensaverWindowPlacement& placement) {
    return CreateWindowExW(placement.exStyle, className, L"Backrooms Maze", placement.style,
        placement.x, placement.y, placement.width, placement.height,
        placement.parent, nullptr, hInstance, nullptr);
}
