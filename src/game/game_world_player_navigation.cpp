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

void GameWorld::RaisePlayerRunPressure(float minRunIntensity, float minRunEffort, float minSmoothedMoveSpeed) {
    player.runIntensity = std::max(player.runIntensity, minRunIntensity);
    player.runEffort = std::max(player.runEffort, minRunEffort);
    player.smoothedMoveSpeed = std::max(player.smoothedMoveSpeed, minSmoothedMoveSpeed);
}

void GameWorld::SetPlayerSmoothedMoveSpeed(float speed) {
    player.smoothedMoveSpeed = speed;
}

Tile GameWorld::StartTile() const {
    return maze.start;
}

XMFLOAT3 GameWorld::StartWorldCenter(float y) const {
    return maze.WorldCenter(maze.start, y);
}

XMFLOAT3 GameWorld::ExitWorldCenter(float y) const {
    return maze.WorldCenter(maze.exit, y);
}

float GameWorld::MazeTileMinimum() const {
    return maze.TileMinimum();
}

bool GameWorld::DeathActive() const {
    return deathActive;
}

size_t GameWorld::TileIndex(Tile t) const {
    return static_cast<size_t>(t.y * maze.w + t.x);
}

uint16_t GameWorld::VisitCount(Tile t) const {
    if (!maze.IsOpen(t.x, t.y)) return 0;
    size_t index = TileIndex(t);
    if (index >= player.visitedTiles.size()) return 0;
    return player.visitedTiles[index];
}

bool GameWorld::HasVisitedMapTiles() const {
    return !player.visitedTiles.empty();
}

void GameWorld::MarkVisited(Tile t) {
    if (!maze.IsOpen(t.x, t.y)) return;
    size_t index = TileIndex(t);
    if (index >= player.visitedTiles.size()) return;
    if (player.visitedTiles[index] < 65535) ++player.visitedTiles[index];
}

bool GameWorld::PlayerTunnelCrouchLocked() const {
    return player.tunnelCrouchLocked;
}

float GameWorld::PlayerTunnelLeanSideTarget() const {
    return player.tunnelLeanSideTarget;
}

float GameWorld::PlayerTunnelPostureHoldTimer() const {
    return player.tunnelPostureHoldTimer;
}

void GameWorld::ResetPlayerTunnelPosture() {
    player.tunnelCrouchLocked = false;
    player.tunnelPostureHoldTimer = 0.0f;
    player.tunnelLeanTarget = 0.0f;
    player.tunnelLeanSideTarget = 1.0f;
}

void GameWorld::SetPlayerTunnelCrouchLocked(bool locked) {
    player.tunnelCrouchLocked = locked;
}

void GameWorld::HoldPlayerTunnelCrouch(float holdSeconds) {
    player.tunnelCrouchLocked = true;
    player.tunnelPostureHoldTimer = std::max(player.tunnelPostureHoldTimer, holdSeconds);
}

void GameWorld::SetPlayerTunnelLeanSideTarget(float side) {
    player.tunnelLeanSideTarget = side;
}

void GameWorld::SetPlayerTunnelLeanTarget(bool active) {
    player.tunnelLeanTarget = active ? 1.0f : 0.0f;
}

void GameWorld::AdvancePlayerTunnelPostureTimer(float dt) {
    player.tunnelPostureHoldTimer = std::max(0.0f, player.tunnelPostureHoldTimer - std::max(0.0f, dt));
}

void GameWorld::AdvancePlayerTunnelCameraLean(float dt) {
    float target = player.tunnelLeanTarget;
    float response = target > player.tunnelLeanAmount ? 5.6f : 2.35f;
    float alpha = 1.0f - std::exp(-std::max(0.0f, dt) * response);
    player.tunnelLeanAmount += (target - player.tunnelLeanAmount) * alpha;
    float sideAlpha = 1.0f - std::exp(-std::max(0.0f, dt) * 5.2f);
    player.tunnelLeanSide += (player.tunnelLeanSideTarget - player.tunnelLeanSide) * sideAlpha;
    if (player.tunnelLeanAmount < 0.001f && target <= 0.0f) {
        player.tunnelLeanAmount = 0.0f;
    }
}
