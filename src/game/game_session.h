#pragma once

#include "runtime_mode.h"

enum class GameSessionInputSource {
    Autopilot,
    Manual
};

enum class GameSessionMapOverlayStyle {
    PlayerExploration,
    AiDebug
};

struct GameSessionSpec {
    RendererRuntimeMode runtimeMode = RendererRuntimeMode::ScreensaverAutopilot;
    GameSessionInputSource inputSource = GameSessionInputSource::Autopilot;
    GameSessionMapOverlayStyle mapOverlayStyle = GameSessionMapOverlayStyle::AiDebug;
    bool progressionEnabled = false;
    bool customGame = false;
    bool showStartNotification = false;
    CustomGameSpec customSpec{};

    static GameSessionSpec ScreensaverAutopilot();
    static GameSessionSpec PlayableRun();
    static GameSessionSpec CustomPlayableRun(const CustomGameSpec& custom);
};

struct GameSessionRuntimeState {
    RendererRuntimeMode mode = RendererRuntimeMode::ScreensaverAutopilot;
    GameSessionInputSource inputSource = GameSessionInputSource::Autopilot;
    GameSessionMapOverlayStyle mapOverlayStyle = GameSessionMapOverlayStyle::AiDebug;
    GameInputSnapshot input{};
    Settings gameplaySettings{};
    uint32_t runtimeSeed = 1;
    std::mt19937 rng{0x51515151u};

    void ConfigureFromSpec(const GameSessionSpec& spec);
    void ConfigurePlayableManual();
    void ConfigureScreensaverAutopilot();
    bool IsPlayableSimulation() const;
    bool IsPlayableGame() const;
    bool IsMainMenu() const;
    bool UsesManualInput() const;
    bool UsesAutopilotInput() const;
    void ClearInput();
    void SeedRuntime(uint32_t seed);
};
