#include "../platform/platform_headers.h"
#include "../core/maze_types.h"
#include "../core/math_utils.h"
#include "../core/constants.h"
#include "../debug/effect_debug_constants.h"
#include "../config/settings.h"
#include "../gameplay/playable_progression_types.h"

#include "game_input.h"
#include "game_session.h"

GameSessionSpec GameSessionSpec::ScreensaverAutopilot() {
    GameSessionSpec spec{};
    spec.runtimeMode = RendererRuntimeMode::ScreensaverAutopilot;
    spec.inputSource = GameSessionInputSource::Autopilot;
    spec.mapOverlayStyle = GameSessionMapOverlayStyle::AiDebug;
    spec.progressionEnabled = false;
    spec.showStartNotification = false;
    return spec;
}

GameSessionSpec GameSessionSpec::PlayableRun() {
    GameSessionSpec spec{};
    spec.runtimeMode = RendererRuntimeMode::PlayableGame;
    spec.inputSource = GameSessionInputSource::Manual;
    spec.mapOverlayStyle = GameSessionMapOverlayStyle::PlayerExploration;
    spec.progressionEnabled = true;
    spec.showStartNotification = true;
    return spec;
}

GameSessionSpec GameSessionSpec::CustomPlayableRun(const CustomGameSpec& custom) {
    GameSessionSpec spec = PlayableRun();
    spec.customGame = true;
    spec.customSpec = custom;
    return spec;
}

void GameSessionRuntimeState::ConfigureFromSpec(const GameSessionSpec& spec) {
    mode = spec.runtimeMode;
    inputSource = spec.inputSource;
    mapOverlayStyle = spec.mapOverlayStyle;
}

void GameSessionRuntimeState::ConfigurePlayableManual() {
    mode = RendererRuntimeMode::PlayableGame;
    inputSource = GameSessionInputSource::Manual;
    mapOverlayStyle = GameSessionMapOverlayStyle::PlayerExploration;
}

void GameSessionRuntimeState::ConfigureScreensaverAutopilot() {
    mode = RendererRuntimeMode::ScreensaverAutopilot;
    inputSource = GameSessionInputSource::Autopilot;
    mapOverlayStyle = GameSessionMapOverlayStyle::AiDebug;
}

bool GameSessionRuntimeState::IsPlayableSimulation() const {
    return IsPlayableSimulationMode(mode);
}

bool GameSessionRuntimeState::IsPlayableGame() const {
    return mode == RendererRuntimeMode::PlayableGame;
}

bool GameSessionRuntimeState::IsMainMenu() const {
    return mode == RendererRuntimeMode::MainMenu;
}

bool GameSessionRuntimeState::UsesManualInput() const {
    return inputSource == GameSessionInputSource::Manual;
}

bool GameSessionRuntimeState::UsesAutopilotInput() const {
    return inputSource == GameSessionInputSource::Autopilot;
}

void GameSessionRuntimeState::ClearInput() {
    input = {};
}

void GameSessionRuntimeState::SeedRuntime(uint32_t seed) {
    runtimeSeed = seed;
    rng.seed(seed ^ 0x9e3779b9u);
}
