// Monster focus-point and preview-camera helpers. 
// Included inside Renderer's private section from player_monster_pressure.inl.

    float MonsterDistance() const {
        if (!MonsterActiveForCurrentMode()) return std::numeric_limits<float>::max();
        GameWorldRenderSnapshot world = gameWorld_.BuildRenderSnapshot();
        float dx = world.monsterPosition.x - world.playerPosition.x;
        float dz = world.monsterPosition.z - world.playerPosition.z;
        return std::sqrt(dx * dx + dz * dz);
    }

    XMFLOAT3 MonsterFocusPoint() const {
        GameWorldRenderSnapshot world = gameWorld_.BuildRenderSnapshot();
        float modelY = std::clamp(settingsRuntime_.live.monsterScale, 0.35f, 1.25f);
        float modelXZ = std::clamp(settingsRuntime_.live.monsterScale, 0.35f, 1.35f);
        float hover = 0.22f + std::sin(timeRuntime_.time * 1.55f + world.monsterPosition.x * 0.07f + world.monsterPosition.z * 0.05f) * 0.050f;
        bool canTrackPlayer = !monsterPreview_.active && MonsterVisualEncounterPlayer();
        float faceYaw = world.monsterYaw;
        if (canTrackPlayer) {
            float cameraYaw = std::atan2(world.playerPosition.x - world.monsterPosition.x, world.playerPosition.z - world.monsterPosition.z);
            faceYaw += AngleWrap(cameraYaw - faceYaw) * 0.22f;
        }

        float deathHeadLock = world.deathActive ? SmoothStep(0.0f, 0.22f, world.deathTimer) : 0.0f;
        float twitch = std::sin(timeRuntime_.time * 13.7f + world.monsterPosition.x * 0.3f) * 0.035f;
        float liveLock = canTrackPlayer ? monsterPresentation_.headLockAmount : 0.0f;
        float headLock = std::max(deathHeadLock, liveLock);
        float scanWeight = 1.0f - SmoothStep(0.0f, 1.0f, Clamp01(headLock));
        float headYaw = faceYaw + monsterPresentation_.headYawOffset * 0.24f * scanWeight +
            twitch * 0.22f * (1.0f - deathHeadLock * 0.85f) * scanWeight;
        XMFLOAT3 hRight{std::cos(headYaw), 0.0f, -std::sin(headYaw)};
        XMFLOAT3 hUp{0.0f, 1.0f, 0.0f};
        XMFLOAT3 hForward{std::sin(headYaw), 0.0f, std::cos(headYaw)};
        XMFLOAT3 skull = Add3(world.monsterPosition, OrientedOffset(hRight, hUp, hForward,
            0.0f, (1.34f + MonsterHeadBobOffset() * 0.55f) * modelY + hover, kMonsterHeadForwardOffset * 0.56f * modelXZ));

        float headPitch = monsterPresentation_.headPitchOffset * 0.32f * scanWeight;
        if (std::abs(headPitch) > 0.0005f) {
            hForward = Normalize3(Add3(Scale3(hForward, std::cos(headPitch)), Scale3(hUp, std::sin(headPitch))), hForward);
            hUp = Normalize3(Cross3(hForward, hRight), hUp);
        }

        if (headLock > 0.001f) {
            XMFLOAT3 cameraFocus{world.playerPosition.x, world.playerPosition.y + 0.04f, world.playerPosition.z};
            XMFLOAT3 lookForward = Normalize3(Sub3(cameraFocus, skull), hForward);
            float trackBlend = SmoothStep(0.0f, 1.0f, SmoothStep(0.0f, 1.0f, Clamp01(headLock)));
            hForward = Normalize3(Lerp3(hForward, lookForward, trackBlend * 0.22f), lookForward);
            hRight = Normalize3(Cross3(hUp, hForward), hRight);
            hUp = Normalize3(Cross3(hForward, hRight), hUp);
        }

        return Add3(skull, OrientedOffset(hRight, hUp, hForward,
            0.0f, 0.118f * modelY, 0.224f * modelXZ));
    }

    void SetMonsterPreviewCamera(float orbitSeconds = 0.0f) {
        if (!monsterPreview_.active) return;
        XMFLOAT3 target = MonsterFocusPoint();
        XMFLOAT3 cameraPosition{};
        if (monsterPreview_.view != MonsterPreviewView::Top && monsterPreview_.manualOrbit) {
            float cp = std::cos(monsterPreview_.orbitPitch);
            cameraPosition = {
                target.x + std::sin(monsterPreview_.orbitYaw) * cp * monsterPreview_.orbitDistance,
                target.y + std::sin(monsterPreview_.orbitPitch) * monsterPreview_.orbitDistance,
                target.z - std::cos(monsterPreview_.orbitYaw) * cp * monsterPreview_.orbitDistance
            };
        } else if (monsterPreview_.view == MonsterPreviewView::Front) {
            cameraPosition = {target.x, target.y - 0.48f, target.z - 3.25f};
        } else if (monsterPreview_.view == MonsterPreviewView::Side) {
            cameraPosition = {target.x + 3.25f, target.y - 0.48f, target.z};
        } else if (monsterPreview_.view == MonsterPreviewView::LeftSide) {
            cameraPosition = {target.x - 3.25f, target.y - 0.48f, target.z};
        } else if (monsterPreview_.view == MonsterPreviewView::Top) {
            cameraPosition = {target.x, target.y + 4.60f, target.z};
        } else {
            float orbit = orbitSeconds * 0.78f;
            cameraPosition = {
                target.x + std::sin(orbit) * 3.15f,
                target.y - 0.52f,
                target.z - std::cos(orbit) * 3.15f
            };
        }
        float dx = target.x - cameraPosition.x;
        float dy = target.y - cameraPosition.y;
        float dz = target.z - cameraPosition.z;
        float yaw = std::atan2(dx, dz);
        float horizontal = std::sqrt(dx * dx + dz * dz);
        float pitch = monsterPreview_.view == MonsterPreviewView::Top
            ? -20.0f
            : std::clamp(std::atan2(dy, std::max(0.001f, horizontal)), -0.36f, 0.48f);
        GameWorldRenderSnapshot world = gameWorld_.BuildRenderSnapshot();
        gameWorld_.SetPlayerCameraPose(cameraPosition, yaw, world.monsterYaw, pitch);
        viewRuntime_.flashlightYaw = yaw;
        viewRuntime_.flashlightPitch = pitch;
    }
