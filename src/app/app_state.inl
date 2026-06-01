// App-wide run modes, game state, window handles, and debug toolbar helpers.
// Included from main.cpp after Renderer is defined.

enum class RunMode {
    Fullscreen,
    Preview,
    Configure,
    SelfTest,
    MonsterPreview,
    MonsterPreviewFront,
    MonsterPreviewSide,
    MonsterPreviewLeftSide,
    MonsterPreviewTop,
    BloodDebug,
    GenerateIni
};

enum class GameState {
    MainMenu,
    PlayGame,
    DebugScene,
    Settings,
    Exit
};

struct App {
    Renderer renderer;
    Settings gameInputSettings;
    bool preview = false;
    bool gameShell = false;
    bool rendererInitialized = false;
    bool gameRunStarted = false;
    bool gameDebugActive = false;
    bool gameWindowActive = true;
    HINSTANCE gameInstance = nullptr;
    GameState gameState = GameState::MainMenu;
    bool gameMouseCaptured = false;
    bool gameRecenteringMouse = false;
    POINT gameMouseCenter{};
    float gameMouseDeltaX = 0.0f;
    float gameMouseDeltaY = 0.0f;
    HWND gameTitle = nullptr;
    HWND gameSinglePlayer = nullptr;
    HWND gameSettings = nullptr;
    HWND gameDebug = nullptr;
    HWND gameBack = nullptr;
    HWND gameExit = nullptr;
    HWND customTitle = nullptr;
    HWND customLayerLabel = nullptr;
    HWND customLayer = nullptr;
    HWND customScaresLabel = nullptr;
    HWND customScareBrokenLamp = nullptr;
    HWND customScareAirVent = nullptr;
    HWND customScareWater = nullptr;
    HWND customScareBlood = nullptr;
    HWND customScareFlesh = nullptr;
    HWND customBossesLabel = nullptr;
    HWND customBossOmukade = nullptr;
    HWND customSizeLabel = nullptr;
    HWND customSizeXLabel = nullptr;
    HWND customSizeX = nullptr;
    HWND customSizeYLabel = nullptr;
    HWND customSizeY = nullptr;
    HWND customPages = nullptr;
    HWND customStart = nullptr;
    HWND customBack = nullptr;
    HWND gameConfig = nullptr;
    GameState gameSettingsReturnState = GameState::MainMenu;
    bool gameSettingsKeyCaptureActive = false;
    bool gameSettingsEscapeConsumed = false;
    int gameMenuHoverId = 0;
    POINT gameMenuMouse{};
    bool gameMenuHasMouse = false;
    bool gameMenuTrackingMouse = false;
    ULONGLONG gameMenuBloodStart = 0;
    ULONGLONG gameMenuLampBurstStart = 0;
    ULONGLONG gameMenuFadeStart = 0;
    int gameMenuPendingCommand = 0;
    bool gameMenuStartCinematic = false;
    bool gameSkipNextLoadingOverlay = false;
    bool gameLoadSavedRunPending = false;
    bool gameForceNewRunPending = false;
    bool gameMenuFadeOut = false;
    bool gameMenuFadeIn = true;
    bool gameCustomMenuOpen = false;
    bool gameCustomGamePending = false;
    ULONGLONG gameCustomMenuOpenStart = 0;
    int gameCustomSelectedScare = -1;
    CustomGameSpec gameCustomSpec{};
    bool firstMouse = true;
    POINT initialMouse{};
    HWND hwnd = nullptr;
    HWND debugPrevEffect = nullptr;
    HWND debugNextEffect = nullptr;
    HWND debugSize = nullptr;
    HWND debugReset = nullptr;
    HWND debugPrevProp = nullptr;
    HWND debugNextProp = nullptr;
    HWND debugSettings = nullptr;
    HWND loadingOverlay = nullptr;
    bool loadingWarmupPending = false;
    ULONGLONG loadingWarmupStart = 0;
    int loadingWarmupAttempts = 0;
    bool quitting = false;

    struct CloneOutput {
        HWND hwnd = nullptr;
        HWND loadingOverlay = nullptr;
        Renderer renderer;
        bool loadingWarmupPending = false;
        ULONGLONG loadingWarmupStart = 0;
        int loadingWarmupAttempts = 0;
    };
    std::vector<std::unique_ptr<CloneOutput>> clones;
};

App* gApp = nullptr;

App::CloneOutput* CloneForWindow(HWND hwnd) {
    if (!gApp) return nullptr;
    for (auto& clone : gApp->clones) {
        if (clone && clone->hwnd == hwnd) return clone.get();
    }
    return nullptr;
}

constexpr int kDebugPrevEffectId = 5101;
constexpr int kDebugNextEffectId = 5102;
constexpr int kDebugSizeId = 5103;
constexpr int kDebugResetId = 5104;
constexpr int kDebugPrevPropId = 5105;
constexpr int kDebugNextPropId = 5106;
constexpr int kDebugSettingsId = 5107;
constexpr int kGameSinglePlayerId = 5201;
constexpr int kGameSettingsId = 5202;
constexpr int kGameDebugId = 5203;
constexpr int kGameBackId = 5204;
constexpr int kGameExitId = 5205;
constexpr int kGameResumeCurrentRunId = 5206;
constexpr int kGameResumeSavedRunId = 5207;
constexpr int kGameCustomGameId = 5208;
constexpr int kGameCustomLayerId = 5301;
constexpr int kGameCustomScareBrokenLampId = 5302;
constexpr int kGameCustomScareAirVentId = 5303;
constexpr int kGameCustomScareWaterId = 5304;
constexpr int kGameCustomScareBloodId = 5305;
constexpr int kGameCustomScareFleshId = 5306;
constexpr int kGameCustomBossOmukadeId = 5307;
constexpr int kGameCustomSizeXId = 5308;
constexpr int kGameCustomSizeYId = 5309;
constexpr int kGameCustomPagesId = 5310;
constexpr int kGameCustomStartId = 5311;
constexpr int kGameCustomBackId = 5312;
constexpr UINT kGameConfigClosedMessage = WM_APP + 31;

DebugSliceEffect StepDebugSliceEffect(DebugSliceEffect effect, int delta) {
    int count = static_cast<int>(DebugSliceEffect::Count);
    int index = (static_cast<int>(effect) + delta) % count;
    if (index < 0) index += count;
    return static_cast<DebugSliceEffect>(index);
}

void UpdateDebugSliceControls(HWND hwnd) {
    if (!gApp || !gEffectDebugViewer) return;
    wchar_t title[160]{};
    if (gDebugSliceEffect == DebugSliceEffect::Props) {
        swprintf_s(title, L"Backrooms Maze Effect Slice Debug - Props - %s (%d/%d)",
            DebugPropName(gDebugPropIndex), WrapDebugPropIndex(gDebugPropIndex) + 1, kDebugPropCount);
    } else {
        swprintf_s(title, L"Backrooms Maze Effect Slice Debug - %s - %dx%d",
            DebugSliceEffectName(gDebugSliceEffect), gDebugSliceTiles, gDebugSliceTiles);
    }
    SetWindowTextW(hwnd, title);
    if (gApp->debugSize) {
        wchar_t sizeText[48]{};
        swprintf_s(sizeText, L"Grid: %dx%d", gDebugSliceTiles, gDebugSliceTiles);
        SetWindowTextW(gApp->debugSize, sizeText);
    }
    if (gApp->debugPrevProp) EnableWindow(gApp->debugPrevProp, TRUE);
    if (gApp->debugNextProp) EnableWindow(gApp->debugNextProp, TRUE);
}

void RedrawDebugSliceControls() {
    if (!gApp || !gEffectDebugViewer) return;
    HWND controls[] = {
        gApp->debugPrevEffect,
        gApp->debugNextEffect,
        gApp->debugSize,
        gApp->debugReset,
        gApp->debugPrevProp,
        gApp->debugNextProp,
        gApp->debugSettings
    };
    for (HWND control : controls) {
        if (!control) continue;
        SetWindowPos(control, HWND_TOP, 0, 0, 0, 0,
            SWP_NOMOVE | SWP_NOSIZE | SWP_NOACTIVATE);
        RedrawWindow(control, nullptr, nullptr,
            RDW_INVALIDATE | RDW_UPDATENOW | RDW_ERASE | RDW_ALLCHILDREN);
    }
}
