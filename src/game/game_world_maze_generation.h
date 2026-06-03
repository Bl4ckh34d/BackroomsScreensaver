#pragma once

#include "player_controller.h"
#include "game_world_types.h"

enum class GameWorldMazeGenerationKind {
    Standard,
    MainMenu,
    DebugSlice,
    BloodDebugCorridor,
    BenchmarkDemo
};

struct GameWorldMazeGenerationRequest {
    MazeLayoutSpec layout{};
    MazeGenerationSpec generation{};
    GameWorldMazeGenerationKind kind = GameWorldMazeGenerationKind::Standard;
    uint32_t runtimeSeed = 0;
    int debugSliceTiles = 1;
    bool applyLayout = true;
    bool updateExit = true;
};
