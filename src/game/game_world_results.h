#pragma once

#include "player_controller.h"
#include "game_world_types.h"

struct GameWorldCollectiblePickupResult {
    bool collected = false;
    int pageIndex = -1;
    int displayCollected = 0;
};

struct GameWorldCollectibleAimResult {
    bool found = false;
    size_t pageSlot = 0;
};

struct GameWorldSavePointSpawnPlan {
    bool eligible = false;
    bool mustSpawn = false;
};

struct GameWorldSavePointCandidateResult {
    bool found = false;
    Tile tile{};
};
