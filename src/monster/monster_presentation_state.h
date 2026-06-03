#pragma once

struct MonsterPresentationSnapshotState {
    std::vector<XMFLOAT3> trail;
    std::vector<MonsterLimbAnchor> limbAnchors;
    std::vector<MonsterHandprint> handprints;
    float headBobPhase = 0.0f;
    float headScanPhase = 0.0f;
    float headYawOffset = 0.0f;
    float headPitchOffset = 0.0f;
    float headLockAmount = 0.0f;
    float headChaseBlend = 0.0f;
};

struct MonsterPresentationRuntimeState {
    std::vector<XMFLOAT3> trail;
    std::vector<MonsterLimbAnchor> limbAnchors;
    std::vector<XMFLOAT3> smoothedBodyPoints;
    std::vector<XMFLOAT3> smoothedBodyUps;
    float bodySmoothTime = -1000.0f;
    float renderVisibleUntil = -1000.0f;
    std::vector<MonsterHandprint> handprints;
    float headBobPhase = 0.0f;
    float headScanPhase = 0.0f;
    float headYawOffset = 0.0f;
    float headPitchOffset = 0.0f;
    float headLockAmount = 0.0f;
    float headChaseBlend = 0.0f;

    MonsterPresentationSnapshotState CaptureSnapshot() const {
        MonsterPresentationSnapshotState snapshot{};
        snapshot.trail = trail;
        snapshot.limbAnchors = limbAnchors;
        snapshot.handprints = handprints;
        snapshot.headBobPhase = headBobPhase;
        snapshot.headScanPhase = headScanPhase;
        snapshot.headYawOffset = headYawOffset;
        snapshot.headPitchOffset = headPitchOffset;
        snapshot.headLockAmount = headLockAmount;
        snapshot.headChaseBlend = headChaseBlend;
        return snapshot;
    }

    void RestoreSnapshot(MonsterPresentationSnapshotState snapshot) {
        trail = std::move(snapshot.trail);
        limbAnchors = std::move(snapshot.limbAnchors);
        handprints = std::move(snapshot.handprints);
        headBobPhase = snapshot.headBobPhase;
        headScanPhase = snapshot.headScanPhase;
        headYawOffset = snapshot.headYawOffset;
        headPitchOffset = snapshot.headPitchOffset;
        headLockAmount = snapshot.headLockAmount;
        headChaseBlend = snapshot.headChaseBlend;
    }
};
