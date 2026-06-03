// BackroomsMaze.scr fullscreen/preview/diagnostic host.
// Included from main.cpp after config, loading overlay, and shared window helpers.

#include "screensaver_app_modes.inl"
#include "screensaver_app_windows.inl"
#include "screensaver_app_renderer.inl"
#include "screensaver_app_loop.inl"
#include "screensaver_self_test.inl"

int RunScreensaver(HINSTANCE hInstance, RunMode mode, HWND previewParent) {
    ScreensaverRunConfig config = BuildScreensaverRunConfig(mode);
    ApplyScreensaverDebugGlobals(config);

    const wchar_t* cls = L"BackroomsMazeScreensaverWindow";
    RegisterScreensaverWindowClass(hInstance, cls);

    App app;
    app.preview = mode == RunMode::Preview || config.diagnosticWindowMode;
    gApp = &app;

    std::vector<PlaybackMonitorRect> playbackMonitors;
    if (mode == RunMode::Fullscreen) {
        playbackMonitors = EnumeratePlaybackMonitors();
    }

    ScreensaverWindowPlacement placement = BuildScreensaverWindowPlacement(
        mode, previewParent, config.diagnosticWindowMode, playbackMonitors);
    HWND hwnd = CreateScreensaverHostWindow(hInstance, cls, placement);
    if (!hwnd) return 1;
    app.hwnd = hwnd;

    CreateScreensaverCloneWindows(app, hInstance, cls, placement, playbackMonitors);
    CreateScreensaverDebugControls(app, hwnd, hInstance);
    ShowScreensaverWindowsAndCreateLoadingOverlays(app, hwnd, hInstance);
    if (!app.preview) ShowCursor(FALSE);

    ScreensaverRendererSettings rendererSettings = BuildScreensaverRendererSettings(mode);
    if (!InitializeScreensaverRenderers(app, hwnd, config, rendererSettings.rendererSettings)) {
        if (!app.preview) ShowCursor(TRUE);
        return 1;
    }

    int result = RunScreensaverMessageLoop(app, hwnd);
    if (!app.preview) ShowCursor(TRUE);
    gApp = nullptr;
    gEffectDebugViewer = false;
    gBloodDebugEveryWall = false;
    return result;
}
