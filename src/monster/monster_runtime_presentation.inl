// Monster runtime presentation state helpers. 
// Included inside Renderer's private section from monster_ai.inl.

    float MonsterHeadBobOffset() const {
        float phase = monsterPreview_.active ? timeRuntime_.time * 2.35f : monsterPresentation_.headBobPhase;
        float chase = monsterPreview_.active ? 0.0f : monsterPresentation_.headChaseBlend;
        float amplitude = Lerp(0.050f, 0.086f, chase);
        float secondary = Lerp(0.006f, 0.014f, chase);
        return std::sin(phase) * amplitude + std::sin(phase * 2.0f + 0.65f) * secondary;
    }

    void ResetMonsterPresentationState(bool clearTrail, bool clearHandprints, bool resetHead = true) {
        if (clearTrail) monsterPresentation_.trail.clear();
        monsterPresentation_.limbAnchors.clear();
        monsterPresentation_.smoothedBodyPoints.clear();
        monsterPresentation_.smoothedBodyUps.clear();
        monsterPresentation_.bodySmoothTime = -1000.0f;
        monsterPresentation_.renderVisibleUntil = -1000.0f;
        if (clearHandprints) monsterPresentation_.handprints.clear();
        if (resetHead) {
            monsterPresentation_.headChaseBlend = 0.0f;
            monsterPresentation_.headLockAmount = 0.0f;
        }
    }

    void PrimeMonsterTrail(float spacing, int pointCount = 96) {
        monsterPresentation_.trail.clear();
        pointCount = std::max(0, pointCount);
        for (int i = 0; i < pointCount; ++i) {
            float back = static_cast<float>(i) * spacing;
            monsterPresentation_.trail.push_back({
                gameWorld_.monster.position.x - std::sin(gameWorld_.monster.yaw) * back,
                0.0f,
                gameWorld_.monster.position.z - std::cos(gameWorld_.monster.yaw) * back
            });
        }
    }

    void RecordMonsterTrailPoint(const XMFLOAT3& p) {
        XMFLOAT3 head{p.x, 0.0f, p.z};
        float minStep = gameWorld_.maze.TileMinimum() * 0.040f;
        if (!monsterPresentation_.trail.empty()) {
            float dx = head.x - monsterPresentation_.trail.front().x;
            float dz = head.z - monsterPresentation_.trail.front().z;
            if (dx * dx + dz * dz < minStep * minStep) return;
        }
        monsterPresentation_.trail.insert(monsterPresentation_.trail.begin(), head);
        if (monsterPresentation_.trail.size() > 256) monsterPresentation_.trail.resize(256);
    }

    XMFLOAT3 MonsterSelfAvoidanceVector(const XMFLOAT3& candidate) const {
        if (monsterPresentation_.trail.size() < 10) return {0.0f, 0.0f, 0.0f};
        float spacing = MonsterBodySpacing();
        float ignoreDistance = std::max(gameWorld_.maze.TileMinimum() * 0.82f, spacing * 4.0f);
        float bodyLength = MonsterBodyLengthMeters();
        float repelRadius = std::max(gameWorld_.maze.TileMinimum() * 0.34f, settingsRuntime_.live.monsterScale * 0.62f);
        float repelRadiusSq = repelRadius * repelRadius;
        XMFLOAT3 repel{0.0f, 0.0f, 0.0f};
        for (float d = ignoreDistance; d <= bodyLength; d += spacing * 1.35f) {
            XMFLOAT3 sample = MonsterTrailSample(d);
            float dx = candidate.x - sample.x;
            float dz = candidate.z - sample.z;
            float distSq = dx * dx + dz * dz;
            if (distSq <= 0.0001f || distSq > repelRadiusSq) continue;
            float dist = std::sqrt(distSq);
            float weight = 1.0f - dist / repelRadius;
            weight *= weight;
            repel.x += (dx / dist) * weight;
            repel.z += (dz / dist) * weight;
        }
        return repel;
    }

    void UpdateMonsterHeadAnimation(float dt, bool seesPlayer) {
        dt = std::clamp(dt, 0.0f, 0.10f);
        Tile mt = MonsterTile();
        bool validTile = ValidMonsterTile(mt);
        bool openScanSpace = validTile &&
            (gameWorld_.maze.OpenNeighborCount(mt) >= 3 || gameWorld_.maze.LocalOpenCount(mt, 2) >= 14);
        bool activeChase = seesPlayer || gameWorld_.monster.AlertAudioActive();
        float curiosity = MonsterCuriosityAmount();

        gameWorld_.monster.canSeePlayerNow = seesPlayer;
        monsterPresentation_.headChaseBlend += ((activeChase ? 1.0f : 0.0f) - monsterPresentation_.headChaseBlend) *
            std::min(1.0f, dt * (activeChase ? 2.35f : 1.00f));
        monsterPresentation_.headLockAmount += ((seesPlayer ? 1.0f : 0.0f) - monsterPresentation_.headLockAmount) *
            std::min(1.0f, dt * (seesPlayer ? 1.80f : 1.15f));

        float bobSpeed = Lerp(2.10f, 5.65f, monsterPresentation_.headChaseBlend);
        monsterPresentation_.headBobPhase += dt * bobSpeed;
        if (monsterPresentation_.headBobPhase > kPi * 64.0f) {
            monsterPresentation_.headBobPhase = std::fmod(monsterPresentation_.headBobPhase, kPi * 2.0f);
        }

        float scanRate = seesPlayer ? 0.0f : (openScanSpace ? 0.54f : 0.22f);
        monsterPresentation_.headScanPhase += dt * scanRate;
        if (monsterPresentation_.headScanPhase > kPi * 64.0f) {
            monsterPresentation_.headScanPhase = std::fmod(monsterPresentation_.headScanPhase, kPi * 2.0f);
        }

        float yawRange = openScanSpace ? 0.62f : 0.25f;
        float scan = monsterPresentation_.headScanPhase;
        float glideYaw = std::sin(scan) * yawRange +
            std::sin(scan * 0.43f + 1.1f) * yawRange * 0.24f;
        float glidePitch = std::sin(scan * 0.71f + 0.4f) * (openScanSpace ? 0.095f : 0.040f) - 0.024f;
        float focusWindow = std::pow(std::max(0.0f, std::sin(scan * 0.38f + 2.2f)), 18.0f);
        float focusSide = Rand01(static_cast<int>(std::floor(scan * 0.19f)) + mt.x * 11, mt.y * 17 + 991, sessionRuntime_.runtimeSeed) > 0.5f ? 1.0f : -1.0f;
        float focusYaw = yawRange * focusSide * (0.72f + Rand01(mt.x + 103, mt.y + 211, sessionRuntime_.runtimeSeed) * 0.42f);
        float focusPitch = -0.045f + Rand01(mt.x + 307, mt.y + 409, sessionRuntime_.runtimeSeed) * 0.12f;
        float jitterBurst = std::pow(std::max(0.0f, std::sin(scan * 2.1f + 0.7f)), 26.0f);
        float microJitter = (std::sin(timeRuntime_.time * 8.0f + gameWorld_.monster.position.x * 0.7f) * 0.010f +
            std::sin(timeRuntime_.time * 13.0f + gameWorld_.monster.position.z * 0.5f) * 0.005f) * jitterBurst;
        float curiosityYaw = std::sin(timeRuntime_.time * 3.4f) * 0.16f + std::sin(timeRuntime_.time * 5.1f + 0.7f) * 0.045f;
        float curiosityPitch = -0.10f + std::sin(timeRuntime_.time * 2.2f + 0.3f) * 0.025f;
        float targetYaw = seesPlayer ? Lerp(0.0f, curiosityYaw, curiosity) : Lerp(glideYaw, focusYaw, focusWindow) + microJitter;
        float targetPitch = seesPlayer ? Lerp(0.0f, curiosityPitch, curiosity) : Lerp(glidePitch, focusPitch, focusWindow) +
            std::sin(timeRuntime_.time * 23.0f) * 0.008f * jitterBurst;
        float response = seesPlayer ? Lerp(1.70f, 2.65f, curiosity) : (openScanSpace ? Lerp(0.58f, 2.80f, focusWindow + jitterBurst * 0.18f) : Lerp(0.48f, 2.35f, focusWindow));
        monsterPresentation_.headYawOffset += AngleWrap(targetYaw - monsterPresentation_.headYawOffset) *
            std::min(1.0f, dt * response);
        monsterPresentation_.headPitchOffset += (targetPitch - monsterPresentation_.headPitchOffset) *
            std::min(1.0f, dt * response);
    }
