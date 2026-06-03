// Developer/profile autostart handling for BackroomsMazeGame.exe.
// Included from game_app.inl after game shell helpers are available.

void ApplyGameAutostart(App& app, HWND hwnd) {
    bool autostartStress75 = MarkerOrEnvEnabled(
        L"BACKROOMS_AUTOSTART_STRESS75",
        L"BackroomsMaze.autostart_stress75.enable");
    bool autostartBenchmarkDemo = BenchmarkDemoEnabled();
    bool autostartGame = MarkerOrEnvEnabled(
        L"BACKROOMS_AUTOSTART_GAME",
        L"BackroomsMaze.autostart_game.enable");
    if (autostartBenchmarkDemo || autostartStress75) {
        CustomGameSpec stress{};
        stress.layer = 1;
        stress.mazeWidth = 75;
        stress.mazeHeight = 75;
        stress.roomCount = 80;
        stress.brokenLampScares = true;
        stress.airVentScares = true;
        stress.waterScares = true;
        stress.bloodWorldScares = true;
        stress.fleshWorldScares = true;
        stress.omukadeBoss = true;
        stress.eightPages = true;
        stress.mapDirtPercent = 100;
        stress.paperDensityPercent = 400;
        stress.propDensityPercent = 400;
        stress.lampOnPercent = 100;
        stress.lampFlickerPercent = 10;
        stress.lampSparkPercent = autostartBenchmarkDemo ? 35 : 15;
        stress.fogStartMeters = 0;
        stress.fogEndMeters = autostartBenchmarkDemo ? 36 : 28;
        stress.fogDarknessPercent = 100;
        stress.jumpscareChancePercent = 100;
        stress.jumpscareStartMinSeconds = 0;
        stress.jumpscareStartMaxSeconds = 0;
        stress.scareChancePercent.fill(100);
        stress.scareStartMinSeconds.fill(0);
        stress.scareStartMaxSeconds.fill(0);
        app.gameCustomSpec = stress;
        app.gameSkipNextLoadingOverlay = true;
        app.gameForceNewRunPending = true;
        app.gameCustomGamePending = true;
        ExecuteGameMenuCommand(hwnd, kGameCustomStartId);
    } else if (autostartGame) {
        app.gameSkipNextLoadingOverlay = true;
        ExecuteGameMenuCommand(hwnd, kGameSinglePlayerId);
    } else {
        app.gameMenuFadeIn = true;
        app.gameMenuFadeOut = false;
        app.gameMenuFadeStart = GetTickCount64();
        InvalidateRect(hwnd, nullptr, FALSE);
    }
}
