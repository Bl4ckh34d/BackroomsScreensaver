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
