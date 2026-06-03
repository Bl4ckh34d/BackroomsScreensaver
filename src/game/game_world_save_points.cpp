#include "../platform/platform_headers.h"
#include "../core/maze_types.h"
#include "../core/math_utils.h"
#include "../core/constants.h"
#include "../debug/effect_debug_constants.h"
#include "../config/settings.h"
#include "../audio/audio_engine.h"
#include "../audio/game_audio_events.h"
#include "../maze/maze.h"
#include "../gameplay/playable_progression_types.h"
#include "../monster/monster_state.h"

#include "player_controller.h"
#include "player_state.h"
#include "game_world.h"

GameWorldSavePointSpawnPlan GameWorld::BuildSavePointSpawnPlan() const {
    GameWorldSavePointSpawnPlan plan{};
    PlayableSavePointSpawnPlan playablePlan = playableRun.BuildSavePointSpawnPlan();
    plan.eligible = playablePlan.eligible;
    plan.mustSpawn = playablePlan.mustSpawn;
    return plan;
}

GameWorldSavePointCandidateResult GameWorld::PickSavePointCandidate(std::mt19937& rng) const {
    GameWorldSavePointCandidateResult result{};
    std::vector<Tile> candidates;
    candidates.reserve(static_cast<size_t>(maze.w * maze.h));
    for (int y = 1; y < maze.h - 1; ++y) {
        for (int x = 1; x < maze.w - 1; ++x) {
            Tile t{x, y};
            if (!maze.IsOpen(x, y) || t == maze.start || t == maze.exit) continue;
            if (std::abs(x - maze.start.x) + std::abs(y - maze.start.y) < 4) continue;
            if (std::abs(x - maze.exit.x) + std::abs(y - maze.exit.y) < 3) continue;
            candidates.push_back(t);
        }
    }
    if (candidates.empty()) return result;

    std::shuffle(candidates.begin(), candidates.end(), rng);
    result.found = true;
    result.tile = candidates.front();
    return result;
}

bool GameWorld::TryGenerateSavePoint(bool enabled, float spawnChance, std::mt19937& rng) {
    ClearSavePoint();
    if (!enabled) return false;

    GameWorldSavePointSpawnPlan spawnPlan = BuildSavePointSpawnPlan();
    if (!spawnPlan.eligible) return false;
    if (!spawnPlan.mustSpawn) {
        bool picked = spawnChance > 0.0f &&
            (static_cast<float>(rng() & 0xffffu) / 65535.0f) < spawnChance;
        if (!picked) return false;
    }

    GameWorldSavePointCandidateResult candidate = PickSavePointCandidate(rng);
    if (!candidate.found) return false;

    XMFLOAT3 center = maze.WorldCenter(candidate.tile, 0.0f);
    std::uniform_real_distribution<float> yawDist(0.0f, kPi * 2.0f);
    ActivateSavePoint({center.x, 0.0f, center.z}, yawDist(rng));
    return true;
}

void GameWorld::ActivateSavePoint(const XMFLOAT3& pos, float yaw) {
    savePoint.pos = pos;
    savePoint.yaw = yaw;
    savePoint.active = true;
    playableRun.saveItemsSpawned = std::min(playableRun.saveItemTarget, playableRun.saveItemsSpawned + 1);
}

bool GameWorld::CanInteractWithSavePoint() const {
    if (!savePoint.active || exitTransitionActive || deathActive) return false;
    float toX = savePoint.pos.x - player.position.x;
    float toY = 0.82f - player.position.y;
    float toZ = savePoint.pos.z - player.position.z;
    float dist = std::sqrt(std::max(0.0f, toX * toX + toY * toY + toZ * toZ));
    if (dist > 1.85f) return false;
    float invDist = dist > 0.0001f ? 1.0f / dist : 0.0f;
    float pitchCos = std::cos(player.pitch);
    float viewX = std::sin(player.yaw) * pitchCos;
    float viewY = std::sin(player.pitch);
    float viewZ = std::cos(player.yaw) * pitchCos;
    float viewDot = toX * invDist * viewX + toY * invDist * viewY + toZ * invDist * viewZ;
    if (viewDot < 0.34f) return false;
    Tile cameraTile = maze.TileFromWorld(player.position.x, player.position.z);
    Tile saveTile = maze.TileFromWorld(savePoint.pos.x, savePoint.pos.z);
    return maze.LineClear(cameraTile, saveTile);
}
