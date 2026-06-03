#pragma once

enum class RendererRuntimeMode {
    ScreensaverAutopilot,
    MainMenu,
    PlayableGame,
    DebugViewer,
    Preview
};

inline bool IsPlayableSimulationMode(RendererRuntimeMode mode) {
    return mode == RendererRuntimeMode::PlayableGame ||
        mode == RendererRuntimeMode::ScreensaverAutopilot;
}
