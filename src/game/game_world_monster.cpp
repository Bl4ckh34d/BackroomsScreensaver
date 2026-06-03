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

void GameWorld::ClearMonsterChasePressure() {
    monster.ClearChasePressure();
}

void GameWorld::ResetMonsterForSession(float initialRoamTimer) {
    monster.ResetSessionState(ExitWorldCenter(0.0f), 0.0f, initialRoamTimer);
}

void GameWorld::SetMonsterPreviewPose() {
    monster.position = {0.0f, 0.0f, 0.0f};
    monster.yaw = kPi;
}

bool GameWorld::MonsterCuriosityActive() const {
    return monster.CuriosityActive();
}

float GameWorld::MonsterCuriosityAmount() const {
    if (!monster.CuriosityActive()) return 0.0f;
    float t = 1.0f - Clamp01(monster.curiosityTimer / monster.curiosityDuration);
    return SmoothStep(0.0f, 0.24f, t) * (1.0f - SmoothStep(0.82f, 1.0f, t));
}

bool GameWorld::MonsterRecognitionFreezeActive() const {
    return monster.RecognitionFreezeActive();
}

bool GameWorld::MonsterChaseCommitted() const {
    return monster.ChaseCommitted();
}

bool GameWorld::MonsterChasePanicActive() const {
    return monster.ChasePanicActive();
}

bool GameWorld::MonsterRecognitionExpired() const {
    return monster.recognitionTimer <= 0.0f;
}

bool GameWorld::MonsterRecognitionPending() const {
    return monster.recognitionActive && !monster.recognizedForChase;
}

bool GameWorld::MonsterRecognizedForChase() const {
    return monster.recognizedForChase;
}

float GameWorld::MonsterChaseMemoryTimer() const {
    return monster.chaseMemoryTimer;
}

float GameWorld::MonsterChasePanic() const {
    return monster.chasePanic;
}

void GameWorld::BeginMonsterRecognition(float duration) {
    monster.BeginRecognition(duration);
}

void GameWorld::CancelMonsterRecognition() {
    monster.CancelRecognition();
}

void GameWorld::AdvanceMonsterRecognition(float dt) {
    monster.AdvanceRecognition(dt);
}

void GameWorld::BeginMonsterCuriosity(float duration) {
    monster.BeginCuriosity(duration);
}

void GameWorld::AdvanceMonsterCuriosity(float dt) {
    monster.AdvanceCuriosity(dt);
}

void GameWorld::ClearMonsterCuriosity() {
    monster.ClearCuriosity();
}

void GameWorld::CommitMonsterChaseRecognition() {
    monster.CommitChaseRecognition();
}

void GameWorld::ReleaseMonsterChaseRecognition() {
    monster.ReleaseChaseRecognition();
}

void GameWorld::HoldMonsterChaseMemory(float holdSeconds) {
    monster.HoldChaseMemory(holdSeconds);
}

void GameWorld::DecayMonsterChaseMemory(float dt) {
    monster.DecayChaseMemory(dt);
}

void GameWorld::UpdateMonsterChasePanic(float target, float response, float dt) {
    monster.chasePanic += (target - monster.chasePanic) * std::min(1.0f, dt * response);
    if (monster.chasePanic < 0.006f && target <= 0.0f) monster.chasePanic = 0.0f;
}
