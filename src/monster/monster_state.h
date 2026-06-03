#pragma once

// Gameplay-owned monster state model. Rendering-only body/trail smoothing stays
// in the renderer until monster presentation is split from monster decisions.

struct MonsterSnapshotState {
    XMFLOAT3 position{};
    float yaw = 0.0f;
    std::vector<Tile> path;
    size_t pathIndex = 0;
    Tile goal{-1000, -1000};
    Tile soundTile{-1000, -1000};
    Tile lastKnownTile{-1000, -1000};
    Tile roamTile{-1000, -1000};
    bool hasSound = false;
    bool hasLastKnown = false;
    float repathTimer = 0.0f;
    bool chasingVisible = false;
    bool heardPlayerNow = false;
    bool canSeePlayerNow = false;
    int killCount = 0;
    float searchTimer = 0.0f;
    float roamTimer = 0.0f;
    float roamPauseTimer = 0.0f;
    float roamBurstTimer = 0.0f;
    bool recognitionActive = false;
    bool recognizedForChase = false;
    float recognitionTimer = 0.0f;
    float recognitionDuration = 0.0f;
    float curiosityTimer = 0.0f;
    float curiosityDuration = 0.0f;
    float chaseMemoryTimer = 0.0f;
    float chasePanic = 0.0f;
};

struct MonsterState {
    XMFLOAT3 position{};
    float yaw = 0.0f;

    std::vector<Tile> path;
    size_t pathIndex = 0;
    Tile goal{-1000, -1000};

    Tile soundTile{-1000, -1000};
    Tile lastKnownTile{-1000, -1000};
    Tile roamTile{-1000, -1000};
    bool hasSound = false;
    bool hasLastKnown = false;

    float repathTimer = 0.0f;
    bool chasingVisible = false;
    bool heardPlayerNow = false;
    bool canSeePlayerNow = false;
    int killCount = 0;

    float searchTimer = 0.0f;
    float roamTimer = 0.0f;
    float roamPauseTimer = 0.0f;
    float roamBurstTimer = 0.0f;

    bool recognitionActive = false;
    bool recognizedForChase = false;
    float recognitionTimer = 0.0f;
    float recognitionDuration = 0.0f;

    float curiosityTimer = 0.0f;
    float curiosityDuration = 0.0f;
    float chaseMemoryTimer = 0.0f;
    float chasePanic = 0.0f;

    void ResetKillCount();
    bool CuriosityActive() const;
    bool RecognitionFreezeActive() const;
    bool ChaseCommitted() const;
    bool ChasePanicActive() const;
    bool AlertAudioActive() const;
    void BeginRecognition(float duration);
    void CancelRecognition();
    void AdvanceRecognition(float dt);
    void BeginCuriosity(float duration);
    void AdvanceCuriosity(float dt);
    void ClearCuriosity();
    void CommitChaseRecognition();
    void ReleaseChaseRecognition();
    void HoldChaseMemory(float holdSeconds);
    void DecayChaseMemory(float dt);
    void ClearChasePressure();
    void ClearChaseCommitment();
    void ClearPursuitState();
    void ResetSessionState(const XMFLOAT3& startPosition, float startYaw, float initialRoamTimer);
    MonsterSnapshotState CaptureSnapshotState() const;
    void RestoreSnapshotState(MonsterSnapshotState snapshot);
};
