// BackroomsMazeGame.exe window creation and main loop.
// Included from main.cpp after WndProc, config, and game shell helpers are available.

#include "game_app_window.inl"
#include "game_app_controls.inl"
#include "game_app_autostart.inl"
#include "game_app_loop.inl"

int RunGame(HINSTANCE hInstance) {
    const wchar_t* cls = L"BackroomsMazeGameWindow";
    RegisterGameWindowClass(hInstance, cls);

    App app;
    app.preview = true;
    app.gameShell = true;
    app.gameInstance = hInstance;
    gApp = &app;

    Settings launchSettings = LoadSettings();
    app.gameInputSettings = launchSettings;
    HWND hwnd = CreateGameHostWindow(hInstance, cls, launchSettings);
    if (!hwnd) {
        gApp = nullptr;
        return 1;
    }
    app.hwnd = hwnd;

    CreateGameHostControls(app, hwnd, hInstance);
    InitializeGameHostUi(app, hwnd);
    ApplyGameAutostart(app, hwnd);
    return RunGameMessageLoop(app, hwnd);
}
