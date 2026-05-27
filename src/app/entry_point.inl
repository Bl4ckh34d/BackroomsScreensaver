// Process entry point shared by BackroomsMaze.scr and BackroomsMazeGame.exe.
// Included after the anonymous namespace so each target only calls its runtime host.

int WINAPI wWinMain(HINSTANCE hInstance, HINSTANCE, PWSTR, int) {
    SetProcessDPIAware();
#if defined(BACKROOMS_GAME_EXE)
    return RunGame(hInstance);
#else
    int argc = 0;
    wchar_t** argv = CommandLineToArgvW(GetCommandLineW(), &argc);
    HWND previewParent = nullptr;
    RunMode mode = ParseMode(argc, argv, previewParent);
    if (argv) LocalFree(argv);

    if (mode == RunMode::Configure) {
        ShowConfig(nullptr);
        return 0;
    }
    if (mode == RunMode::SelfTest) {
        return RunSelfTest(hInstance);
    }
    if (mode == RunMode::GenerateIni) {
        WriteTextFile(PackagedSettingsPath(), DefaultConfigText());
        return 0;
    }

    return RunScreensaver(hInstance, mode, previewParent);
#endif
}
