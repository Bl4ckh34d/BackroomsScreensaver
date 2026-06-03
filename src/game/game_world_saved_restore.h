#pragma once

#include "player_controller.h"
#include "game_world_types.h"

struct GameWorldSavedMazeRestoreState {
    int w = 15;
    int h = 15;
    float tileW = 2.0f;
    float tileD = 2.0f;
    Tile start{1, 1};
    Tile exit{13, 13};
    std::wstring open;
    std::wstring wallFeatures;
};

struct GameWorldSavedRuntimeRestoreState {
    XMFLOAT3 playerPosition{};
    float playerYaw = 0.0f;
    float playerBodyYaw = 0.0f;
    float playerPitch = -0.055f;
    float playerHealth = 100.0f;
    float playerStamina = 100.0f;
    int collectedPages = 0;
    SavePoint savePoint;
    std::array<uint8_t, kCollectiblePageMaterialCount> pageCollected{};
};
