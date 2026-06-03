#pragma once

#include "player_controller.h"
#include "game_world_types.h"

// Gameplay-owned world state. Renderer aliases still point into this aggregate
// while simulation systems move across in small, verifiable slices.

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

struct GameWorld {
    Maze maze;
    PlayerState player;
    MonsterState monster;
    PlayableRunState playableRun;

    std::array<CollectiblePage, kCollectiblePageMaterialCount> collectiblePages{};
    int collectiblePagesCollected = 0;
    SavePoint savePoint;
    std::vector<PlayerAudibleSoundPulse> playerSoundPulses;
    std::vector<GameAudioEvent> audioEvents;

    bool progressionEnabled = false;
    bool exitTransitionActive = false;
    float exitTransitionTimer = 0.0f;
    bool deathActive = false;
    float deathTimer = 0.0f;

    void ResetSessionState();
    GameWorldRenderSnapshot BuildRenderSnapshot() const;
    GameWorldSnapshotState CaptureSnapshotState() const;
    void RestoreSnapshotGenerationState(const GameWorldSnapshotState& snapshot);
    void RestoreSnapshotRuntimeState(GameWorldSnapshotState snapshot);
    void ResetMonsterKillCount();
    void ResetPlayableRun();
    void ApplyMazeLayout(const MazeLayoutSpec& spec, bool updateExit = true);
    void GenerateMaze(const GameWorldMazeGenerationRequest& request);
    const Maze& MazeView() const;
    static std::wstring EncodeSavedMazeBytes(const std::vector<uint8_t>& bytes);
    static void DecodeSavedMazeBytes(const std::wstring& text, std::vector<uint8_t>& out, size_t expectedSize);

    void WriteSavedRunFields(std::wostream& out) const;
    GameWorldSavedMazeRestoreState ReadSavedMazeRestoreState(
        const SavedRunKeyValues& values,
        const PlayableLevelSpec& spec,
        const MazeLayoutSpec& fallbackLayout) const;
    void RestoreSavedMazeGeometry(const GameWorldSavedMazeRestoreState& saved);
    GameWorldSavedRuntimeRestoreState ReadSavedRuntimeRestoreState(const SavedRunKeyValues& values) const;
    void RestoreSavedRuntimeState(const GameWorldSavedRuntimeRestoreState& saved);

    void RestoreFlashlightState(bool flashlightEnabled);
    void RestoreInteractLatch(bool interactPressedLastFrame);
    void ResetPlayerInputLatches(bool flashlightEnabled);
    void RefillPlayerStamina();
    void RestorePlayerFullVitals();
    void ResetPlayerForSession(float initialSmoothedMoveSpeed);
    void SetPlayerCameraPose(const XMFLOAT3& position, float yaw, float bodyYaw, float pitch);
    void SetPlayerHorizontalPosition(float x, float z);
    void AdvancePlayerStepPhase(float metersMoved, float speedMetersPerSecond, const PlayerMovementTuning& tuning);
    void MovePlayerHorizontalAndAdvanceStep(float x, float z, float metersMoved, float speedMetersPerSecond, const PlayerMovementTuning& tuning);
    PlayerManualControlResult UpdatePlayerManualControl(
        const PlayerManualControlInput& input,
        const PlayerManualControlServices& services);
    void ClearMonsterChasePressure();
    void ResetMonsterForSession(float initialRoamTimer);
    void SetMonsterPreviewPose();
    bool MonsterCuriosityActive() const;
    float MonsterCuriosityAmount() const;
    bool MonsterRecognitionFreezeActive() const;
    bool MonsterChaseCommitted() const;
    bool MonsterChasePanicActive() const;
    bool MonsterRecognitionExpired() const;
    bool MonsterRecognitionPending() const;
    bool MonsterRecognizedForChase() const;
    float MonsterChaseMemoryTimer() const;
    float MonsterChasePanic() const;
    void BeginMonsterRecognition(float duration);
    void CancelMonsterRecognition();
    void AdvanceMonsterRecognition(float dt);
    void BeginMonsterCuriosity(float duration);
    void AdvanceMonsterCuriosity(float dt);
    void ClearMonsterCuriosity();
    void CommitMonsterChaseRecognition();
    void ReleaseMonsterChaseRecognition();
    void HoldMonsterChaseMemory(float holdSeconds);
    void DecayMonsterChaseMemory(float dt);
    void UpdateMonsterChasePanic(float target, float response, float dt);
    void RaisePlayerRunPressure(float minRunIntensity, float minRunEffort, float minSmoothedMoveSpeed);
    void SetPlayerSmoothedMoveSpeed(float speed);
    Tile StartTile() const;
    XMFLOAT3 StartWorldCenter(float y) const;
    XMFLOAT3 ExitWorldCenter(float y) const;
    float MazeTileMinimum() const;
    bool DeathActive() const;
    size_t TileIndex(Tile t) const;
    uint16_t VisitCount(Tile t) const;
    bool HasVisitedMapTiles() const;
    void MarkVisited(Tile t);
    bool PlayerTunnelCrouchLocked() const;
    float PlayerTunnelLeanSideTarget() const;
    float PlayerTunnelPostureHoldTimer() const;
    void ResetPlayerTunnelPosture();
    void SetPlayerTunnelCrouchLocked(bool locked);
    void HoldPlayerTunnelCrouch(float holdSeconds);
    void SetPlayerTunnelLeanSideTarget(float side);
    void SetPlayerTunnelLeanTarget(bool active);
    void AdvancePlayerTunnelPostureTimer(float dt);
    void AdvancePlayerTunnelCameraLean(float dt);
    bool PlayableRunFinished() const;
    bool PlayableRunActive() const;
    bool CanSaveActiveRun() const;
    void WritePlayableRunFields(std::wostream& out) const;
    int RestoreSavedPlayableRunState(PlayableRunState::SavedRunRestoreState savedRun);
    void RestorePlayableSaveItemsSpawned(int saveItemsSpawned);
    PlayableLevelSpec CurrentPlayableLevel() const;
    bool PlayableRunIsCustomGame() const;
    const CustomGameSpec& CurrentCustomSpec() const;
    bool DarkLayerOneAppliesTo(int layer) const;
    PlayableLevelSpec BuildLayerOneLevelSpec(int levelInLayer, std::mt19937& rng);
    void BeginPlayableLevel(const PlayableLevelSpec& level);
    void BeginLayerRun(bool darkLayerOne, std::mt19937& rng);
    void BeginCustomRun(const CustomGameSpec& customSpec, std::mt19937& rng);
    void WriteLevelStartNotice(std::wostream& out) const;
    float RunSeconds() const;
    int TotalScore() const;
    bool PlayableScoreScreenActive() const;
    bool PlayableLevelRunning() const;
    bool PlayableBossLevelRunning() const;
    float AirParticleDensityScale() const;
    float MapDirtProgression() const;
    PlayableCustomScareGate CustomScareGateFor(int effectIndex) const;

    int ScoreCompletedPlayableLevel(float levelSeconds, bool bossEncounter) const;
    int ScoreCurrentPlayableLevel() const;
    bool CanAdvancePlayableProgression() const;
    PlayableLevelCompletionUpdate CompleteCurrentPlayableLevel();
    bool CanContinueAfterScoreScreen() const;
    int NextLevelAfterScoreScreen() const;
    bool AdvancePlayableProgressionTimers(float dt);

    void ResetCollectiblePagesForGeneration();
    bool PlaceCollectiblePage(int pageSlot, const CollectiblePage& page);

    void GenerateCollectiblePagesForCurrentLevel(bool enabled, float wallHeightMeters, std::mt19937& rng);
    GameWorldCollectibleAimResult FindCollectiblePageInView(float maxDistance) const;
    bool IsLayerPageCollected(int pageIndex) const;
    GameWorldCollectiblePickupResult CollectPage(size_t pageSlot, bool updatePlayableRun);
    void ClearSavePoint();
    void RestoreSavePoint(bool active, const XMFLOAT3& pos, float yaw);
    void RestoreCollectiblePageCollected(size_t pageSlot, bool collected, bool updatePlayableRun);

    void ReconcileCollectiblePageRestore(int savedCollectedPages);
    void RecomputePlayerNoiseRadiusFromPulses();
    void AdvancePlayerSoundPulses(float dt);
    bool EmitPlayerSoundPulse(const XMFLOAT3& pos, float radius, float life, size_t maxPulses);
    void QueueAudioEvent(const GameAudioEvent& event);
    std::vector<GameAudioEvent> DrainAudioEvents();

    GameWorldSavePointSpawnPlan BuildSavePointSpawnPlan() const;
    GameWorldSavePointCandidateResult PickSavePointCandidate(std::mt19937& rng) const;
    bool TryGenerateSavePoint(bool enabled, float spawnChance, std::mt19937& rng);
    void ActivateSavePoint(const XMFLOAT3& pos, float yaw);
    bool CanInteractWithSavePoint() const;
    bool BeginExitTransition();
    void AdvanceExitTransition(float dt);
    void EndExitTransition();
    bool BeginDeath();
    bool BeginPlayerDeath();
    void AdvanceDeath(float dt);
};
