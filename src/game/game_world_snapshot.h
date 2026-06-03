#pragma once

#include "player_controller.h"
#include "game_world_types.h"

struct GameWorldSnapshotState {
    Maze maze;
    PlayerSnapshotState playerState;
    MonsterSnapshotState monsterState;
    PlayableRunState playableRun;
    std::array<CollectiblePage, kCollectiblePageMaterialCount> collectiblePages{};
    int collectiblePagesCollected = 0;
    SavePoint savePoint;
    std::vector<PlayerAudibleSoundPulse> playerSoundPulses;
    bool progressionEnabled = false;
    bool exitTransitionActive = false;
    float exitTransitionTimer = 0.0f;
    bool deathActive = false;
    float deathTimer = 0.0f;
};

struct GameWorldRenderSnapshot {
    const Maze* maze = nullptr;
    XMFLOAT3 playerPosition{};
    float playerYaw = 0.0f;
    float playerBodyYaw = 0.0f;
    float playerPitch = 0.0f;
    float playerHealth = 100.0f;
    float playerStamina = 100.0f;
    bool playerFlashlightEnabled = false;
    float playerAudibleNoiseRadiusMeters = 0.0f;
    float playerStepPhase = 0.0f;
    float playerRunIntensity = 0.0f;
    float playerRunEffort = 0.0f;
    float playerTunnelLeanSide = 0.0f;
    float playerTunnelLeanAmount = 0.0f;
    XMFLOAT3 monsterPosition{};
    float monsterYaw = 0.0f;
    int monsterKillCount = 0;
    float monsterChasePanic = 0.0f;
    float monsterRoamBurstTimer = 0.0f;
    bool monsterChasingVisible = false;
    bool monsterHeardPlayerNow = false;
    bool monsterCanSeePlayerNow = false;
    bool monsterHasSoundTarget = false;
    bool monsterHasLastKnownTarget = false;
    Tile monsterGoal{-1000, -1000};
    Tile monsterSoundTile{-1000, -1000};
    Tile monsterLastKnownTile{-1000, -1000};
    Tile monsterRoamTile{-1000, -1000};
    const std::vector<Tile>* monsterPath = nullptr;
    size_t monsterPathIndex = 0;
    const std::vector<PlayerAudibleSoundPulse>* playerSoundPulses = nullptr;
    int collectiblePagesCollected = 0;
    const std::array<CollectiblePage, kCollectiblePageMaterialCount>* collectiblePages = nullptr;
    SavePoint savePoint{};
    bool progressionEnabled = false;
    bool exitTransitionActive = false;
    float exitTransitionTimer = 0.0f;
    bool deathActive = false;
    float deathTimer = 0.0f;
};
