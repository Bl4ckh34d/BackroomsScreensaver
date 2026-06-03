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

void GameWorld::ResetSessionState() {
    collectiblePagesCollected = 0;
    savePoint = {};
    playerSoundPulses.clear();
    audioEvents.clear();
    exitTransitionActive = false;
    exitTransitionTimer = 0.0f;
    deathActive = false;
    deathTimer = 0.0f;
}

GameWorldRenderSnapshot GameWorld::BuildRenderSnapshot() const {
    GameWorldRenderSnapshot snapshot{};
    snapshot.maze = &maze;
    snapshot.playerPosition = player.position;
    snapshot.playerYaw = player.yaw;
    snapshot.playerBodyYaw = player.bodyYaw;
    snapshot.playerPitch = player.pitch;
    snapshot.playerHealth = player.health;
    snapshot.playerStamina = player.stamina;
    snapshot.playerFlashlightEnabled = player.flashlightEnabled;
    snapshot.playerAudibleNoiseRadiusMeters = player.audibleNoiseRadiusMeters;
    snapshot.playerStepPhase = player.stepPhase;
    snapshot.playerRunIntensity = player.runIntensity;
    snapshot.playerRunEffort = player.runEffort;
    snapshot.playerTunnelLeanSide = player.tunnelLeanSide;
    snapshot.playerTunnelLeanAmount = player.tunnelLeanAmount;
    snapshot.monsterPosition = monster.position;
    snapshot.monsterYaw = monster.yaw;
    snapshot.monsterKillCount = monster.killCount;
    snapshot.monsterChasePanic = monster.chasePanic;
    snapshot.monsterRoamBurstTimer = monster.roamBurstTimer;
    snapshot.monsterChasingVisible = monster.chasingVisible;
    snapshot.monsterHeardPlayerNow = monster.heardPlayerNow;
    snapshot.monsterCanSeePlayerNow = monster.canSeePlayerNow;
    snapshot.monsterHasSoundTarget = monster.hasSound;
    snapshot.monsterHasLastKnownTarget = monster.hasLastKnown;
    snapshot.monsterGoal = monster.goal;
    snapshot.monsterSoundTile = monster.soundTile;
    snapshot.monsterLastKnownTile = monster.lastKnownTile;
    snapshot.monsterRoamTile = monster.roamTile;
    snapshot.monsterPath = &monster.path;
    snapshot.monsterPathIndex = monster.pathIndex;
    snapshot.playerSoundPulses = &playerSoundPulses;
    snapshot.collectiblePagesCollected = collectiblePagesCollected;
    snapshot.collectiblePages = &collectiblePages;
    snapshot.savePoint = savePoint;
    snapshot.progressionEnabled = progressionEnabled;
    snapshot.exitTransitionActive = exitTransitionActive;
    snapshot.exitTransitionTimer = exitTransitionTimer;
    snapshot.deathActive = deathActive;
    snapshot.deathTimer = deathTimer;
    return snapshot;
}

GameWorldSnapshotState GameWorld::CaptureSnapshotState() const {
    GameWorldSnapshotState snapshot{};
    snapshot.maze = maze;
    snapshot.playerState = player.CaptureSnapshotState();
    snapshot.monsterState = monster.CaptureSnapshotState();
    snapshot.playableRun = playableRun;
    snapshot.collectiblePages = collectiblePages;
    snapshot.collectiblePagesCollected = collectiblePagesCollected;
    snapshot.savePoint = savePoint;
    snapshot.playerSoundPulses = playerSoundPulses;
    snapshot.progressionEnabled = progressionEnabled;
    snapshot.exitTransitionActive = exitTransitionActive;
    snapshot.exitTransitionTimer = exitTransitionTimer;
    snapshot.deathActive = deathActive;
    snapshot.deathTimer = deathTimer;
    return snapshot;
}

void GameWorld::RestoreSnapshotGenerationState(const GameWorldSnapshotState& snapshot) {
    playableRun = snapshot.playableRun;
    maze = snapshot.maze;
    progressionEnabled = snapshot.progressionEnabled;
}

void GameWorld::RestoreSnapshotRuntimeState(GameWorldSnapshotState snapshot) {
    player.RestoreSnapshotState(std::move(snapshot.playerState));
    monster.RestoreSnapshotState(std::move(snapshot.monsterState));
    playerSoundPulses = std::move(snapshot.playerSoundPulses);
    audioEvents.clear();
    exitTransitionActive = snapshot.exitTransitionActive;
    exitTransitionTimer = snapshot.exitTransitionTimer;
    deathActive = snapshot.deathActive;
    deathTimer = snapshot.deathTimer;
    collectiblePages = snapshot.collectiblePages;
    collectiblePagesCollected = snapshot.collectiblePagesCollected;
    savePoint = snapshot.savePoint;
}

void GameWorld::ResetMonsterKillCount() {
    monster.ResetKillCount();
}

void GameWorld::ResetPlayableRun() {
    playableRun.Reset();
}

void GameWorld::ApplyMazeLayout(const MazeLayoutSpec& spec, bool updateExit) {
    maze.w = spec.width;
    maze.h = spec.height;
    maze.tileW = spec.tileW;
    maze.tileD = spec.tileD;
    if (updateExit) maze.exit = {maze.w - 2, maze.h - 2};
}

void GameWorld::GenerateMaze(const GameWorldMazeGenerationRequest& request) {
    maze.rng.seed(request.runtimeSeed);
    if (request.applyLayout) {
        ApplyMazeLayout(request.layout, request.updateExit);
    }

    switch (request.kind) {
    case GameWorldMazeGenerationKind::MainMenu:
        maze.GenerateMenuRoom();
        break;
    case GameWorldMazeGenerationKind::DebugSlice:
        maze.GenerateDebugSlice(request.debugSliceTiles);
        break;
    case GameWorldMazeGenerationKind::BloodDebugCorridor:
        maze.GenerateBloodDebugCorridor();
        break;
    case GameWorldMazeGenerationKind::BenchmarkDemo:
        maze.GenerateBenchmarkDemo();
        break;
    case GameWorldMazeGenerationKind::Standard:
    default:
        maze.Generate(request.generation);
        break;
    }
}

const Maze& GameWorld::MazeView() const {
    return maze;
}

std::wstring GameWorld::EncodeSavedMazeBytes(const std::vector<uint8_t>& bytes) {
    std::wstring out;
    out.reserve(bytes.size());
    for (uint8_t v : bytes) out.push_back(static_cast<wchar_t>(L'0' + std::clamp<int>(v, 0, 9)));
    return out;
}

void GameWorld::DecodeSavedMazeBytes(const std::wstring& text, std::vector<uint8_t>& out, size_t expectedSize) {
    out.assign(expectedSize, 0);
    size_t count = std::min(expectedSize, text.size());
    for (size_t i = 0; i < count; ++i) {
        wchar_t ch = text[i];
        out[i] = (ch >= L'0' && ch <= L'9') ? static_cast<uint8_t>(ch - L'0') : 0;
    }
}

void GameWorld::WriteSavedRunFields(std::wostream& out) const {
    out << L"MazeW=" << maze.w << L"\n";
    out << L"MazeH=" << maze.h << L"\n";
    out << L"MazeTileW=" << maze.tileW << L"\n";
    out << L"MazeTileD=" << maze.tileD << L"\n";
    out << L"StartX=" << maze.start.x << L"\n";
    out << L"StartY=" << maze.start.y << L"\n";
    out << L"ExitX=" << maze.exit.x << L"\n";
    out << L"ExitY=" << maze.exit.y << L"\n";
    out << L"Open=" << EncodeSavedMazeBytes(maze.open) << L"\n";
    out << L"WallFeatures=" << EncodeSavedMazeBytes(maze.wallFeatures) << L"\n";
    out << L"CameraX=" << player.position.x << L"\n";
    out << L"CameraY=" << player.position.y << L"\n";
    out << L"CameraZ=" << player.position.z << L"\n";
    out << L"Yaw=" << player.yaw << L"\n";
    out << L"BodyYaw=" << player.bodyYaw << L"\n";
    out << L"LookPitch=" << player.pitch << L"\n";
    out << L"PlayerHealth=" << player.health << L"\n";
    out << L"PlayerStamina=" << player.stamina << L"\n";
    out << L"CollectedPages=" << collectiblePagesCollected << L"\n";
    out << L"SavePointActive=" << (savePoint.active ? 1 : 0) << L"\n";
    out << L"SavePointX=" << savePoint.pos.x << L"\n";
    out << L"SavePointY=" << savePoint.pos.y << L"\n";
    out << L"SavePointZ=" << savePoint.pos.z << L"\n";
    out << L"SavePointYaw=" << savePoint.yaw << L"\n";
    for (size_t i = 0; i < collectiblePages.size(); ++i) {
        out << L"Page" << i << L"Collected=" << (collectiblePages[i].collected ? 1 : 0) << L"\n";
    }
}

GameWorldSavedMazeRestoreState GameWorld::ReadSavedMazeRestoreState(
    const SavedRunKeyValues& values,
    const PlayableLevelSpec& spec,
    const MazeLayoutSpec& fallbackLayout) const {
    GameWorldSavedMazeRestoreState saved{};
    saved.w = std::clamp(SavedRunInt(values, L"MazeW", spec.mazeWidth), 3, 151);
    saved.h = std::clamp(SavedRunInt(values, L"MazeH", spec.mazeHeight), 3, 151);
    saved.tileW = std::clamp(SavedRunFloat(values, L"MazeTileW", fallbackLayout.tileW), 1.2f, 8.0f);
    saved.tileD = std::clamp(SavedRunFloat(values, L"MazeTileD", fallbackLayout.tileD), 1.2f, 8.0f);
    saved.start = {
        std::clamp(SavedRunInt(values, L"StartX", 1), 0, saved.w - 1),
        std::clamp(SavedRunInt(values, L"StartY", 1), 0, saved.h - 1)
    };
    saved.exit = {
        std::clamp(SavedRunInt(values, L"ExitX", saved.w - 2), 0, saved.w - 1),
        std::clamp(SavedRunInt(values, L"ExitY", saved.h - 2), 0, saved.h - 1)
    };
    saved.open = SavedRunString(values, L"Open");
    saved.wallFeatures = SavedRunString(values, L"WallFeatures");
    return saved;
}

void GameWorld::RestoreSavedMazeGeometry(const GameWorldSavedMazeRestoreState& saved) {
    maze.w = saved.w;
    maze.h = saved.h;
    maze.tileW = saved.tileW;
    maze.tileD = saved.tileD;
    maze.start = saved.start;
    maze.exit = saved.exit;
    const size_t cellCount = static_cast<size_t>(maze.w * maze.h);
    DecodeSavedMazeBytes(saved.open, maze.open, cellCount);
    DecodeSavedMazeBytes(saved.wallFeatures, maze.wallFeatures, cellCount);
    if (!maze.IsOpen(maze.start.x, maze.start.y)) maze.SetOpen(maze.start.x, maze.start.y);
    if (!maze.IsOpen(maze.exit.x, maze.exit.y)) maze.SetOpen(maze.exit.x, maze.exit.y);
}

GameWorldSavedRuntimeRestoreState GameWorld::ReadSavedRuntimeRestoreState(const SavedRunKeyValues& values) const {
    GameWorldSavedRuntimeRestoreState saved{};
    XMFLOAT3 defaultPlayer = maze.WorldCenter(maze.start, 1.45f);
    saved.playerPosition = {
        SavedRunFloat(values, L"CameraX", defaultPlayer.x),
        SavedRunFloat(values, L"CameraY", 1.45f),
        SavedRunFloat(values, L"CameraZ", defaultPlayer.z)
    };
    saved.playerYaw = SavedRunFloat(values, L"Yaw", 0.0f);
    saved.playerBodyYaw = SavedRunFloat(values, L"BodyYaw", saved.playerYaw);
    saved.playerPitch = SavedRunFloat(values, L"LookPitch", -0.055f);
    saved.playerHealth = SavedRunFloat(values, L"PlayerHealth", 100.0f);
    saved.playerStamina = SavedRunFloat(values, L"PlayerStamina", 100.0f);
    saved.collectedPages = SavedRunInt(values, L"CollectedPages", 0);
    saved.savePoint.active = SavedRunInt(values, L"SavePointActive", savePoint.active ? 1 : 0) != 0;
    saved.savePoint.pos = {
        SavedRunFloat(values, L"SavePointX", savePoint.pos.x),
        SavedRunFloat(values, L"SavePointY", savePoint.pos.y),
        SavedRunFloat(values, L"SavePointZ", savePoint.pos.z)
    };
    saved.savePoint.yaw = SavedRunFloat(values, L"SavePointYaw", savePoint.yaw);
    for (size_t i = 0; i < saved.pageCollected.size(); ++i) {
        std::wstring key = L"Page" + std::to_wstring(i) + L"Collected";
        bool fallback = i < collectiblePages.size() && collectiblePages[i].collected;
        saved.pageCollected[i] = SavedRunInt(values, key.c_str(), fallback ? 1 : 0) != 0 ? 1 : 0;
    }
    return saved;
}

void GameWorld::RestoreSavedRuntimeState(const GameWorldSavedRuntimeRestoreState& saved) {
    player.RestoreSavedRunState(
        saved.playerPosition,
        saved.playerYaw,
        saved.playerBodyYaw,
        saved.playerPitch,
        saved.playerHealth,
        saved.playerStamina);
    RestoreSavePoint(saved.savePoint.active, saved.savePoint.pos, saved.savePoint.yaw);
    for (size_t i = 0; i < saved.pageCollected.size(); ++i) {
        RestoreCollectiblePageCollected(i, saved.pageCollected[i] != 0, true);
    }
    ReconcileCollectiblePageRestore(saved.collectedPages);
}

void GameWorld::RestoreFlashlightState(bool flashlightEnabled) {
    player.flashlightEnabled = flashlightEnabled;
    player.flashlightPressedLastFrame = false;
}

void GameWorld::RestoreInteractLatch(bool interactPressedLastFrame) {
    player.interactPressedLastFrame = interactPressedLastFrame;
}

void GameWorld::ResetPlayerInputLatches(bool flashlightEnabled) {
    PlayerController::ResetInputLatches(player, flashlightEnabled);
}

void GameWorld::RefillPlayerStamina() {
    player.RefillStamina();
}

void GameWorld::RestorePlayerFullVitals() {
    player.RestoreFullVitals();
}

void GameWorld::ResetPlayerForSession(float initialSmoothedMoveSpeed) {
    player.ResetSessionState(
        StartWorldCenter(1.45f),
        initialSmoothedMoveSpeed,
        static_cast<size_t>(std::max(0, maze.w * maze.h)));
}

void GameWorld::SetPlayerCameraPose(const XMFLOAT3& position, float yaw, float bodyYaw, float pitch) {
    player.position = position;
    player.yaw = yaw;
    player.bodyYaw = bodyYaw;
    player.pitch = pitch;
}

void GameWorld::SetPlayerHorizontalPosition(float x, float z) {
    player.position.x = x;
    player.position.z = z;
}

void GameWorld::AdvancePlayerStepPhase(float metersMoved, float speedMetersPerSecond, const PlayerMovementTuning& tuning) {
    PlayerController::AdvanceStepPhase(player, metersMoved, speedMetersPerSecond, tuning);
}

void GameWorld::MovePlayerHorizontalAndAdvanceStep(
    float x,
    float z,
    float metersMoved,
    float speedMetersPerSecond,
    const PlayerMovementTuning& tuning) {
    SetPlayerHorizontalPosition(x, z);
    AdvancePlayerStepPhase(metersMoved, speedMetersPerSecond, tuning);
}

PlayerManualControlResult GameWorld::UpdatePlayerManualControl(
    const PlayerManualControlInput& input,
    const PlayerManualControlServices& services) {
    return PlayerController::UpdateManualControl(player, input, services);
}

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

bool GameWorld::PlayableRunFinished() const {
    return playableRun.runFinished;
}

bool GameWorld::PlayableRunActive() const {
    return playableRun.IsActive();
}

bool GameWorld::CanSaveActiveRun() const {
    return playableRun.CanSaveActiveRun();
}

void GameWorld::WritePlayableRunFields(std::wostream& out) const {
    playableRun.WriteSavedRunFields(out);
}

int GameWorld::RestoreSavedPlayableRunState(PlayableRunState::SavedRunRestoreState savedRun) {
    return playableRun.RestoreSavedRunState(std::move(savedRun));
}

void GameWorld::RestorePlayableSaveItemsSpawned(int saveItemsSpawned) {
    playableRun.RestoreSaveItemsSpawned(saveItemsSpawned);
}

PlayableLevelSpec GameWorld::CurrentPlayableLevel() const {
    return playableRun.CurrentLevel();
}

bool GameWorld::PlayableRunIsCustomGame() const {
    return playableRun.IsCustomGame();
}

const CustomGameSpec& GameWorld::CurrentCustomSpec() const {
    return playableRun.CustomSpec();
}

bool GameWorld::DarkLayerOneAppliesTo(int layer) const {
    return playableRun.DarkLayerOneAppliesTo(layer);
}

PlayableLevelSpec GameWorld::BuildLayerOneLevelSpec(int levelInLayer, std::mt19937& rng) {
    return playableRun.BuildLayerOneLevelSpec(levelInLayer, rng);
}

void GameWorld::BeginPlayableLevel(const PlayableLevelSpec& level) {
    playableRun.BeginLevel(level);
}

void GameWorld::BeginLayerRun(bool darkLayerOne, std::mt19937& rng) {
    playableRun.BeginLayerRun(darkLayerOne, rng);
}

void GameWorld::BeginCustomRun(const CustomGameSpec& customSpec, std::mt19937& rng) {
    playableRun.BeginCustomRun(customSpec, rng);
}

void GameWorld::WriteLevelStartNotice(std::wostream& out) const {
    playableRun.WriteLevelStartNotice(out);
}

float GameWorld::RunSeconds() const {
    return playableRun.RunSeconds();
}

int GameWorld::TotalScore() const {
    return playableRun.TotalScore();
}

bool GameWorld::PlayableScoreScreenActive() const {
    return playableRun.scoreScreenActive;
}

bool GameWorld::PlayableLevelRunning() const {
    return playableRun.active && playableRun.levelRunning;
}

bool GameWorld::PlayableBossLevelRunning() const {
    return PlayableLevelRunning() && playableRun.currentLevel.bossEncounter;
}

float GameWorld::AirParticleDensityScale() const {
    return playableRun.AirParticleDensityScale();
}

float GameWorld::MapDirtProgression() const {
    return playableRun.MapDirtProgression();
}

PlayableCustomScareGate GameWorld::CustomScareGateFor(int effectIndex) const {
    return playableRun.CustomScareGateFor(effectIndex);
}

int GameWorld::ScoreCompletedPlayableLevel(float levelSeconds, bool bossEncounter) const {
    int base = 1000 + playableRun.currentLevel.levelInLayer * 250;
    int speedBonus = std::max(0, 900 - static_cast<int>(std::round(levelSeconds * 6.0f)));
    int healthBonus = static_cast<int>(std::round(std::clamp(player.health, 0.0f, 100.0f) * 3.0f));
    int pageBonus = collectiblePagesCollected * 150;
    int bossBonus = bossEncounter ? 750 : 0;
    return std::max(0, base + speedBonus + healthBonus + pageBonus + bossBonus);
}

int GameWorld::ScoreCurrentPlayableLevel() const {
    return ScoreCompletedPlayableLevel(
        playableRun.CurrentLevelSeconds(),
        playableRun.CurrentLevelHasBossEncounter());
}

bool GameWorld::CanAdvancePlayableProgression() const {
    return progressionEnabled && PlayableLevelRunning() && !deathActive && !exitTransitionActive;
}

PlayableLevelCompletionUpdate GameWorld::CompleteCurrentPlayableLevel() {
    return playableRun.CompleteCurrentLevel(ScoreCurrentPlayableLevel());
}

bool GameWorld::CanContinueAfterScoreScreen() const {
    return playableRun.CanContinueAfterScoreScreen();
}

int GameWorld::NextLevelAfterScoreScreen() const {
    return playableRun.NextLevelAfterScoreScreen();
}

bool GameWorld::AdvancePlayableProgressionTimers(float dt) {
    if (!CanAdvancePlayableProgression()) return false;
    playableRun.AdvanceRunningTimers(dt);
    return true;
}

void GameWorld::ResetCollectiblePagesForGeneration() {
    collectiblePagesCollected = 0;
    for (CollectiblePage& page : collectiblePages) {
        page = {};
        page.collected = true;
    }
}

bool GameWorld::PlaceCollectiblePage(int pageSlot, const CollectiblePage& page) {
    if (pageSlot < 0 || pageSlot >= static_cast<int>(collectiblePages.size())) return false;
    collectiblePages[static_cast<size_t>(pageSlot)] = page;
    return true;
}

bool GameWorld::IsLayerPageCollected(int pageIndex) const {
    return playableRun.active && playableRun.IsLayerPageCollected(pageIndex);
}

GameWorldCollectiblePickupResult GameWorld::CollectPage(size_t pageSlot, bool updatePlayableRun) {
    GameWorldCollectiblePickupResult result{};
    if (pageSlot >= collectiblePages.size()) return result;

    CollectiblePage& page = collectiblePages[pageSlot];
    if (page.collected) return result;

    page.collected = true;
    collectiblePagesCollected = std::clamp(collectiblePagesCollected + 1, 0, kCollectiblePageMaterialCount);
    if (updatePlayableRun && playableRun.active) {
        playableRun.MarkLayerPageCollected(page.pageIndex);
    }

    result.collected = true;
    result.pageIndex = page.pageIndex;
    result.displayCollected = updatePlayableRun && playableRun.active
        ? playableRun.layerPagesCollected
        : collectiblePagesCollected;
    return result;
}

void GameWorld::ClearSavePoint() {
    savePoint = {};
}

void GameWorld::RestoreSavePoint(bool active, const XMFLOAT3& pos, float yaw) {
    savePoint.active = active;
    savePoint.pos = pos;
    savePoint.yaw = yaw;
}

void GameWorld::RestoreCollectiblePageCollected(size_t pageSlot, bool collected, bool updatePlayableRun) {
    if (pageSlot >= collectiblePages.size()) return;

    CollectiblePage& page = collectiblePages[pageSlot];
    page.collected = collected;
    if (updatePlayableRun && playableRun.active && page.collected) {
        playableRun.RestoreLayerPageCollected(page.pageIndex, true);
    }
}

void GameWorld::ReconcileCollectiblePageRestore(int savedCollectedPages) {
    collectiblePagesCollected = std::clamp(savedCollectedPages, 0, kCollectiblePageMaterialCount);
    playableRun.ReconcileLayerPagesCollected();
}

void GameWorld::RecomputePlayerNoiseRadiusFromPulses() {
    player.audibleNoiseRadiusMeters = 0.0f;
    for (const PlayerAudibleSoundPulse& pulse : playerSoundPulses) {
        if (pulse.radius <= 0.0f || pulse.life <= 0.0f || pulse.age >= pulse.life) continue;
        player.audibleNoiseRadiusMeters = std::max(player.audibleNoiseRadiusMeters, pulse.radius);
    }
}

void GameWorld::AdvancePlayerSoundPulses(float dt) {
    float step = std::max(0.0f, dt);
    for (PlayerAudibleSoundPulse& pulse : playerSoundPulses) {
        pulse.age += step;
    }
    playerSoundPulses.erase(std::remove_if(playerSoundPulses.begin(), playerSoundPulses.end(),
        [](const PlayerAudibleSoundPulse& pulse) {
            return pulse.life <= 0.0f || pulse.age >= pulse.life;
        }), playerSoundPulses.end());
    RecomputePlayerNoiseRadiusFromPulses();
}

bool GameWorld::EmitPlayerSoundPulse(const XMFLOAT3& pos, float radius, float life, size_t maxPulses) {
    if (radius <= 0.01f) return false;

    PlayerAudibleSoundPulse pulse{};
    pulse.pos = pos;
    pulse.radius = radius;
    pulse.life = std::max(0.10f, life);
    playerSoundPulses.push_back(pulse);
    while (playerSoundPulses.size() > maxPulses) {
        playerSoundPulses.erase(playerSoundPulses.begin());
    }
    player.audibleNoiseRadiusMeters = std::max(player.audibleNoiseRadiusMeters, radius);
    return true;
}

void GameWorld::QueueAudioEvent(const GameAudioEvent& event) {
    if (event.kind == GameAudioEventKind::PlayOneShot && event.volume <= 0.0f && event.hearingRadius <= 0.0f) return;
    if (event.kind == GameAudioEventKind::PlayerNoise && event.hearingRadius <= 0.0f) return;
    audioEvents.push_back(event);
    while (audioEvents.size() > 64) {
        audioEvents.erase(audioEvents.begin());
    }
}

std::vector<GameAudioEvent> GameWorld::DrainAudioEvents() {
    std::vector<GameAudioEvent> events;
    events.swap(audioEvents);
    return events;
}

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

bool GameWorld::BeginExitTransition() {
    if (exitTransitionActive) return false;
    exitTransitionActive = true;
    exitTransitionTimer = 0.0f;
    return true;
}

void GameWorld::AdvanceExitTransition(float dt) {
    if (!exitTransitionActive) return;
    exitTransitionTimer += dt;
}

void GameWorld::EndExitTransition() {
    exitTransitionActive = false;
}

bool GameWorld::BeginDeath() {
    if (deathActive) return false;
    deathActive = true;
    deathTimer = 0.0f;
    monster.killCount = std::min(monster.killCount + 1, 16);
    return true;
}

bool GameWorld::BeginPlayerDeath() {
    player.Kill();
    return BeginDeath();
}

void GameWorld::AdvanceDeath(float dt) {
    if (!deathActive) return;
    deathTimer += dt;
}

void GameWorld::GenerateCollectiblePagesForCurrentLevel(bool enabled, float wallHeightMeters, std::mt19937& rng) {
        ResetCollectiblePagesForGeneration();
        if (!enabled || maze.open.empty()) return;

        int firstPageIndex = 0;
        int pageCount = kCollectiblePageMaterialCount;
        if (playableRun.active) {
            playableRun.EnsureLayerPageDistribution();
            firstPageIndex = playableRun.LayerPageStartForLevel(playableRun.levelInLayer);
            pageCount = playableRun.LayerPageCountForLevel(playableRun.levelInLayer);
        }
        if (pageCount <= 0) return;

        auto randRange = [&rng](float a, float b) {
            std::uniform_real_distribution<float> dist(a, b);
            return dist(rng);
        };
        auto scale = [](XMFLOAT3 a, float s) {
            return XMFLOAT3{a.x * s, a.y * s, a.z * s};
        };
        auto add = [](XMFLOAT3 a, XMFLOAT3 b) {
            return XMFLOAT3{a.x + b.x, a.y + b.y, a.z + b.z};
        };
        auto cross = [](XMFLOAT3 a, XMFLOAT3 b) {
            return XMFLOAT3{
                a.y * b.z - a.z * b.y,
                a.z * b.x - a.x * b.z,
                a.x * b.y - a.y * b.x
            };
        };
        auto dot = [](XMFLOAT3 a, XMFLOAT3 b) {
            return a.x * b.x + a.y * b.y + a.z * b.z;
        };
        auto normalize = [&scale, &dot](XMFLOAT3 a, XMFLOAT3 fallback) {
            float len = std::sqrt(std::max(0.0f, dot(a, a)));
            if (len <= 0.00001f) return fallback;
            return scale(a, 1.0f / len);
        };

        std::vector<Tile> openTiles;
        openTiles.reserve(static_cast<size_t>(maze.w * maze.h));
        auto collectibleFloorTile = [this](Tile t) {
            return maze.IsOpen(t.x, t.y) && maze.WallFeature(t.x, t.y) == MazeWallFeature::None;
        };
        auto collectibleWallNeighbor = [this](Tile t) {
            return maze.InBounds(t.x, t.y) &&
                !maze.IsOpen(t.x, t.y) &&
                maze.WallFeature(t.x, t.y) == MazeWallFeature::None;
        };
        for (int y = 1; y < maze.h - 1; ++y) {
            for (int x = 1; x < maze.w - 1; ++x) {
                Tile t{x, y};
                if (!collectibleFloorTile(t) || t == maze.start || t == maze.exit) continue;
                if (std::abs(x - maze.start.x) + std::abs(y - maze.start.y) < 4) continue;
                openTiles.push_back(t);
            }
        }
        if (openTiles.empty()) return;

        std::array<Tile, kCollectiblePageMaterialCount> usedTiles{};
        int usedCount = 0;
        auto tileUsed = [&](Tile t) {
            for (int i = 0; i < usedCount; ++i) {
                if (usedTiles[static_cast<size_t>(i)] == t) return true;
            }
            return false;
        };
        auto rememberTile = [&](Tile t) {
            if (usedCount < static_cast<int>(usedTiles.size())) {
                usedTiles[static_cast<size_t>(usedCount++)] = t;
            }
        };

        constexpr float pageH = 0.297f;
        const int dirs[4][2] = {{1, 0}, {-1, 0}, {0, 1}, {0, -1}};
        for (int pageOffset = 0; pageOffset < pageCount; ++pageOffset) {
            int pageIndex = firstPageIndex + pageOffset;
            if (pageIndex < 0 || pageIndex >= kCollectiblePageMaterialCount) continue;
            if (playableRun.active && IsLayerPageCollected(pageIndex)) continue;

            bool placed = false;
            for (int attempt = 0; attempt < 220 && !placed; ++attempt) {
                Tile t = openTiles[static_cast<size_t>(rng() % openTiles.size())];
                if (tileUsed(t) && attempt < 96) continue;
                XMFLOAT3 tileCenter = maze.WorldCenter(t, 0.0f);
                bool preferWall = randRange(0.0f, 1.0f) < 0.68f;
                if (preferWall) {
                    std::array<int, 4> order{{0, 1, 2, 3}};
                    std::shuffle(order.begin(), order.end(), rng);
                    for (int ord : order) {
                        int dx = dirs[ord][0];
                        int dy = dirs[ord][1];
                        if (!collectibleWallNeighbor({t.x + dx, t.y + dy})) continue;
                        XMFLOAT3 normal{static_cast<float>(-dx), 0.0f, static_cast<float>(-dy)};
                        normal = normalize(normal, {0.0f, 0.0f, 1.0f});
                        XMFLOAT3 right = normalize(cross({0.0f, 1.0f, 0.0f}, normal), {1.0f, 0.0f, 0.0f});
                        float lateral = randRange(-0.36f, 0.36f) * ((dx != 0) ? maze.tileD : maze.tileW);
                        XMFLOAT3 wallCenter = tileCenter;
                        wallCenter.x += static_cast<float>(dx) * maze.tileW * 0.5f;
                        wallCenter.z += static_cast<float>(dy) * maze.tileD * 0.5f;
                        wallCenter = add(wallCenter, scale(normal, 0.010f));
                        wallCenter = add(wallCenter, scale(right, lateral));
                        wallCenter.y = std::clamp(1.38f + randRange(-0.22f, 0.20f), pageH * 0.5f + 0.08f, wallHeightMeters - pageH * 0.5f - 0.05f);
                        XMFLOAT3 baseRight = right;
                        XMFLOAT3 baseUp{0.0f, 1.0f, 0.0f};
                        float roll = randRange(-0.115f, 0.115f);
                        float cr = std::cos(roll);
                        float sr = std::sin(roll);
                        right = normalize(add(scale(baseRight, cr), scale(baseUp, sr)), baseRight);
                        XMFLOAT3 pageUp = normalize(add(scale(baseUp, cr), scale(baseRight, -sr)), baseUp);
                        PlaceCollectiblePage(pageIndex, {wallCenter, right, pageUp, normal, pageIndex, false});
                        rememberTile(t);
                        placed = true;
                        break;
                    }
                }
                if (!placed) {
                    float px = tileCenter.x + randRange(-0.32f, 0.32f) * std::max(0.1f, maze.tileW - pageH);
                    float pz = tileCenter.z + randRange(-0.32f, 0.32f) * std::max(0.1f, maze.tileD - pageH);
                    float yaw = randRange(0.0f, kPi * 2.0f);
                    XMFLOAT3 right{std::cos(yaw), 0.0f, -std::sin(yaw)};
                    XMFLOAT3 up{std::sin(yaw), 0.0f, std::cos(yaw)};
                    PlaceCollectiblePage(pageIndex, {{px, 0.006f + pageIndex * 0.00015f, pz}, right, up, {0.0f, 1.0f, 0.0f}, pageIndex, false});
                    rememberTile(t);
                    placed = true;
                }
            }
        }
    }

GameWorldCollectibleAimResult GameWorld::FindCollectiblePageInView(float maxDistance) const {
        GameWorldCollectibleAimResult result{};
        XMFLOAT3 origin{player.position.x, player.position.y, player.position.z};
        float pitchCos = std::cos(player.pitch);
        XMFLOAT3 dir{
            std::sin(player.yaw) * pitchCos,
            std::sin(player.pitch),
            std::cos(player.yaw) * pitchCos
        };
        float bestT = std::max(0.0f, maxDistance);
        Tile cameraTile = maze.TileFromWorld(player.position.x, player.position.z);

        for (size_t i = 0; i < collectiblePages.size(); ++i) {
            const CollectiblePage& page = collectiblePages[i];
            if (page.collected) continue;

            float denom = dir.x * page.normal.x + dir.y * page.normal.y + dir.z * page.normal.z;
            if (std::abs(denom) < 0.08f) continue;

            XMFLOAT3 toPage{
                page.center.x - origin.x,
                page.center.y - origin.y,
                page.center.z - origin.z
            };
            float t = (toPage.x * page.normal.x + toPage.y * page.normal.y + toPage.z * page.normal.z) / denom;
            if (t <= 0.05f || t >= bestT) continue;

            XMFLOAT3 hit{
                origin.x + dir.x * t,
                origin.y + dir.y * t,
                origin.z + dir.z * t
            };
            XMFLOAT3 local{
                hit.x - page.center.x,
                hit.y - page.center.y,
                hit.z - page.center.z
            };
            float u = local.x * page.right.x + local.y * page.right.y + local.z * page.right.z;
            float v = local.x * page.up.x + local.y * page.up.y + local.z * page.up.z;
            if (std::abs(u) > 0.115f || std::abs(v) > 0.160f) continue;

            Tile pageTile = maze.TileFromWorld(page.center.x, page.center.z);
            if (!maze.LineClear(cameraTile, pageTile)) continue;

            bestT = t;
            result.found = true;
            result.pageSlot = i;
        }
        return result;
    }
