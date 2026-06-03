#include "../platform/platform_headers.h"
#include "../core/maze_types.h"

#include "monster_state.h"

void MonsterState::ResetKillCount() {
    killCount = 0;
}

bool MonsterState::CuriosityActive() const {
    return curiosityTimer > 0.0f && curiosityDuration > 0.001f;
}

bool MonsterState::RecognitionFreezeActive() const {
    return recognitionActive && recognitionTimer > 0.0f;
}

bool MonsterState::ChaseCommitted() const {
    return recognizedForChase || chaseMemoryTimer > 0.0f || chasePanic > 0.08f;
}

bool MonsterState::ChasePanicActive() const {
    return chaseMemoryTimer > 0.0f || chasePanic > 0.08f;
}

bool MonsterState::AlertAudioActive() const {
    return chasingVisible || recognizedForChase || hasLastKnown || hasSound || ChasePanicActive();
}

void MonsterState::BeginRecognition(float duration) {
    recognitionDuration = duration;
    recognitionTimer = duration;
    recognitionActive = true;
    recognizedForChase = false;
}

void MonsterState::CancelRecognition() {
    recognitionActive = false;
    recognitionTimer = 0.0f;
}

void MonsterState::AdvanceRecognition(float dt) {
    recognitionTimer = std::max(0.0f, recognitionTimer - dt);
}

void MonsterState::BeginCuriosity(float duration) {
    curiosityDuration = duration;
    curiosityTimer = duration;
    recognitionActive = false;
}

void MonsterState::AdvanceCuriosity(float dt) {
    curiosityTimer = std::max(0.0f, curiosityTimer - dt);
}

void MonsterState::ClearCuriosity() {
    curiosityTimer = 0.0f;
    curiosityDuration = 0.0f;
}

void MonsterState::CommitChaseRecognition() {
    recognitionActive = false;
    recognitionTimer = 0.0f;
    recognizedForChase = true;
}

void MonsterState::ReleaseChaseRecognition() {
    recognizedForChase = false;
}

void MonsterState::HoldChaseMemory(float holdSeconds) {
    chaseMemoryTimer = std::max(chaseMemoryTimer, holdSeconds);
}

void MonsterState::DecayChaseMemory(float dt) {
    chaseMemoryTimer = std::max(0.0f, chaseMemoryTimer - dt);
}

void MonsterState::ClearChasePressure() {
    chaseMemoryTimer = 0.0f;
    chasePanic = 0.0f;
}

void MonsterState::ClearChaseCommitment() {
    CancelRecognition();
    recognizedForChase = false;
    ClearCuriosity();
    ClearChasePressure();
}

void MonsterState::ClearPursuitState() {
    path.clear();
    pathIndex = 0;
    repathTimer = 0.0f;
    goal = {-1000, -1000};
    soundTile = {-1000, -1000};
    lastKnownTile = {-1000, -1000};
    roamTile = {-1000, -1000};
    hasSound = false;
    hasLastKnown = false;
    chasingVisible = false;
    heardPlayerNow = false;
    canSeePlayerNow = false;
    recognitionActive = false;
    recognitionTimer = 0.0f;
    recognitionDuration = 0.0f;
    ClearChaseCommitment();
}

void MonsterState::ResetSessionState(const XMFLOAT3& startPosition, float startYaw, float initialRoamTimer) {
    position = startPosition;
    yaw = startYaw;
    ClearPursuitState();
    searchTimer = 0.0f;
    roamTimer = initialRoamTimer;
    roamPauseTimer = 0.0f;
    roamBurstTimer = 0.0f;
}

MonsterSnapshotState MonsterState::CaptureSnapshotState() const {
    MonsterSnapshotState snapshot{};
    snapshot.position = position;
    snapshot.yaw = yaw;
    snapshot.path = path;
    snapshot.pathIndex = pathIndex;
    snapshot.goal = goal;
    snapshot.soundTile = soundTile;
    snapshot.lastKnownTile = lastKnownTile;
    snapshot.roamTile = roamTile;
    snapshot.hasSound = hasSound;
    snapshot.hasLastKnown = hasLastKnown;
    snapshot.repathTimer = repathTimer;
    snapshot.chasingVisible = chasingVisible;
    snapshot.heardPlayerNow = heardPlayerNow;
    snapshot.canSeePlayerNow = canSeePlayerNow;
    snapshot.killCount = killCount;
    snapshot.searchTimer = searchTimer;
    snapshot.roamTimer = roamTimer;
    snapshot.roamPauseTimer = roamPauseTimer;
    snapshot.roamBurstTimer = roamBurstTimer;
    snapshot.recognitionActive = recognitionActive;
    snapshot.recognizedForChase = recognizedForChase;
    snapshot.recognitionTimer = recognitionTimer;
    snapshot.recognitionDuration = recognitionDuration;
    snapshot.curiosityTimer = curiosityTimer;
    snapshot.curiosityDuration = curiosityDuration;
    snapshot.chaseMemoryTimer = chaseMemoryTimer;
    snapshot.chasePanic = chasePanic;
    return snapshot;
}

void MonsterState::RestoreSnapshotState(MonsterSnapshotState snapshot) {
    position = snapshot.position;
    yaw = snapshot.yaw;
    path = std::move(snapshot.path);
    pathIndex = snapshot.pathIndex;
    goal = snapshot.goal;
    soundTile = snapshot.soundTile;
    lastKnownTile = snapshot.lastKnownTile;
    roamTile = snapshot.roamTile;
    hasSound = snapshot.hasSound;
    hasLastKnown = snapshot.hasLastKnown;
    repathTimer = snapshot.repathTimer;
    chasingVisible = snapshot.chasingVisible;
    heardPlayerNow = snapshot.heardPlayerNow;
    canSeePlayerNow = snapshot.canSeePlayerNow;
    killCount = snapshot.killCount;
    searchTimer = snapshot.searchTimer;
    roamTimer = snapshot.roamTimer;
    roamPauseTimer = snapshot.roamPauseTimer;
    roamBurstTimer = snapshot.roamBurstTimer;
    recognitionActive = snapshot.recognitionActive;
    recognizedForChase = snapshot.recognizedForChase;
    recognitionTimer = snapshot.recognitionTimer;
    recognitionDuration = snapshot.recognitionDuration;
    curiosityTimer = snapshot.curiosityTimer;
    curiosityDuration = snapshot.curiosityDuration;
    chaseMemoryTimer = snapshot.chaseMemoryTimer;
    chasePanic = snapshot.chasePanic;
}
