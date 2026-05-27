// Player/camera movement, navigation, attention, chase, and camera-state helpers.
// Included inside Renderer's private section from main.cpp.

    void ResetSimulation() {
        XMFLOAT3 start = maze_.WorldCenter(maze_.start, 1.45f);
        camera_ = start;
        effectAnimationStartTime_ = time_;
        yaw_ = 0.0f;
        bodyYaw_ = 0.0f;
        turnLookBlend_ = 0.0f;
        turnLookYaw_ = yaw_;
        lookPitch_ = -0.055f;
        stepPhase_ = 0.0f;
        headScanTimer_ = 0.0f;
        headScanDuration_ = 0.0f;
        headScanCenter_ = 0.0f;
        stopTimer_ = 0.0f;
        nextLookBackTime_ = 8.0f + RandRange(settings_.lookBackMinSeconds * 0.25f, settings_.lookBackMinSeconds * 0.6f);
        lookBack_ = false;
        junctionScanActive_ = false;
        junctionScanCount_ = 0;
        junctionScanTile_ = {-1000, -1000};
        branchLookTimer_ = 0.0f;
        branchLookDuration_ = 0.0f;
        branchLookYaw_ = yaw_;
        branchLookPitch_ = -0.045f;
        branchLookCooldown_ = RandRange(0.65f, 1.45f);
        branchLookPaused_ = false;
        lastBranchLookTile_ = {-1000, -1000};
        roomSurveyTimer_ = 0.0f;
        roomSurveyDuration_ = 0.0f;
        roomSurveyYawCount_ = 0;
        roomSurveyPitchCount_ = 0;
        roomSurveyCooldown_ = RandRange(1.2f, 2.8f);
        lastTile_ = maze_.start;
        previousTile_ = {-1000, -1000};
        path_.clear();
        pathIndex_ = 0;
        visitedTiles_.assign(static_cast<size_t>(std::max(0, maze_.w * maze_.h)), 0);
        MarkVisited(maze_.start);
        airParticles_.clear();
        airParticles_.reserve(22000);
        airParticleBudgetScale_ = 1.0f;
        airParticleFrameDt_ = 0.0f;
        propLookTimer_ = 0.0f;
        propLookDuration_ = 0.0f;
        propLookCooldown_ = RandRange(0.7f, 2.0f);
        propLookScanSeed_ = RandRange(0.0f, 1.0f);
        lastPropLookTarget_ = {};
        hasLastPropLookTarget_ = false;
        sparks_.clear();
        sparkFlashes_.clear();
        sparkChains_.clear();
        for (RuntimeLampState& lamp : runtimeLamps_) {
            lamp.damage = 0.0f;
            lamp.sparkTimer = RandRange(0.08f, 0.72f);
            lamp.broken = false;
        }
        if (!lampDamagePixels_.empty()) {
            std::fill(lampDamagePixels_.begin(), lampDamagePixels_.end(), 0);
            lampDamageDirty_ = true;
        }
        steam_.clear();
        ventDrops_.clear();
        bloodScarePoints_.clear();
        bloodRevealRegions_.clear();
        activeBloodScareIndex_ = -1;
        bloodScareActiveUntil_ = 0.0f;
        bloodWorldFlickerTimer_ = 0.0f;
        bloodWorldFlickerDuration_ = settings_.bloodWorldFlickerDuration;
        bloodWorldActivationTime_ = -1000.0f;
        bloodFocusTimer_ = 0.0f;
        bloodFocusDuration_ = 0.0f;
        bloodFocusReactionsTaken_ = 0;
        bloodFocusReactionCooldown_ = 0.0f;
        proximityBloodPulseCooldown_ = RandRange(4.5f, 9.0f);
        bloodFocusTarget_ = {};
        float scareScale = ScareCooldownScale();
        sparkCooldown_ = settings_.sparkParticles
            ? RandRange(settings_.sparkBurstMinSeconds, settings_.sparkBurstMaxSeconds) * AmbientSparkCooldownScale()
            : 1000000.0f;
        scareCooldown_ = RandRange(7.0f, 15.0f) * scareScale;
        scareEventTile_ = CameraTile();
        fleshFlickerTimer_ = 0.0f;
        fleshFlickerDuration_ = settings_.fleshFlickerDuration;
        fleshFlickerCooldown_ = settings_.fleshFlicker
            ? RandRange(settings_.fleshFlickerMinSeconds * 0.90f, settings_.fleshFlickerMaxSeconds) * scareScale
            : 1000000.0f;
        bloodWorldFlickerCooldown_ = (settings_.bloodWorldFlicker && settings_.bloodWorldCoverage > 0.001f)
            ? RandRange(settings_.bloodWorldFlickerMinSeconds * 0.90f, settings_.bloodWorldFlickerMaxSeconds) * scareScale
            : 1000000.0f;
        secondsSinceLookBack_ = 0.0f;
        exitSpotted_ = false;
        exitLookBlend_ = 0.0f;
        exitLookFocus_ = {};
        threatRepath_ = 0.0f;
        dangerLevel_ = 0.0f;
        dreadLevel_ = 0.0f;
        dreadMeterLevel_ = 0.0f;
        chaseMemoryTimer_ = 0.0f;
        chasePanic_ = 0.0f;
        monsterRunLaunchMeters_ = 3.0f;
        monsterRunLaunchActive_ = false;
        smoothedMoveSpeed_ = settings_.walkSpeed;
        runIntensity_ = 0.0f;
        runEffort_ = 0.0f;
        threatVisibleLast_ = false;
        monsterSpottedLast_ = false;
        monsterSightDreadCooldown_ = 0.0f;
        chaseLookBackTimer_ = 0.0f;
        chaseLookBackDuration_ = 0.0f;
        chaseLookBackCooldown_ = RandRange(0.55f, 1.25f);
        chaseLookBackYaw_ = yaw_;
        chaseLookBackPitch_ = lookPitch_;
        stumbleTimer_ = 0.0f;
        stumbleDuration_ = 0.0f;
        stumbleYawOffset_ = 0.0f;
        flashlightYaw_ = yaw_;
        flashlightPitch_ = lookPitch_;
        previousCameraYaw_ = yaw_;
        previousCameraPitch_ = lookPitch_;
        cameraMotionBlur_ = {};
        flashlightAgitation_ = 0.0f;
        flashlightDartTimer_ = 0.0f;
        flashlightDartDuration_ = 0.0f;
        flashlightDartCooldown_ = RandRange(1.00f, 2.60f);
        flashlightDartYaw_ = 0.0f;
        flashlightDartPitch_ = 0.0f;
        flashlightSnapTimer_ = 0.0f;
        flashlightSnapDuration_ = 0.0f;
        flashlightSnapCooldown_ = RandRange(0.45f, 1.35f);
        flashlightSnapYaw_ = 0.0f;
        flashlightSnapPitch_ = 0.0f;
        flashlightSnapSharp_ = false;
        flashlightHoldYaw_ = 0.0f;
        flashlightHoldPitch_ = 0.0f;
        ventReactionTimer_ = 0.0f;
        ventReactionDuration_ = 0.0f;
        ventReactionLookDelay_ = 0.0f;
        ventReactionBackDuration_ = 0.0f;
        ventReactionScanSeed_ = 0.0f;
        ventReactionTarget_ = {};
        ventReactionAway_ = {0.0f, 0.0f, 1.0f};
        exitTransitionActive_ = false;
        exitTransitionTimer_ = 0.0f;
        fadeInTimer_ = settings_.fadeInSeconds;
        exitDoorAngle_ = 0.0f;
        deathActive_ = false;
        deathTimer_ = 0.0f;
        playerHealth_ = 100.0f;
        playerStamina_ = 100.0f;
        playerVerticalOffset_ = 0.0f;
        playerVerticalVelocity_ = 0.0f;
        playerStaminaRegenDelay_ = 0.0f;
        playerGrounded_ = true;
        monster_ = maze_.WorldCenter(maze_.exit, 0.0f);
        monsterPath_.clear();
        monsterPathIndex_ = 0;
        monsterRepath_ = 0.0f;
        monsterYaw_ = 0.0f;
        monsterGoal_ = {-1000, -1000};
        monsterSoundTile_ = {-1000, -1000};
        monsterLastKnownTile_ = {-1000, -1000};
        monsterRoamTile_ = {-1000, -1000};
        monsterHasSound_ = false;
        monsterHasLastKnown_ = false;
        monsterChasingVisible_ = false;
        monsterSearchTimer_ = 0.0f;
        monsterRoamTimer_ = RandRange(0.8f, 2.4f);
        monsterRecognitionActive_ = false;
        monsterRecognizedForChase_ = false;
        monsterRecognitionTimer_ = 0.0f;
        monsterRecognitionDuration_ = 0.0f;
        monsterHeadBobPhase_ = RandRange(0.0f, kPi * 2.0f);
        monsterHeadScanPhase_ = RandRange(0.0f, kPi * 2.0f);
        monsterHeadYawOffset_ = 0.0f;
        monsterHeadPitchOffset_ = 0.0f;
        monsterHeadLockAmount_ = 0.0f;
        monsterHeadChaseBlend_ = 0.0f;
        monsterCanSeePlayerNow_ = false;
        if (gEffectDebugViewer) {
            ApplyDebugSliceCamera();
            fadeInTimer_ = 0.0f;
            path_.clear();
            pathIndex_ = 0;
        } else if (gBloodDebugEveryWall) {
            ApplyBloodDebugCamera();
            fadeInTimer_ = 0.0f;
            path_.clear();
            pathIndex_ = 0;
        }
        if (settings_.bloodStudyView) {
            ApplyBloodStudyCamera();
            fadeInTimer_ = 0.0f;
        }
        if (monsterPreview_) {
            monster_ = {0.0f, 0.0f, 0.0f};
            monsterYaw_ = kPi;
            SetMonsterPreviewCamera(0.0f);
            dangerLevel_ = 0.45f;
            dreadLevel_ = 0.35f;
            path_.clear();
            pathIndex_ = 0;
        }
    }

    void ApplyBloodDebugCamera() {
        if (gEffectDebugViewer) {
            ApplyDebugSliceCamera();
            return;
        }
        Tile t{std::min(3, std::max(1, maze_.w - 3)), maze_.start.y};
        if (!maze_.IsOpen(t.x, t.y)) t = maze_.start;
        XMFLOAT3 c = maze_.WorldCenter(t, 0.0f);
        yaw_ = kPi * 0.5f;
        bodyYaw_ = yaw_;
        camera_ = {c.x - maze_.tileW * 0.34f, 1.36f, c.z};
        lookPitch_ = -0.045f;
        flashlightYaw_ = yaw_;
        flashlightPitch_ = lookPitch_;
        previousCameraYaw_ = yaw_;
        previousCameraPitch_ = lookPitch_;
    }

    void ApplyDebugSliceCamera() {
        int tiles = std::clamp(gDebugSliceTiles, 1, 5);
        float ox = -static_cast<float>(maze_.w) * maze_.tileW * 0.5f;
        float oz = -static_cast<float>(maze_.h) * maze_.tileD * 0.5f;
        float centerX = ox + (1.0f + static_cast<float>(tiles) * 0.5f) * maze_.tileW;
        float centerZ = oz + (1.0f + static_cast<float>(tiles) * 0.5f) * maze_.tileD;
        float southInsideZ = oz + (static_cast<float>(tiles) + 0.72f) * maze_.tileD;
        southInsideZ = std::min(southInsideZ, oz + (static_cast<float>(maze_.h) - 1.12f) * maze_.tileD);

        camera_ = {centerX, 1.42f, southInsideZ};
        XMFLOAT3 target{centerX, settings_.wallHeightMeters * 0.46f, centerZ - maze_.tileD * 0.30f};
        if (gDebugSliceEffect == DebugSliceEffect::FloorWater) {
            camera_.y = 1.68f;
            target = {centerX, 0.08f, centerZ};
        } else if (gDebugSliceEffect == DebugSliceEffect::CeilingWater ||
                   gDebugSliceEffect == DebugSliceEffect::CeilingLamps ||
                   gDebugSliceEffect == DebugSliceEffect::BrokenLamps) {
            camera_.y = 1.12f;
            target = {centerX, settings_.wallHeightMeters - 0.10f, centerZ};
        } else if (gDebugSliceEffect == DebugSliceEffect::WallWater ||
                   gDebugSliceEffect == DebugSliceEffect::AirVents) {
            camera_.y = 1.36f;
            target = {centerX, settings_.wallHeightMeters * 0.56f, oz + maze_.tileD + 0.020f};
        } else if (gDebugSliceEffect == DebugSliceEffect::Blood) {
            camera_.y = 1.34f;
            target = {centerX, settings_.wallHeightMeters * 0.52f, oz + maze_.tileD + 0.025f};
        } else if (gDebugSliceEffect == DebugSliceEffect::Props) {
            float distanceScale = DebugPropCameraDistanceScale(gDebugPropIndex);
            float requestedZ = centerZ + maze_.tileD * distanceScale;
            float safeSouthZ = southInsideZ - maze_.tileD * 0.08f;
            camera_ = {centerX, 1.18f, std::min(requestedZ, safeSouthZ)};
            target = {centerX, DebugPropCameraTargetY(gDebugPropIndex), centerZ};
        }

        yaw_ = YawToPoint(target);
        bodyYaw_ = yaw_;
        lookPitch_ = std::clamp(PitchToPoint(target), -0.62f, 0.62f);
        flashlightYaw_ = yaw_;
        flashlightPitch_ = lookPitch_;
        previousCameraYaw_ = yaw_;
        previousCameraPitch_ = lookPitch_;
    }

    void RestartMaze() {
        if (gEffectDebugViewer) {
            ApplyDebugSliceSettings();
            maze_.w = settings_.mazeWidth;
            maze_.h = settings_.mazeHeight;
            maze_.GenerateDebugSlice(gDebugSliceTiles);
        } else if (gBloodDebugEveryWall) {
            maze_.GenerateBloodDebugCorridor();
        } else {
            maze_.w = settings_.mazeWidth;
            maze_.h = settings_.mazeHeight;
            maze_.tileW = settings_.tileWidthMeters;
            maze_.tileD = settings_.tileLengthMeters;
            maze_.exit = {maze_.w - 2, maze_.h - 2};
            maze_.Generate(settings_);
        }
        CreateMazeMaskTexture();
        ResetSimulation();
        CreateMazeMesh();
    }

    Tile FindBloodStudyTile() const {
        auto hasWallWithOpenBack = [&](Tile t) {
            if (!maze_.IsOpen(t.x, t.y)) return false;
            return (!maze_.IsOpen(t.x, t.y - 1) && maze_.IsOpen(t.x, t.y + 1)) ||
                (!maze_.IsOpen(t.x, t.y + 1) && maze_.IsOpen(t.x, t.y - 1)) ||
                (!maze_.IsOpen(t.x - 1, t.y) && maze_.IsOpen(t.x + 1, t.y)) ||
                (!maze_.IsOpen(t.x + 1, t.y) && maze_.IsOpen(t.x - 1, t.y));
        };
        for (int y = 1; y < maze_.h - 1; ++y) {
            for (int x = 1; x < maze_.w - 1; ++x) {
                Tile t{x, y};
                if (hasWallWithOpenBack(t)) return t;
            }
        }
        for (int y = 0; y < maze_.h; ++y) {
            for (int x = 0; x < maze_.w; ++x) {
                if (maze_.IsOpen(x, y)) return {x, y};
            }
        }
        return maze_.start;
    }

    void ApplyBloodStudyCamera() {
        bloodStudyTile_ = FindBloodStudyTile();
        XMFLOAT3 c = maze_.WorldCenter(bloodStudyTile_, 0.0f);
        int side = 0;
        struct StudySide { int side; Tile wall; Tile back; };
        StudySide sides[] = {
            {0, {bloodStudyTile_.x, bloodStudyTile_.y - 1}, {bloodStudyTile_.x, bloodStudyTile_.y + 1}},
            {1, {bloodStudyTile_.x, bloodStudyTile_.y + 1}, {bloodStudyTile_.x, bloodStudyTile_.y - 1}},
            {2, {bloodStudyTile_.x - 1, bloodStudyTile_.y}, {bloodStudyTile_.x + 1, bloodStudyTile_.y}},
            {3, {bloodStudyTile_.x + 1, bloodStudyTile_.y}, {bloodStudyTile_.x - 1, bloodStudyTile_.y}}
        };
        for (const StudySide& candidate : sides) {
            if (!maze_.IsOpen(candidate.wall.x, candidate.wall.y) && maze_.IsOpen(candidate.back.x, candidate.back.y)) {
                side = candidate.side;
                break;
            }
        }
        XMFLOAT3 forward{0.0f, 0.0f, -1.0f};
        if (side == 0) {
            yaw_ = kPi;
            forward = {0.0f, 0.0f, -1.0f};
        } else if (side == 1) {
            yaw_ = 0.0f;
            forward = {0.0f, 0.0f, 1.0f};
        } else if (side == 2) {
            yaw_ = -kPi * 0.5f;
            forward = {-1.0f, 0.0f, 0.0f};
        } else {
            yaw_ = kPi * 0.5f;
            forward = {1.0f, 0.0f, 0.0f};
        }
        float axisLength = (side == 0 || side == 1) ? maze_.tileD : maze_.tileW;
        camera_ = Add3({c.x, 1.08f, c.z}, Scale3(forward, -axisLength * 0.86f));
        bodyYaw_ = yaw_;
        XMFLOAT3 wallTarget = Add3({c.x, settings_.wallHeightMeters * 0.52f, c.z}, Scale3(forward, axisLength * 0.50f));
        lookPitch_ = std::clamp(PitchToPoint(wallTarget), -0.10f, 0.18f);
        flashlightYaw_ = yaw_;
        flashlightPitch_ = lookPitch_;
    }

    Tile CameraTile() const {
        return maze_.TileFromWorld(camera_.x, camera_.z);
    }

    Tile MonsterTile() const {
        return maze_.TileFromWorld(monster_.x, monster_.z);
    }

    size_t TileIndex(Tile t) const {
        return static_cast<size_t>(t.y * maze_.w + t.x);
    }

    uint16_t VisitCount(Tile t) const {
        if (!maze_.IsOpen(t.x, t.y)) return 0;
        size_t index = TileIndex(t);
        if (index >= visitedTiles_.size()) return 0;
        return visitedTiles_[index];
    }

    void MarkVisited(Tile t) {
        if (!maze_.IsOpen(t.x, t.y)) return;
        size_t index = TileIndex(t);
        if (index >= visitedTiles_.size()) return;
        if (visitedTiles_[index] < 65535) ++visitedTiles_[index];
    }

    float RandRange(float a, float b) {
        std::uniform_real_distribution<float> dist(a, b);
        return dist(rng_);
    }

    float RandEffectRange(float a, float b) {
        if (b < a) std::swap(a, b);
        return RandRange(a, b);
    }

    int RandEffectRangeInt(int a, int b) {
        if (b < a) std::swap(a, b);
        if (b <= a) return a;
        int value = a + static_cast<int>(RandRange(0.0f, static_cast<float>(b - a + 1)));
        return std::clamp(value, a, b);
    }

    float PickBrokenLampSparkIntensity() {
        return RandEffectRange(settings_.effectBrokenLampSparkIntensityMin, settings_.effectBrokenLampSparkIntensityMax);
    }

    int PickBrokenLampChainBursts() {
        return RandEffectRangeInt(settings_.effectBrokenLampChainBurstsMin, settings_.effectBrokenLampChainBurstsMax);
    }

    float PickAirVentSteamIntensity() {
        return RandEffectRange(settings_.effectAirVentSteamIntensityMin, settings_.effectAirVentSteamIntensityMax);
    }

    float JumpscareFrequency() const {
        return Clamp01(settings_.jumpscareFrequency);
    }

    float ScareCooldownScale() const {
        return 1.0f / std::max(0.08f, JumpscareFrequency());
    }

    float AmbientSparkCooldownScale() const {
        return Lerp(4.0f, 1.0f, JumpscareFrequency());
    }

    void AdvanceStepPhase(float metersMoved, float speedMetersPerSecond) {
        if (metersMoved <= 0.0001f || speedMetersPerSecond <= 0.0001f) return;
        float runBlend = Clamp01((speedMetersPerSecond - settings_.walkSpeed) / std::max(0.1f, settings_.runSpeed * 1.55f - settings_.walkSpeed));
        float strideMeters = Lerp(0.82f, 0.62f, runBlend);
        stepPhase_ += metersMoved * (kPi / std::max(0.25f, strideMeters));
        if (stepPhase_ > kPi * 128.0f) {
            stepPhase_ = std::fmod(stepPhase_, kPi * 2.0f);
        }
    }

    float MonsterDistance() const {
        float dx = monster_.x - camera_.x;
        float dz = monster_.z - camera_.z;
        return std::sqrt(dx * dx + dz * dz);
    }

    XMFLOAT3 MonsterEyeFocus() const {
        float modelY = std::clamp(settings_.monsterScale, 0.35f, 1.25f);
        float modelXZ = std::clamp(settings_.monsterScale, 0.35f, 1.35f);
        float hover = 0.22f + std::sin(time_ * 1.55f + monster_.x * 0.07f + monster_.z * 0.05f) * 0.050f;
        bool canTrackPlayer = !monsterPreview_ && MonsterLineOfSightToPlayer();
        float faceYaw = monsterYaw_;
        if (canTrackPlayer) {
            float cameraYaw = std::atan2(camera_.x - monster_.x, camera_.z - monster_.z);
            faceYaw += AngleWrap(cameraYaw - faceYaw) * 0.42f;
        }

        float deathHeadLock = deathActive_ ? SmoothStep(0.0f, 0.22f, deathTimer_) : 0.0f;
        float twitch = std::sin(time_ * 13.7f + monster_.x * 0.3f) * 0.035f;
        float liveLock = canTrackPlayer ? monsterHeadLockAmount_ : 0.0f;
        float headLock = std::max(deathHeadLock, liveLock);
        float scanWeight = 1.0f - Clamp01(headLock);
        float headYaw = faceYaw + monsterHeadYawOffset_ * scanWeight +
            twitch * (1.0f - deathHeadLock * 0.85f) * scanWeight;
        XMFLOAT3 hRight{std::cos(headYaw), 0.0f, -std::sin(headYaw)};
        XMFLOAT3 hUp{0.0f, 1.0f, 0.0f};
        XMFLOAT3 hForward{std::sin(headYaw), 0.0f, std::cos(headYaw)};
        XMFLOAT3 skull = Add3(monster_, OrientedOffset(hRight, hUp, hForward,
            0.0f, (2.00f + MonsterHeadBobOffset()) * modelY + hover, kMonsterHeadForwardOffset * modelXZ));

        float headPitch = monsterHeadPitchOffset_ * scanWeight;
        if (std::abs(headPitch) > 0.0005f) {
            hForward = Normalize3(Add3(Scale3(hForward, std::cos(headPitch)), Scale3(hUp, std::sin(headPitch))), hForward);
            hUp = Normalize3(Cross3(hForward, hRight), hUp);
        }

        if (headLock > 0.001f) {
            XMFLOAT3 cameraFocus{camera_.x, camera_.y + 0.04f, camera_.z};
            XMFLOAT3 lookForward = Normalize3(Sub3(cameraFocus, skull), hForward);
            hForward = Normalize3(Lerp3(hForward, lookForward, Clamp01(headLock)), lookForward);
            hRight = Normalize3(Cross3(hUp, hForward), hRight);
            hUp = Normalize3(Cross3(hForward, hRight), hUp);
        }

        if (!skullMesh_.empty()) {
            float leftX = monsterUsingAltSkull_ ? settings_.monsterAltLeftEyeX : settings_.monsterLeftEyeX;
            float leftY = monsterUsingAltSkull_ ? settings_.monsterAltLeftEyeY : settings_.monsterLeftEyeY;
            float leftZ = monsterUsingAltSkull_ ? settings_.monsterAltLeftEyeZ : settings_.monsterLeftEyeZ;
            float rightX = monsterUsingAltSkull_ ? settings_.monsterAltRightEyeX : settings_.monsterRightEyeX;
            float rightY = monsterUsingAltSkull_ ? settings_.monsterAltRightEyeY : settings_.monsterRightEyeY;
            float rightZ = monsterUsingAltSkull_ ? settings_.monsterAltRightEyeZ : settings_.monsterRightEyeZ;
            XMFLOAT3 leftEye = Add3(skull, OrientedOffset(hRight, hUp, hForward,
                leftX * modelXZ, leftY * modelY, leftZ * modelXZ));
            XMFLOAT3 rightEye = Add3(skull, OrientedOffset(hRight, hUp, hForward,
                rightX * modelXZ, rightY * modelY, rightZ * modelXZ));
            return Lerp3(leftEye, rightEye, 0.5f);
        }
        return Add3(skull, OrientedOffset(hRight, hUp, hForward,
            0.0f, 0.025f * modelY, 0.162f * modelXZ));
    }

    void SetMonsterPreviewCamera(float orbitSeconds = 0.0f) {
        if (!monsterPreview_) return;
        XMFLOAT3 target = MonsterEyeFocus();
        if (monsterPreviewView_ != MonsterPreviewView::Top && monsterPreviewManualOrbit_) {
            float cp = std::cos(monsterPreviewOrbitPitch_);
            camera_ = {
                target.x + std::sin(monsterPreviewOrbitYaw_) * cp * monsterPreviewOrbitDistance_,
                target.y + std::sin(monsterPreviewOrbitPitch_) * monsterPreviewOrbitDistance_,
                target.z - std::cos(monsterPreviewOrbitYaw_) * cp * monsterPreviewOrbitDistance_
            };
        } else if (monsterPreviewView_ == MonsterPreviewView::Front) {
            camera_ = {target.x, target.y - 0.48f, target.z - 3.25f};
        } else if (monsterPreviewView_ == MonsterPreviewView::Side) {
            camera_ = {target.x + 3.25f, target.y - 0.48f, target.z};
        } else if (monsterPreviewView_ == MonsterPreviewView::LeftSide) {
            camera_ = {target.x - 3.25f, target.y - 0.48f, target.z};
        } else if (monsterPreviewView_ == MonsterPreviewView::Top) {
            camera_ = {target.x, target.y + 4.60f, target.z};
        } else {
            float orbit = orbitSeconds * 0.78f;
            camera_ = {
                target.x + std::sin(orbit) * 3.15f,
                target.y - 0.52f,
                target.z - std::cos(orbit) * 3.15f
            };
        }
        yaw_ = YawToPoint(target);
        if (monsterPreviewView_ == MonsterPreviewView::Top) {
            lookPitch_ = -20.0f;
        } else {
            lookPitch_ = std::clamp(PitchToPoint(target), -0.36f, 0.48f);
        }
        bodyYaw_ = monsterYaw_;
        flashlightYaw_ = yaw_;
        flashlightPitch_ = lookPitch_;
    }

    void AddDread(float amount) {
        if (!settings_.dreadEnabled || amount <= 0.0f) return;
        dreadLevel_ = Clamp01(dreadLevel_ + amount);
        flashlightAgitation_ = std::max(flashlightAgitation_, std::min(1.0f, amount * 2.2f));
    }

    void UpdateDreadMeterDisplay(float dt) {
        if (!settings_.dreadEnabled) {
            dreadMeterLevel_ = 0.0f;
            return;
        }
        float response = dreadLevel_ > dreadMeterLevel_ ? 18.0f : 5.5f;
        dreadMeterLevel_ += (dreadLevel_ - dreadMeterLevel_) * std::min(1.0f, std::max(0.0f, dt) * response);
    }

    void UpdateChasePanic(float dt, bool threat, float monsterDist) {
        float range = std::max(0.1f, MonsterSightDistance());
        float proximity = Clamp01((range - monsterDist) / range);
        bool chaseAlreadyCommitted = monsterRecognizedForChase_ || chaseMemoryTimer_ > 0.0f || chasePanic_ > 0.08f;
        if (threat && !threatVisibleLast_) {
            if (!chaseAlreadyCommitted) {
                monsterRecognitionDuration_ = RandRange(0.20f, 0.80f);
                monsterRecognitionTimer_ = monsterRecognitionDuration_;
                monsterRecognitionActive_ = true;
                monsterRecognizedForChase_ = false;
                panicFlashlightDuration_ = RandRange(1.10f, 1.55f);
                panicFlashlightTimer_ = panicFlashlightDuration_;
                flashlightAgitation_ = std::max(flashlightAgitation_, 1.0f);
                threatRepath_ = 0.0f;
                path_.clear();
                pathIndex_ = 0;
                turnLookBlend_ = 0.0f;
                turnLookYaw_ = yaw_;
                smoothedMoveSpeed_ = 0.0f;
            } else {
                monsterRecognitionActive_ = false;
                monsterRecognitionTimer_ = 0.0f;
                monsterRecognizedForChase_ = true;
                threatRepath_ = 0.0f;
                flashlightAgitation_ = std::max(flashlightAgitation_, 0.74f);
            }
        }
        if (threat) {
            stopTimer_ = 0.0f;
            headScanTimer_ = 0.0f;
            headScanDuration_ = 0.0f;
            lookBack_ = false;
            junctionScanActive_ = false;
            branchLookTimer_ = 0.0f;
            roomSurveyTimer_ = 0.0f;
            propLookTimer_ = 0.0f;
            bloodFocusTimer_ = 0.0f;
            ventReactionTimer_ = 0.0f;
            monsterRecognitionTimer_ = std::max(0.0f, monsterRecognitionTimer_ - dt);
            if (monsterRecognitionTimer_ <= 0.0f) {
                if (monsterRecognitionActive_ && !monsterRecognizedForChase_) {
                    monsterRunLaunchMeters_ = 0.0f;
                    monsterRunLaunchActive_ = true;
                    runIntensity_ = std::max(runIntensity_, 0.86f);
                    runEffort_ = std::max(runEffort_, 0.94f);
                    smoothedMoveSpeed_ = std::max(smoothedMoveSpeed_, settings_.runSpeed * 0.42f);
                }
                monsterRecognitionActive_ = false;
                monsterRecognizedForChase_ = true;
            }
            float hold = Lerp(2.4f, 5.2f, SmoothStep(0.12f, 0.82f, proximity));
            chaseMemoryTimer_ = std::max(chaseMemoryTimer_, hold);
        } else {
            monsterRecognitionActive_ = false;
            monsterRecognitionTimer_ = 0.0f;
            chaseMemoryTimer_ = std::max(0.0f, chaseMemoryTimer_ - dt);
        }

        float target = 0.0f;
        if (threat) {
            target = std::max(0.54f, SmoothStep(0.04f, 0.82f, proximity));
        } else if (chaseMemoryTimer_ > 0.0f) {
            target = Clamp01(chaseMemoryTimer_ / 4.8f) * 0.72f;
        }

        float response = target > chasePanic_
            ? (threat ? 1.85f : 0.75f)
            : (chaseMemoryTimer_ > 0.0f ? 0.45f : 0.80f);
        chasePanic_ += (target - chasePanic_) * std::min(1.0f, dt * response);
        if (chasePanic_ < 0.006f && target <= 0.0f) chasePanic_ = 0.0f;
        if (!threat && chaseMemoryTimer_ <= 0.0f && chasePanic_ <= 0.02f) {
            monsterRecognizedForChase_ = false;
            monsterRunLaunchActive_ = false;
            monsterRunLaunchMeters_ = 3.0f;
        }
        threatVisibleLast_ = threat;
    }

    bool ChasePanicActive() const {
        return chaseMemoryTimer_ > 0.0f || chasePanic_ > 0.08f;
    }

    bool MonsterSightingFreezeActive() const {
        return monsterRecognitionActive_ && monsterRecognitionTimer_ > 0.0f;
    }

    void UpdateDread(float dt, bool threat, float monsterDist) {
        if (!settings_.dreadEnabled) {
            dreadLevel_ = 0.0f;
            dreadMeterLevel_ = 0.0f;
            monsterSpottedLast_ = false;
            monsterSightDreadCooldown_ = 0.0f;
            return;
        }
        float decay = settings_.dreadDecayPerSecond * dt * (threat ? 0.20f : 1.0f);
        dreadLevel_ = std::max(0.0f, dreadLevel_ - decay);

        float proximity = Clamp01((settings_.dreadMonsterDistance - monsterDist) / std::max(0.1f, settings_.dreadMonsterDistance));
        if (proximity > 0.0f) {
            float visibleWeight = threat ? 1.0f : 0.32f;
            float gain = std::pow(proximity, 1.45f) * settings_.dreadMonsterGainPerSecond * visibleWeight * dt;
            dreadLevel_ = Clamp01(dreadLevel_ + gain);
            if (threat) {
                dreadLevel_ = std::max(dreadLevel_, proximity * 0.58f);
            }
        }
    }

    void UpdateMonsterSightDread(float dt, bool threat, float monsterDist) {
        if (!settings_.dreadEnabled) return;
        monsterSightDreadCooldown_ = std::max(0.0f, monsterSightDreadCooldown_ - dt);
        bool spotted = threat && PlayerLooksAt({monster_.x, 1.18f, monster_.z}, MonsterSightDistance(), 0.38f);
        if (spotted && !monsterSpottedLast_ && monsterSightDreadCooldown_ <= 0.0f) {
            float proximity = Clamp01((settings_.dreadMonsterDistance - monsterDist) / std::max(0.1f, settings_.dreadMonsterDistance));
            float spike = std::max(settings_.dreadJumpscareGain * 1.08f, 0.36f + proximity * 0.24f);
            AddDread(spike);
            flashlightAgitation_ = std::max(flashlightAgitation_, 0.88f);
            monsterSightDreadCooldown_ = RandRange(5.5f, 8.5f);
        }
        monsterSpottedLast_ = spotted;
    }

    void IncludeBloodReveal(const BloodScarePoint& point) {
        BloodRevealRegion region{};
        region.center = point.pos;
        float tileMin = std::max(0.1f, maze_.TileMinimum());
        float tileAvg = std::max(0.1f, maze_.TileAverage());
        float maxRadius = point.waterLiquid ? tileAvg * 2.65f : tileAvg * 1.35f;
        region.radius = std::clamp(point.radius, tileMin * 0.72f, maxRadius);
        region.activationTime = point.activationTime;
        region.waterLiquid = point.waterLiquid;

        for (BloodRevealRegion& existing : bloodRevealRegions_) {
            float dx = existing.center.x - region.center.x;
            float dz = existing.center.z - region.center.z;
            float distSq = dx * dx + dz * dz;
            float mergeLimit = tileMin * 0.24f;
            if (point.waterLiquid && existing.waterLiquid) {
                mergeLimit = std::max(tileAvg * 1.85f, std::min(existing.radius, region.radius) * 0.92f);
            } else if (point.waterLiquid != existing.waterLiquid) {
                mergeLimit = tileMin * 0.18f;
            }
            if (distSq <= mergeLimit * mergeLimit) {
                float dist = std::sqrt(distSq);
                if (point.waterLiquid && existing.waterLiquid && dist > 0.001f) {
                    float existingWeight = std::max(existing.radius, 0.1f);
                    float regionWeight = std::max(region.radius, 0.1f);
                    float totalWeight = existingWeight + regionWeight;
                    existing.center.x = (existing.center.x * existingWeight + region.center.x * regionWeight) / totalWeight;
                    existing.center.z = (existing.center.z * existingWeight + region.center.z * regionWeight) / totalWeight;
                    existing.radius = std::clamp(std::max(existing.radius, region.radius) + dist * 0.55f,
                        tileMin * 0.72f, maxRadius);
                } else {
                    existing.radius = std::max(existing.radius, region.radius);
                }
                existing.activationTime = std::min(existing.activationTime, region.activationTime);
                return;
            }
        }

        bloodRevealRegions_.push_back(region);
        constexpr size_t kMaxBloodRevealRegions = 96;
        if (bloodRevealRegions_.size() > kMaxBloodRevealRegions) {
            bloodRevealRegions_.erase(bloodRevealRegions_.begin());
        }
    }

    void UpdateMonsterProximityBlood(float dt) {
        if (deathActive_ || exitTransitionActive_ || bloodScarePoints_.empty()) return;
        float monsterRange = std::max(0.1f, settings_.dreadMonsterDistance);
        float proximity = SmoothStep(0.16f, 1.0f, Clamp01((monsterRange - MonsterDistance()) / monsterRange));
        if (proximity <= 0.001f) {
            proximityBloodPulseCooldown_ = std::min(proximityBloodPulseCooldown_, 9.0f);
            return;
        }

        proximityBloodPulseCooldown_ -= dt;
        if (proximityBloodPulseCooldown_ > 0.0f) return;

        Tile cameraTile = CameraTile();
        float tileMin = std::max(0.1f, maze_.TileMinimum());
        float tileAvg = std::max(0.1f, maze_.TileAverage());
        float maxDist = Lerp(tileAvg * 4.2f, tileAvg * 8.8f, proximity);
        XMFLOAT3 forward = FlashlightForward();

        int revealCount = 1;
        if (proximity > 0.64f && RandRange(0.0f, 1.0f) < 0.42f) revealCount = 2;
        if (proximity > 0.86f && RandRange(0.0f, 1.0f) < 0.24f) revealCount = 3;

        for (int reveal = 0; reveal < revealCount; ++reveal) {
            int bestIndex = -1;
            float bestScore = -1.0e9f;
            for (size_t i = 0; i < bloodScarePoints_.size(); ++i) {
                const BloodScarePoint& point = bloodScarePoints_[i];
                if (point.triggered || !point.revealBlood || point.waterLiquid) continue;
                float dx = point.pos.x - camera_.x;
                float dz = point.pos.z - camera_.z;
                float distSq = dx * dx + dz * dz;
                if (distSq < tileMin * tileMin * 0.36f || distSq > maxDist * maxDist) continue;
                float dist = std::sqrt(distSq);
                Tile bloodTile = maze_.TileFromWorld(point.pos.x, point.pos.z);
                float lineBonus = maze_.LineClear(cameraTile, bloodTile) ? 2.4f : 0.0f;
                float ahead = dist > 0.001f ? (dx * forward.x + dz * forward.z) / dist : 0.0f;
                float aheadBonus = SmoothStep(-0.20f, 0.78f, ahead) * 1.7f;
                float nearPreferred = 1.0f - std::abs(dist - tileAvg * 3.2f) / std::max(tileAvg * 4.5f, 0.1f);
                float score = nearPreferred * 2.6f + lineBonus + aheadBonus + RandRange(0.0f, 1.0f);
                if (score > bestScore) {
                    bestScore = score;
                    bestIndex = static_cast<int>(i);
                }
            }
            if (bestIndex < 0) break;

            BloodScarePoint& point = bloodScarePoints_[static_cast<size_t>(bestIndex)];
            point.triggered = true;
            point.activationTime = time_ - RandRange(0.75f, 2.8f) * (0.72f + proximity * 0.86f);
            IncludeBloodReveal(point);
            bloodScareActiveUntil_ = std::max(bloodScareActiveUntil_, time_ + 150.0f);
        }

        float scareScale = ScareCooldownScale();
        float minSeconds = Lerp(8.5f, 1.35f, proximity) * scareScale;
        float maxSeconds = Lerp(14.0f, 2.80f, proximity) * scareScale;
        proximityBloodPulseCooldown_ = RandRange(std::max(0.85f, minSeconds), std::max(1.20f, maxSeconds));
        if (proximity > 0.50f) {
            dreadLevel_ = Clamp01(dreadLevel_ + proximity * 0.018f);
        }
    }

    void UpdateBloodDread(float dt) {
        bloodFocusReactionCooldown_ = std::max(0.0f, bloodFocusReactionCooldown_ - dt);
        if (deathActive_ || exitTransitionActive_ || bloodScarePoints_.empty()) return;
        if (IsThreatVisible() || ChasePanicActive()) {
            bloodFocusTimer_ = 0.0f;
            activeBloodScareIndex_ = -1;
            return;
        }
        Tile cameraTile = CameraTile();
        XMFLOAT3 flashlightDir = FlashlightForward();
        for (size_t i = 0; i < bloodScarePoints_.size(); ++i) {
            BloodScarePoint& point = bloodScarePoints_[i];
            XMFLOAT3 target = point.source.y > 0.01f ? point.source : point.pos;
            XMFLOAT3 visibilityTarget = target;
            if (point.requireFacing) {
                float inset = std::clamp(maze_.TileMinimum() * 0.32f, 0.22f, 0.58f);
                visibilityTarget = Add3(target, Scale3(point.normal, inset));
            }
            float dx = target.x - camera_.x;
            float dy = target.y - camera_.y;
            float dz = target.z - camera_.z;
            float floorDx = point.pos.x - camera_.x;
            float floorDz = point.pos.z - camera_.z;
            float horizontalDist = std::sqrt(floorDx * floorDx + floorDz * floorDz);
            Tile bloodTile = maze_.TileFromWorld(visibilityTarget.x, visibilityTarget.z);
            float tileMin = std::max(0.1f, maze_.TileMinimum());
            float tileAvg = std::max(0.1f, maze_.TileAverage());
            if (!point.triggered) {
                float revealDistance = point.waterLiquid ? tileAvg * 8.25f : tileAvg * 5.35f;
                float aheadLimit = point.waterLiquid ? -0.18f : -0.06f;
                int revealSteps = point.waterLiquid ? 12 : 8;
                if (!ScareSourceAhead(visibilityTarget,
                    tileMin * 0.36f,
                    revealDistance,
                    revealSteps,
                    aheadLimit)) continue;
            } else if (horizontalDist > tileAvg * (point.waterLiquid ? 8.25f : 5.35f)) {
                continue;
            }
            if (!maze_.LineClear(cameraTile, bloodTile)) continue;
            if (point.requireFacing) {
                XMFLOAT3 fromSurface{
                    camera_.x - target.x,
                    0.0f,
                    camera_.z - target.z
                };
                float facing = Dot3(fromSurface, point.normal);
                if (facing < 0.045f) continue;
                if (!CameraSegmentOpenThroughOpen(camera_.x, camera_.z, visibilityTarget.x, visibilityTarget.z, false)) continue;
            }

            float dist = std::sqrt(dx * dx + dy * dy + dz * dz);
            float aim = dist > 0.001f ? (dx * flashlightDir.x + dy * flashlightDir.y + dz * flashlightDir.z) / dist : 1.0f;
            if (point.triggered && aim < -0.18f) continue;

            if (!point.triggered) {
                point.triggered = true;
                if (point.waterLiquid) {
                    float prewarm = Clamp01((horizontalDist - tileAvg * 2.0f) / std::max(tileAvg * 3.8f, 0.1f)) * 4.6f;
                    point.activationTime = time_ - prewarm;
                    point.focusTaken = true;
                } else {
                    point.activationTime = time_;
                }
                if (point.revealBlood) {
                    IncludeBloodReveal(point);
                }
                bloodScareActiveUntil_ = time_ + 150.0f;
                continue;
            }
            if (point.waterLiquid) continue;
            if (horizontalDist > tileAvg * 3.45f) continue;
            float visibleAge = time_ - point.activationTime;
            float minVisibleAge = point.requireFacing
                ? 1.10f
                : (target.y > settings_.wallHeightMeters * 0.55f ? 0.92f : 0.76f);
            if (point.focusTaken || visibleAge < std::max(point.focusDelaySeconds, minVisibleAge)) continue;
            bool focusAllowed = bloodFocusReactionsTaken_ == 0 ||
                (bloodFocusReactionsTaken_ == 1 && bloodFocusReactionCooldown_ <= 0.0f);
            if (!focusAllowed) continue;

            activeBloodScareIndex_ = static_cast<int>(i);
            bloodScareActiveUntil_ = std::max(bloodScareActiveUntil_, time_ + 150.0f);
            point.focusTaken = true;
            ++bloodFocusReactionsTaken_;
            bloodFocusReactionCooldown_ = bloodFocusReactionsTaken_ == 1
                ? RandRange(5.0f, 15.0f)
                : 1000000.0f;
            bloodFocusDuration_ = RandRange(1.45f, 2.25f);
            bloodFocusTimer_ = bloodFocusDuration_;
            bloodFocusTarget_ = {
                target.x,
                std::clamp(target.y, 0.16f, settings_.wallHeightMeters - 0.05f),
                target.z
            };
            stopTimer_ = 0.0f;
            headScanTimer_ = 0.0f;
            lookBack_ = false;
            junctionScanActive_ = false;
            propLookTimer_ = 0.0f;
            float closeness = Clamp01((point.radius - horizontalDist) / std::max(0.001f, point.radius));
            float gaze = SmoothStep(0.42f, 0.88f, aim);
            float reactionScale = std::clamp(point.dreadScale, 0.20f, 1.35f);
            float spike = std::max(settings_.dreadJumpscareGain * 1.15f, 0.42f + closeness * 0.24f + gaze * 0.24f) * reactionScale;
            AddDread(spike);
            AlertMonsterToPlayerTrigger(target);
            flashlightAgitation_ = std::max(flashlightAgitation_, (0.90f + closeness * 0.20f + gaze * 0.18f) * Lerp(0.62f, 1.0f, reactionScale));
            flashlightSnapCooldown_ = std::min(flashlightSnapCooldown_, 0.08f);
            stumbleTimer_ = std::max(stumbleTimer_, (0.10f + closeness * 0.08f) * reactionScale);
            stumbleDuration_ = std::max(stumbleDuration_, 0.20f * reactionScale);
            secondsSinceLookBack_ = 0.0f;
        }
    }

    float DreadPressure() const {
        if (!settings_.dreadEnabled) return 0.0f;
        float proximity = Clamp01((settings_.dreadMonsterDistance - MonsterDistance()) / std::max(0.1f, settings_.dreadMonsterDistance));
        return Clamp01(std::max(dreadLevel_ * 0.86f, proximity * 0.74f));
    }

    float DreadFlashlightMultiplier() const {
        if (!settings_.dreadEnabled) return 1.0f;
        float monsterRange = std::max(0.1f, settings_.dreadMonsterDistance);
        float monsterProximity = Clamp01((monsterRange - MonsterDistance()) / monsterRange);
        float pressure = Clamp01(SmoothStep(0.10f, 1.0f, monsterProximity) * settings_.dreadFlashlightFlicker);
        if (pressure <= 0.02f) return 1.0f;
        float waves =
            std::sin(time_ * 19.7f + std::sin(time_ * 3.1f) * 0.8f) +
            std::sin(time_ * 43.3f + 1.8f) * 0.58f +
            std::sin(time_ * 91.9f + 0.4f) * 0.28f;
        float gate = waves > (1.06f - pressure * 1.18f) ? 1.0f : 0.0f;
        float flutter = 0.5f + 0.5f * std::sin(time_ * (11.0f + pressure * 29.0f) + std::sin(time_ * 5.7f) * 1.4f);
        float drop = pressure * (0.05f + flutter * 0.12f + gate * (0.24f + pressure * 0.42f));
        return std::clamp(1.0f - drop, 0.18f, 1.05f);
    }

    float YawToPoint(const XMFLOAT3& target) const {
        return std::atan2(target.x - camera_.x, target.z - camera_.z);
    }

    float PitchToPoint(const XMFLOAT3& target) const {
        float dx = target.x - camera_.x;
        float dy = target.y - camera_.y;
        float dz = target.z - camera_.z;
        float horizontal = std::sqrt(dx * dx + dz * dz);
        return std::atan2(dy, std::max(0.001f, horizontal));
    }

    XMFLOAT3 Forward() const {
        return {std::sin(yaw_), 0.0f, std::cos(yaw_)};
    }

    static XMFLOAT3 DirectionFromYawPitch(float yaw, float pitch) {
        float cp = std::cos(pitch);
        return {std::sin(yaw) * cp, std::sin(pitch), std::cos(yaw) * cp};
    }

    XMFLOAT3 FlashlightForward() const {
        return DirectionFromYawPitch(flashlightYaw_, flashlightPitch_);
    }

    XMFLOAT3 FlashlightOrigin() const {
        XMFLOAT3 forward = Normalize3(FlashlightForward(), {0.0f, 0.0f, 1.0f});
        XMFLOAT3 right = Normalize3(Cross3({0.0f, 1.0f, 0.0f}, forward), {1.0f, 0.0f, 0.0f});
        return Add3(camera_, Add3(Scale3(right, 0.16f), Add3(Scale3({0.0f, 1.0f, 0.0f}, -0.18f), Scale3(forward, 0.08f))));
    }

    float FlashlightFocusTargetDistance() const {
        float maxDist = std::clamp(settings_.flashlightShadowDistanceMeters * 0.56f, 3.6f, 9.5f);
        XMFLOAT3 origin = camera_;
        XMFLOAT3 dir = Normalize3(DirectionFromYawPitch(yaw_, lookPitch_), {0.0f, 0.0f, 1.0f});
        float horizontalLen = std::sqrt(dir.x * dir.x + dir.z * dir.z);
        float target = maxDist;
        if (horizontalLen > 0.04f) {
            float yaw = std::atan2(dir.x, dir.z);
            target = std::min(target, ViewRayOpenDistance(yaw, maxDist * horizontalLen) / horizontalLen);
        }
        if (dir.y > 0.025f) {
            target = std::min(target, (settings_.wallHeightMeters - origin.y - 0.08f) / dir.y);
        } else if (dir.y < -0.025f) {
            target = std::min(target, (0.08f - origin.y) / dir.y);
        }
        return std::clamp(target, 0.55f, maxDist);
    }

    void UpdateAirParticleFocus(float dt) {
        float target = FlashlightFocusTargetDistance();
        if (airFocusDistance_ <= 0.0f) airFocusDistance_ = target;
        float follow = std::min(1.0f, std::max(0.001f, dt) * 1.85f);
        airFocusDistance_ += (target - airFocusDistance_) * follow;
    }

    void UpdateAirParticlePerformanceBudget(float dt) {
        if (!settings_.airParticles || monsterPreview_) {
            airParticleBudgetScale_ = 1.0f;
            airParticleFrameDt_ = 0.0f;
            return;
        }
        if (dt <= 0.0f || dt >= 0.20f) return;
        if (airParticleFrameDt_ <= 0.0f) airParticleFrameDt_ = dt;
        airParticleFrameDt_ += (dt - airParticleFrameDt_) * std::min(1.0f, dt * 2.0f);

        float target = 1.0f;
        if (time_ > 2.0f) {
            if (airParticleFrameDt_ > 0.034f) target = 0.40f;
            else if (airParticleFrameDt_ > 0.028f) target = 0.55f;
            else if (airParticleFrameDt_ > 0.023f) target = 0.72f;
            else if (airParticleFrameDt_ > 0.019f) target = 0.88f;
        }
        float follow = target < airParticleBudgetScale_
            ? std::min(1.0f, dt * 3.0f)
            : std::min(1.0f, dt * 0.22f);
        airParticleBudgetScale_ += (target - airParticleBudgetScale_) * follow;
        airParticleBudgetScale_ = std::clamp(airParticleBudgetScale_, 0.34f, 1.0f);
    }

    int DesiredAirParticleCount() const {
        if (!settings_.airParticles || settings_.airParticleDensity <= 0.001f || monsterPreview_) return 0;
        float density = std::clamp(settings_.airParticleDensity, 0.0f, 4.0f);
        float budget = std::clamp(airParticleBudgetScale_, 0.34f, 1.0f);
        return std::clamp(static_cast<int>(std::round(3400.0f * density * budget)), 0, 11000);
    }

    void RespawnAirParticle(AirParticle& p, bool initial) {
        float radius = std::clamp(settings_.flashlightShadowDistanceMeters * 0.70f, 6.0f, 12.0f);
        XMFLOAT3 forward{std::sin(flashlightYaw_), 0.0f, std::cos(flashlightYaw_)};
        XMFLOAT3 right{std::cos(flashlightYaw_), 0.0f, -std::sin(flashlightYaw_)};
        XMFLOAT3 pos = camera_;
        float coneHalf = std::clamp(settings_.flashlightConeDegrees, 20.0f, 140.0f) * 0.5f * kPi / 180.0f;
        float coneSpread = std::tan(std::min(coneHalf * 0.72f, 1.12f));
        float layerRoll = RandRange(0.0f, 1.0f);
        p.nearLayer = layerRoll < 0.018f ? 2.0f : (layerRoll < 0.165f ? 1.0f : 0.0f);
        for (int attempt = 0; attempt < 24; ++attempt) {
            float depthT = std::pow(RandRange(0.0f, 1.0f), 0.42f);
            float depth = Lerp(0.45f, radius, depthT);
            if (p.nearLayer > 1.5f) {
                depth = RandRange(0.24f, 1.20f);
            } else if (p.nearLayer > 0.5f) {
                depth = RandRange(0.65f, 2.85f);
            } else if (RandRange(0.0f, 1.0f) < 0.10f) {
                depth = RandRange(0.30f, radius);
            }
            float sideLimit = std::clamp(depth * coneSpread, 0.22f, radius * 0.82f);
            float side = RandRange(-sideLimit, sideLimit);
            float yMin = p.nearLayer > 0.5f ? 0.34f : 0.22f;
            float yMax = std::max(yMin + 0.02f, settings_.wallHeightMeters - (p.nearLayer > 0.5f ? 0.24f : 0.14f));
            float y = RandRange(yMin, yMax);
            pos = Add3({camera_.x, y, camera_.z}, Add3(Scale3(forward, depth), Scale3(right, side)));
            Tile tile = maze_.TileFromWorld(pos.x, pos.z);
            if (maze_.IsOpen(tile.x, tile.y)) break;
        }

        p.pos = pos;
        float driftRoll = std::pow(RandRange(0.0f, 1.0f), 1.35f);
        float driftScale = Lerp(0.36f, 2.15f, driftRoll);
        if (RandRange(0.0f, 1.0f) < 0.075f) driftScale *= RandRange(1.35f, 2.25f);
        if (p.nearLayer > 0.5f) driftScale *= RandRange(0.78f, 1.32f);
        p.vel = {
            RandRange(-0.030f, 0.030f) * driftScale,
            RandRange(-0.010f, 0.026f) * driftScale,
            RandRange(-0.030f, 0.030f) * driftScale
        };
        p.life = RandRange(28.0f, 68.0f);
        p.age = initial ? RandRange(0.0f, p.life * 0.82f) : 0.0f;
        float sizeRoll = RandRange(0.0f, 1.0f);
        float sizeT = RandRange(0.0f, 1.0f);
        float baseSize = 0.0f;
        if (sizeRoll < 0.50f) {
            baseSize = Lerp(0.0020f, 0.0052f, std::pow(sizeT, 1.35f));
        } else if (sizeRoll < 0.90f) {
            baseSize = Lerp(0.0052f, 0.0105f, sizeT);
        } else {
            baseSize = Lerp(0.0105f, 0.0185f, std::sqrt(sizeT));
        }
        float layerScale = p.nearLayer > 1.5f ? RandRange(1.85f, 3.10f) : (p.nearLayer > 0.5f ? RandRange(1.25f, 2.05f) : 1.0f);
        p.size = baseSize * layerScale * std::clamp(settings_.airParticleSize, 0.20f, 4.0f);
        p.aspect = RandRange(0.86f, 1.16f);
        p.seed = RandRange(0.0f, 1.0f);
        p.angle = RandRange(0.0f, kPi * 2.0f);
        p.spin = RandRange(-0.18f, 0.18f);
    }

    void UpdateAirParticles(float dt) {
        int desired = DesiredAirParticleCount();
        if (desired <= 0) {
            airParticles_.clear();
            return;
        }
        if (airParticles_.size() < static_cast<size_t>(desired)) {
            size_t oldSize = airParticles_.size();
            airParticles_.resize(static_cast<size_t>(desired));
            for (size_t i = oldSize; i < airParticles_.size(); ++i) {
                RespawnAirParticle(airParticles_[i], true);
            }
        } else if (airParticles_.size() > static_cast<size_t>(desired)) {
            airParticles_.resize(static_cast<size_t>(desired));
        }

        float step = std::min(std::max(dt, 0.0f), 0.10f);
        float keepRadius = std::clamp(settings_.flashlightShadowDistanceMeters * 0.78f, 6.5f, 13.0f);
        float keepRadiusSq = keepRadius * keepRadius;
        for (AirParticle& p : airParticles_) {
            p.age += step;
            float driftA = p.seed * kPi * 2.0f;
            p.pos.x += (p.vel.x + std::sin(time_ * 0.19f + driftA) * 0.012f) * step;
            p.pos.y += (p.vel.y + std::sin(time_ * 0.13f + driftA * 1.7f) * 0.008f) * step;
            p.pos.z += (p.vel.z + std::cos(time_ * 0.17f + driftA * 1.3f) * 0.012f) * step;
            p.angle += p.spin * step;

            float dx = p.pos.x - camera_.x;
            float dz = p.pos.z - camera_.z;
            Tile tile = maze_.TileFromWorld(p.pos.x, p.pos.z);
            if (p.age >= p.life || p.pos.y < 0.16f || p.pos.y > settings_.wallHeightMeters - 0.10f ||
                dx * dx + dz * dz > keepRadiusSq || !maze_.IsOpen(tile.x, tile.y)) {
                RespawnAirParticle(p, false);
            }
        }
    }

    void UpdateFlashlightAim(float dt) {
        dt = std::max(0.001f, dt);
        constexpr float kDartFrequencyDivisor = 4.0f;
        float yawDelta = AngleWrap(yaw_ - previousCameraYaw_);
        float pitchDelta = lookPitch_ - previousCameraPitch_;
        float turnRate = (std::abs(yawDelta) + std::abs(pitchDelta) * 0.65f) / dt;
        float fastTurn = Clamp01((turnRate - 2.1f) / 5.8f);
        float blurScale = monsterPreview_ ? 0.0f : std::clamp(settings_.motionBlurAmount, 0.0f, 2.0f);
        XMFLOAT2 blurTarget{
            std::clamp(-yawDelta * 0.36f * blurScale, -0.045f, 0.045f),
            std::clamp(pitchDelta * 0.48f * blurScale, -0.045f, 0.045f)
        };
        float blurFollow = std::min(1.0f, dt * (fastTurn > 0.20f ? 18.0f : 7.0f));
        cameraMotionBlur_.x += (blurTarget.x - cameraMotionBlur_.x) * blurFollow;
        cameraMotionBlur_.y += (blurTarget.y - cameraMotionBlur_.y) * blurFollow;
        bool calmExplorationTurn = !ChasePanicActive() && dangerLevel_ < 0.16f && DreadPressure() < 0.18f;
        if (fastTurn > 0.0f && !calmExplorationTurn) {
            flashlightAgitation_ = std::max(flashlightAgitation_, fastTurn);
            flashlightDartCooldown_ = std::min(flashlightDartCooldown_, RandRange(0.09f, 0.22f) * kDartFrequencyDivisor);
        }

        flashlightAgitation_ = std::max(0.0f, flashlightAgitation_ - dt * 0.42f);
        panicFlashlightTimer_ = std::max(0.0f, panicFlashlightTimer_ - dt);
        flashlightDartCooldown_ = std::max(0.0f, flashlightDartCooldown_ - dt);
        flashlightDartTimer_ = std::max(0.0f, flashlightDartTimer_ - dt);

        float dreadPressure = DreadPressure();
        float activeAgitation = std::max(flashlightAgitation_, dreadPressure * 0.68f);
        auto mergeFlashlightHold = [&](float yawOffset, float pitchOffset, float carry) {
            constexpr float kHoldYawLimit = 0.24f;
            constexpr float kHoldPitchLimit = 0.15f;
            flashlightHoldYaw_ = std::clamp(flashlightHoldYaw_ * carry + yawOffset, -kHoldYawLimit, kHoldYawLimit);
            flashlightHoldPitch_ = std::clamp(flashlightHoldPitch_ * carry + pitchOffset, -kHoldPitchLimit, kHoldPitchLimit);
        };

        flashlightSnapCooldown_ = std::max(0.0f, flashlightSnapCooldown_ - dt);
        flashlightSnapTimer_ = std::max(0.0f, flashlightSnapTimer_ - dt);
        if (flashlightSnapTimer_ <= 0.0f && flashlightSnapCooldown_ <= 0.0f) {
            float pressure = Clamp01(activeAgitation * 0.72f + dreadPressure * 0.48f + fastTurn * 0.40f);
            flashlightSnapSharp_ = RandRange(0.0f, 1.0f) < (0.22f + pressure * 0.56f);
            flashlightSnapDuration_ = flashlightSnapSharp_
                ? RandRange(0.055f, 0.14f)
                : RandRange(0.34f, 1.18f);
            flashlightSnapTimer_ = flashlightSnapDuration_;
            float sharpBoost = flashlightSnapSharp_ ? 1.55f : 0.62f;
            float amount = (0.45f + pressure * 1.10f) * settings_.flashlightSwayAmount * sharpBoost;
            flashlightSnapYaw_ = RandRange(-0.075f, 0.075f) * amount;
            flashlightSnapPitch_ = RandRange(-0.040f, 0.050f) * amount;
            mergeFlashlightHold(flashlightSnapYaw_, flashlightSnapPitch_, flashlightSnapSharp_ ? 0.42f : 0.64f);
            flashlightSnapCooldown_ = flashlightSnapSharp_
                ? RandRange(0.34f, 0.95f) * (1.0f - pressure * 0.28f)
                : RandRange(0.80f, 2.60f) * (1.0f - pressure * 0.22f);
            flashlightSnapCooldown_ = std::max(0.18f, flashlightSnapCooldown_);
        }

        if (activeAgitation > 0.14f && flashlightDartTimer_ <= 0.0f && flashlightDartCooldown_ <= 0.0f) {
            flashlightDartDuration_ = RandRange(0.105f, 0.24f);
            flashlightDartTimer_ = flashlightDartDuration_;
            float amount = SmoothStep(0.10f, 1.0f, activeAgitation);
            float dreadJitter = 1.0f + dreadPressure * 0.75f;
            flashlightDartYaw_ = RandRange(-0.22f, 0.22f) * amount * dreadJitter * settings_.flashlightPanicDartAmount;
            flashlightDartPitch_ = RandRange(-0.13f, 0.10f) * amount * dreadJitter * settings_.flashlightPanicDartAmount;
            mergeFlashlightHold(std::clamp(flashlightDartYaw_ * 0.46f, -0.18f, 0.18f),
                std::clamp(flashlightDartPitch_ * 0.48f, -0.11f, 0.11f), 0.54f);
            float cooldown = RandRange(0.26f, 0.62f) * (1.0f - amount * 0.18f) * (1.0f - dreadPressure * 0.12f);
            flashlightDartCooldown_ = std::max(0.16f * kDartFrequencyDivisor, cooldown * kDartFrequencyDivisor);
        }

        float gentleScale = (1.0f + dangerLevel_ * 1.10f + activeAgitation * 1.05f + dreadPressure * 0.95f) * settings_.flashlightSwayAmount;
        float gentleYaw =
            std::sin(time_ * 0.31f + std::sin(time_ * 0.13f) * 1.6f) * 0.026f * gentleScale +
            std::sin(time_ * 1.18f + std::sin(time_ * 0.41f) * 0.7f) * 0.044f * gentleScale +
            std::sin(time_ * 2.85f + 1.7f) * 0.020f * gentleScale;
        float gentlePitch =
            std::sin(time_ * 0.27f + 1.4f) * 0.014f * gentleScale +
            std::sin(time_ * 1.02f + 2.4f) * 0.022f * gentleScale +
            std::sin(time_ * 2.32f) * 0.011f * gentleScale;

        float tremorScale = (0.0035f + activeAgitation * 0.0075f + dreadPressure * 0.0105f) * settings_.flashlightSwayAmount;
        float tremorYaw =
            std::sin(time_ * 12.7f + std::sin(time_ * 3.9f) * 1.4f) * 0.70f +
            std::sin(time_ * 28.9f + 2.1f) * 0.24f +
            std::sin(time_ * 61.0f + 0.7f) * 0.09f;
        float tremorPitch =
            std::sin(time_ * 10.6f + 1.8f) * 0.58f +
            std::sin(time_ * 24.4f + std::sin(time_ * 5.5f)) * 0.28f +
            std::sin(time_ * 57.0f + 2.6f) * 0.08f;
        gentleYaw += tremorYaw * tremorScale;
        gentlePitch += tremorPitch * tremorScale * 0.72f;

        float stepRun = Clamp01(runIntensity_ * 0.78f + runEffort_ * 0.62f);
        if (stepRun > 0.001f) {
            float stepWave = std::sin(stepPhase_);
            float crossWave = std::sin(stepPhase_ * 0.5f + 0.65f);
            float runSwing = settings_.flashlightSwayAmount * stepRun;
            gentleYaw += (stepWave * 0.040f + crossWave * 0.024f) * runSwing;
            gentlePitch += (-stepWave * 0.028f + std::abs(crossWave) * 0.018f) * runSwing;
        }

        if (panicFlashlightTimer_ > 0.0f && panicFlashlightDuration_ > 0.001f) {
            float t = 1.0f - panicFlashlightTimer_ / panicFlashlightDuration_;
            float envelope = (1.0f - SmoothStep(0.48f, 1.0f, t)) * SmoothStep(0.0f, 0.10f, t);
            float swing = std::sin(t * kPi * 3.35f);
            float snap = std::sin(t * kPi * 7.10f) * 0.26f;
            float panic = envelope * (0.78f + chasePanic_ * 0.42f) * settings_.flashlightPanicDartAmount;
            gentleYaw += (swing * 0.080f + snap * 0.034f) * panic;
            gentlePitch += (swing * 0.052f - envelope * 0.030f) * panic;
        }

        bool impulseHolding = flashlightSnapTimer_ > 0.0f || flashlightDartTimer_ > 0.0f;
        float holdPressure = Clamp01(activeAgitation + dreadPressure * 0.7f);
        float holdReturnRate = impulseHolding
            ? Lerp(0.18f, 0.08f, holdPressure)
            : Lerp(1.10f, 0.42f, holdPressure);
        float holdReturn = 1.0f - std::exp(-dt * holdReturnRate);
        flashlightHoldYaw_ += (0.0f - flashlightHoldYaw_) * holdReturn;
        flashlightHoldPitch_ += (0.0f - flashlightHoldPitch_) * holdReturn;

        float targetYaw = yaw_ + gentleYaw + flashlightHoldYaw_;
        float targetPitch = std::clamp(lookPitch_ + gentlePitch + flashlightHoldPitch_, -0.48f, 0.34f);
        float propInspectWeight = 0.0f;
        if (!ChasePanicActive() && propLookTimer_ > 0.0f && propLookDuration_ > 0.001f) {
            float t = 1.0f - propLookTimer_ / propLookDuration_;
            propInspectWeight = SmoothStep(0.0f, 0.22f, t) * (1.0f - SmoothStep(0.72f, 1.0f, t));
            float scan = std::sin((t * 1.25f + propLookScanSeed_) * kPi * 2.0f);
            float fineScan = std::sin((t * 2.70f + propLookScanSeed_ * 1.7f) * kPi * 2.0f);
            float propYaw = YawToPoint(propLookTarget_) + scan * 0.034f + fineScan * 0.010f;
            float propPitch = std::clamp(PitchToPoint(propLookTarget_) + scan * 0.012f - fineScan * 0.006f, -0.40f, 0.26f);
            targetYaw += AngleWrap(propYaw - targetYaw) * (propInspectWeight * 0.72f);
            targetPitch += (propPitch - targetPitch) * (propInspectWeight * 0.68f);
        }
        float snapFollowBoost = flashlightSnapSharp_ && flashlightSnapTimer_ > 0.0f ? 0.55f : 0.0f;
        float dartFollowBoost = flashlightDartTimer_ > 0.0f ? 0.42f : 0.0f;
        float followSpeed = Lerp(3.7f, 17.5f, std::max(fastTurn,
            std::max(propInspectWeight * 0.26f, std::max(activeAgitation * 0.7f, std::max(snapFollowBoost, dartFollowBoost))))) * settings_.flashlightFollowSpeed;
        flashlightYaw_ += AngleWrap(targetYaw - flashlightYaw_) * std::min(1.0f, dt * followSpeed);
        flashlightPitch_ += (targetPitch - flashlightPitch_) * std::min(1.0f, dt * followSpeed);

        previousCameraYaw_ = yaw_;
        previousCameraPitch_ = lookPitch_;
    }

    bool IsThreatVisible() const {
        return MonsterLineOfSightToPlayer();
    }

    bool IsRoomLike(Tile t) const {
        return maze_.OpenNeighborCount(t) >= 3 || maze_.LocalOpenCount(t, 2) >= 14;
    }

    bool IsOpenAreaLike(Tile t) const {
        if (!maze_.IsOpen(t.x, t.y)) return false;
        return HasOpenSquare(t) || maze_.LocalOpenCount(t, 2) >= 14;
    }

    bool IsTightCorridor(Tile t) const {
        if (!maze_.IsOpen(t.x, t.y)) return false;
        return maze_.OpenNeighborCount(t) <= 2 && !IsRoomLike(t);
    }

    bool IsCorridorLike(Tile t) const {
        if (!maze_.IsOpen(t.x, t.y)) return false;
        return !IsOpenAreaLike(t);
    }

    bool IsStraightCorridor(Tile t) const {
        if (!maze_.IsOpen(t.x, t.y)) return false;
        if (maze_.OpenNeighborCount(t) != 2 || IsRoomLike(t)) return false;
        bool east = maze_.IsOpen(t.x + 1, t.y);
        bool west = maze_.IsOpen(t.x - 1, t.y);
        bool south = maze_.IsOpen(t.x, t.y + 1);
        bool north = maze_.IsOpen(t.x, t.y - 1);
        return (east && west) || (south && north);
    }

    bool OpenAreaAllowsFreeRun(Tile t) const {
        if (!maze_.IsOpen(t.x, t.y)) return false;
        return IsRoomLike(t) || HasOpenSquare(t) || maze_.LocalOpenCount(t, 1) >= 6;
    }

    bool HasOpenSquare(Tile t) const {
        for (int oy = -1; oy <= 0; ++oy) {
            for (int ox = -1; ox <= 0; ++ox) {
                if (maze_.IsOpen(t.x + ox,     t.y + oy) &&
                    maze_.IsOpen(t.x + ox + 1, t.y + oy) &&
                    maze_.IsOpen(t.x + ox,     t.y + oy + 1) &&
                    maze_.IsOpen(t.x + ox + 1, t.y + oy + 1)) {
                    return true;
                }
            }
        }
        return false;
    }

    bool IsTightTJunction(Tile cur, Tile previous) const {
        if (!maze_.IsOpen(cur.x, cur.y)) return false;
        if (std::abs(cur.x - previous.x) + std::abs(cur.y - previous.y) != 1) return false;
        if (!maze_.IsOpen(previous.x, previous.y)) return false;
        if (maze_.OpenNeighborCount(cur) != 3) return false;
        if (HasOpenSquare(cur)) return false;
        if (maze_.LocalOpenCount(cur, 1) > 5) return false;
        if (maze_.LocalOpenCount(cur, 2) > 13) return false;
        return true;
    }

    int PathSideBranchCount(Tile cur, Tile previous, Tile nextTarget) const {
        static constexpr int dirs[4][2] = {{1, 0}, {-1, 0}, {0, 1}, {0, -1}};
        int count = 0;
        for (const auto& d : dirs) {
            Tile n{cur.x + d[0], cur.y + d[1]};
            if (!maze_.IsOpen(n.x, n.y)) continue;
            if (n == previous || n == nextTarget) continue;
            ++count;
        }
        return count;
    }

    bool IsRoomSurveySpot(Tile t) const {
        if (!IsOpenAreaLike(t)) return false;
        int directOpen = maze_.OpenNeighborCount(t);
        if (directOpen >= 3) return true;
        if (maze_.LocalOpenCount(t, 1) >= 6) return true;
        return HasOpenSquare(t) && maze_.LocalOpenCount(t, 2) >= 15;
    }

    bool ActivePathValid(Tile cur) const {
        if (pathIndex_ >= path_.size()) return false;
        if (!maze_.IsOpen(cur.x, cur.y)) return false;
        Tile target = path_[pathIndex_];
        if (!maze_.IsOpen(target.x, target.y)) return false;
        if (cur == target) return true;
        if (pathIndex_ == 0 || !(cur == path_[pathIndex_ - 1])) return false;
        return std::abs(target.x - cur.x) + std::abs(target.y - cur.y) == 1;
    }

    bool ActivePathValidForMode(Tile cur, bool freeRun) const {
        if (ActivePathValid(cur)) return true;
        if (!freeRun || pathIndex_ >= path_.size() || !maze_.IsOpen(cur.x, cur.y)) return false;
        Tile target = path_[pathIndex_];
        if (!OpenAreaAllowsFreeRun(cur) || !OpenAreaAllowsFreeRun(target)) return false;
        return maze_.LineClear(cur, target);
    }

    XMFLOAT3 PathLookAheadPoint(float lookAheadTiles) const {
        XMFLOAT3 current{camera_.x, camera_.y, camera_.z};
        if (pathIndex_ >= path_.size()) return current;

        float tile = std::max(maze_.TileMinimum(), 0.1f);
        float remaining = std::max(tile * 0.08f, std::max(0.0f, lookAheadTiles) * tile);
        XMFLOAT3 previous = current;
        size_t index = pathIndex_;
        Tile cameraTile = CameraTile();
        if (index < path_.size() && path_[index] == cameraTile && index + 1 < path_.size()) {
            ++index;
        }

        for (; index < path_.size(); ++index) {
            XMFLOAT3 center = maze_.WorldCenter(path_[index], camera_.y);
            float dx = center.x - previous.x;
            float dz = center.z - previous.z;
            float segment = std::sqrt(dx * dx + dz * dz);
            if (segment <= 0.001f) {
                previous = center;
                continue;
            }
            if (remaining <= segment) {
                float t = remaining / segment;
                return {Lerp(previous.x, center.x, t), camera_.y, Lerp(previous.z, center.z, t)};
            }
            remaining -= segment;
            previous = center;
        }
        return previous;
    }

    bool CameraFootprintOpen(float x, float z) const {
        Tile t = maze_.TileFromWorld(x, z);
        if (!maze_.IsOpen(t.x, t.y)) return false;

        float ox = -static_cast<float>(maze_.w) * maze_.tileW * 0.5f;
        float oz = -static_cast<float>(maze_.h) * maze_.tileD * 0.5f;
        float localX = (x - (ox + static_cast<float>(t.x) * maze_.tileW));
        float localZ = (z - (oz + static_cast<float>(t.y) * maze_.tileD));
        float margin = std::clamp(maze_.TileMinimum() * 0.26f, 0.12f, 0.46f);

        if (localX < margin && !maze_.IsOpen(t.x - 1, t.y)) return false;
        if (maze_.tileW - localX < margin && !maze_.IsOpen(t.x + 1, t.y)) return false;
        if (localZ < margin && !maze_.IsOpen(t.x, t.y - 1)) return false;
        if (maze_.tileD - localZ < margin && !maze_.IsOpen(t.x, t.y + 1)) return false;
        if (localX < margin && localZ < margin && !maze_.IsOpen(t.x - 1, t.y - 1)) return false;
        if (maze_.tileW - localX < margin && localZ < margin && !maze_.IsOpen(t.x + 1, t.y - 1)) return false;
        if (localX < margin && maze_.tileD - localZ < margin && !maze_.IsOpen(t.x - 1, t.y + 1)) return false;
        if (maze_.tileW - localX < margin && maze_.tileD - localZ < margin && !maze_.IsOpen(t.x + 1, t.y + 1)) return false;
        return true;
    }

    bool CameraSegmentOpen(float fromX, float fromZ, float toX, float toZ, Tile allowedStart, Tile allowedTarget) const {
        float dx = toX - fromX;
        float dz = toZ - fromZ;
        float len = std::sqrt(dx * dx + dz * dz);
        int steps = std::max(1, static_cast<int>(std::ceil(len / std::max(0.05f, maze_.TileMinimum() * 0.08f))));
        Tile prev = maze_.TileFromWorld(fromX, fromZ);
        if (!maze_.IsOpen(prev.x, prev.y)) return false;
        if (!(prev == allowedStart) && !(prev == allowedTarget)) return false;
        for (int i = 1; i <= steps; ++i) {
            float t = static_cast<float>(i) / static_cast<float>(steps);
            float sx = Lerp(fromX, toX, t);
            float sz = Lerp(fromZ, toZ, t);
            Tile cur = maze_.TileFromWorld(sx, sz);
            if (!(cur == allowedStart) && !(cur == allowedTarget)) return false;
            if (!(cur == prev)) {
                if (!maze_.IsOpen(cur.x, cur.y)) return false;
                if (!AdjacentTiles(prev, cur)) return false;
                prev = cur;
            }
            if (!CameraFootprintOpen(sx, sz)) return false;
        }
        return true;
    }

    bool CameraSegmentOpenThroughOpen(float fromX, float fromZ, float toX, float toZ, bool allowDiagonal) const {
        float dx = toX - fromX;
        float dz = toZ - fromZ;
        float len = std::sqrt(dx * dx + dz * dz);
        int steps = std::max(1, static_cast<int>(std::ceil(len / std::max(0.05f, maze_.TileMinimum() * 0.07f))));
        Tile prev = maze_.TileFromWorld(fromX, fromZ);
        if (!maze_.IsOpen(prev.x, prev.y) || !CameraFootprintOpen(fromX, fromZ)) return false;
        for (int i = 1; i <= steps; ++i) {
            float t = static_cast<float>(i) / static_cast<float>(steps);
            float sx = Lerp(fromX, toX, t);
            float sz = Lerp(fromZ, toZ, t);
            Tile cur = maze_.TileFromWorld(sx, sz);
            if (!maze_.IsOpen(cur.x, cur.y)) return false;
            if (!(cur == prev)) {
                int stepX = cur.x - prev.x;
                int stepY = cur.y - prev.y;
                if (std::abs(stepX) > 1 || std::abs(stepY) > 1) return false;
                if (std::abs(stepX) == 1 && std::abs(stepY) == 1) {
                    if (!allowDiagonal) return false;
                    if (!maze_.IsOpen(prev.x + stepX, prev.y) || !maze_.IsOpen(prev.x, prev.y + stepY)) return false;
                    if (!OpenAreaAllowsFreeRun(prev) || !OpenAreaAllowsFreeRun(cur)) return false;
                }
                prev = cur;
            }
            if (!CameraFootprintOpen(sx, sz)) return false;
        }
        return true;
    }

    bool RecoverCameraToOpenTile() {
        Tile cur = CameraTile();
        Tile best = cur;
        float bestScore = std::numeric_limits<float>::infinity();
        if (!maze_.IsOpen(best.x, best.y)) {
            for (int y = 1; y < maze_.h - 1; ++y) {
                for (int x = 1; x < maze_.w - 1; ++x) {
                    if (!maze_.IsOpen(x, y)) continue;
                    XMFLOAT3 c = maze_.WorldCenter({x, y}, camera_.y);
                    float dx = c.x - camera_.x;
                    float dz = c.z - camera_.z;
                    float score = dx * dx + dz * dz;
                    if (score < bestScore) {
                        bestScore = score;
                        best = {x, y};
                    }
                }
            }
        }
        if (!maze_.IsOpen(best.x, best.y)) return false;

        XMFLOAT3 center = maze_.WorldCenter(best, camera_.y);
        camera_.x = center.x;
        camera_.z = center.z;
        if (!(best == lastTile_) && AdjacentTiles(best, lastTile_)) {
            previousTile_ = lastTile_;
        }
        lastTile_ = best;
        path_.clear();
        pathIndex_ = 0;
        return true;
    }

    bool MoveCameraSafely(float stepX, float stepZ, float moveDistance, float speed, Tile allowedTarget, bool freeRun = false) {
        if (moveDistance <= 0.0001f) return true;
        Tile startTile = CameraTile();
        float nextX = camera_.x + stepX;
        float nextZ = camera_.z + stepZ;
        if (freeRun && CameraSegmentOpenThroughOpen(camera_.x, camera_.z, nextX, nextZ, true)) {
            camera_.x = nextX;
            camera_.z = nextZ;
            AdvanceStepPhase(moveDistance, speed);
            return true;
        }
        if (CameraSegmentOpen(camera_.x, camera_.z, nextX, nextZ, startTile, allowedTarget)) {
            camera_.x = nextX;
            camera_.z = nextZ;
            AdvanceStepPhase(moveDistance, speed);
            return true;
        }

        auto tryAxisMove = [&](float sx, float sz) {
            float axisDistance = std::sqrt(sx * sx + sz * sz);
            if (axisDistance <= 0.0001f) return false;
            float ax = camera_.x + sx;
            float az = camera_.z + sz;
            if (!CameraSegmentOpen(camera_.x, camera_.z, ax, az, startTile, allowedTarget)) return false;
            camera_.x = ax;
            camera_.z = az;
            AdvanceStepPhase(axisDistance, speed);
            return true;
        };

        if (std::abs(stepX) > 0.0001f && std::abs(stepZ) > 0.0001f) {
            bool tryZFirst = std::abs(stepZ) < std::abs(stepX);
            if (tryZFirst) {
                if (tryAxisMove(0.0f, stepZ)) return true;
                if (tryAxisMove(stepX, 0.0f)) return true;
            } else {
                if (tryAxisMove(stepX, 0.0f)) return true;
                if (tryAxisMove(0.0f, stepZ)) return true;
            }
        }

        Tile cur = CameraTile();
        if (!maze_.IsOpen(cur.x, cur.y)) {
            RecoverCameraToOpenTile();
            return false;
        }

        XMFLOAT3 center = maze_.WorldCenter(cur, camera_.y);
        float cx = center.x - camera_.x;
        float cz = center.z - camera_.z;
        float centerDist = std::sqrt(cx * cx + cz * cz);
        bool recentered = false;
        if (centerDist > 0.001f) {
            float recenter = std::min(moveDistance, centerDist);
            float rx = camera_.x + cx / centerDist * recenter;
            float rz = camera_.z + cz / centerDist * recenter;
            if (CameraSegmentOpen(camera_.x, camera_.z, rx, rz, cur, cur)) {
                camera_.x = rx;
                camera_.z = rz;
                AdvanceStepPhase(recenter, speed);
                recentered = true;
            }
        }
        if (recentered) return true;
        path_.clear();
        pathIndex_ = 0;
        return false;
    }

    float BranchLookWeight() const {
        if (branchLookTimer_ <= 0.0f || branchLookDuration_ <= 0.001f) return 0.0f;
        float t = 1.0f - branchLookTimer_ / branchLookDuration_;
        return SmoothStep(0.0f, 0.16f, t) * (1.0f - SmoothStep(0.86f, 1.0f, t));
    }

    float BranchLookTargetYaw() const {
        float weight = BranchLookWeight();
        float scan = (std::sin(time_ * 4.7f + branchLookYaw_ * 1.3f) * 0.020f +
            std::sin(time_ * 8.9f + branchLookYaw_) * 0.009f) * weight;
        return branchLookYaw_ + scan;
    }

    float RoomSurveyWeight() const {
        if (roomSurveyTimer_ <= 0.0f || roomSurveyDuration_ <= 0.001f) return 0.0f;
        float t = 1.0f - roomSurveyTimer_ / roomSurveyDuration_;
        return SmoothStep(0.0f, 0.18f, t) * (1.0f - SmoothStep(0.90f, 1.0f, t));
    }

    int RoomSurveyIndex(float& localT) const {
        localT = 0.0f;
        if (roomSurveyYawCount_ <= 0 || roomSurveyTimer_ <= 0.0f || roomSurveyDuration_ <= 0.001f) return -1;
        float t = Clamp01(1.0f - roomSurveyTimer_ / roomSurveyDuration_);
        float segment = t * static_cast<float>(roomSurveyYawCount_);
        int index = std::min(roomSurveyYawCount_ - 1, static_cast<int>(segment));
        localT = segment - static_cast<float>(index);
        return index;
    }

    float RoomSurveyYaw() const {
        if (roomSurveyTimer_ <= 0.0f || roomSurveyDuration_ <= 0.001f) return yaw_;
        float localT = 0.0f;
        int index = RoomSurveyIndex(localT);
        if (index < 0) {
            float t = 1.0f - roomSurveyTimer_ / roomSurveyDuration_;
            float sweep = -std::cos(Clamp01(t) * kPi * 2.0f);
            return roomSurveyCenter_ + roomSurveyDirection_ * sweep * roomSurveySpan_;
        }

        float target = roomSurveyYaws_[static_cast<size_t>(index)];
        float from = index > 0 ? roomSurveyYaws_[static_cast<size_t>(index - 1)] : roomSurveyCenter_;
        float enter = SmoothStep(0.0f, 0.24f, localT);
        float yaw = from + AngleWrap(target - from) * enter;
        if (index + 1 < roomSurveyYawCount_) {
            float leave = SmoothStep(0.76f, 1.0f, localT);
            yaw += AngleWrap(roomSurveyYaws_[static_cast<size_t>(index + 1)] - yaw) * leave * 0.54f;
        }
        float inspect = (1.0f - SmoothStep(0.62f, 1.0f, localT)) * SmoothStep(0.18f, 0.52f, localT);
        yaw += std::sin(time_ * 4.1f + static_cast<float>(index) * 1.9f) * 0.026f * inspect;
        return yaw;
    }

    float RoomSurveyPitch() const {
        if (roomSurveyTimer_ <= 0.0f || roomSurveyDuration_ <= 0.001f || roomSurveyPitchCount_ <= 0) return -0.045f;
        float localT = 0.0f;
        int index = RoomSurveyIndex(localT);
        if (index < 0) return -0.045f;
        index = std::min(index, roomSurveyPitchCount_ - 1);
        float target = roomSurveyPitches_[static_cast<size_t>(index)];
        float from = index > 0 ? roomSurveyPitches_[static_cast<size_t>(index - 1)] : -0.045f;
        float enter = SmoothStep(0.0f, 0.28f, localT);
        return from + (target - from) * enter;
    }

    bool FindRoomPropFocus(Tile cur, XMFLOAT3& focus) const {
        if (propLookPoints_.empty()) return false;
        float bestScore = -1.0e9f;
        XMFLOAT3 best{};
        for (const XMFLOAT3& p : propLookPoints_) {
            float dx = p.x - camera_.x;
            float dz = p.z - camera_.z;
            float dist = std::sqrt(dx * dx + dz * dz);
            if (dist < 1.05f || dist > 8.5f) continue;
            Tile pt = maze_.TileFromWorld(p.x, p.z);
            if (!maze_.LineClear(cur, pt)) continue;
            float yawDelta = std::abs(AngleWrap(YawToPoint(p) - bodyYaw_));
            float closeInterest = SmoothStep(8.5f, 1.6f, dist);
            float sideInterest = SmoothStep(0.12f, 1.25f, yawDelta) * (1.0f - SmoothStep(2.65f, 3.14f, yawDelta));
            float heightInterest = 1.0f - Clamp01(std::abs(p.y - 0.95f) / 2.0f) * 0.22f;
            float repeatPenalty = 0.0f;
            if (hasLastPropLookTarget_) {
                float ldx = p.x - lastPropLookTarget_.x;
                float ldz = p.z - lastPropLookTarget_.z;
                if (ldx * ldx + ldz * ldz < 0.90f) repeatPenalty = 0.85f;
            }
            float score = closeInterest * 2.1f + sideInterest * 1.1f + heightInterest -
                std::min(yawDelta, 2.8f) * 0.10f - repeatPenalty;
            if (score > bestScore) {
                bestScore = score;
                best = p;
            }
        }
        if (bestScore < 1.35f) return false;
        focus = best;
        return true;
    }

    void BeginRoomSurvey(Tile cur, bool pauseFirst) {
        if (!IsRoomSurveySpot(cur) || DreadPressure() > 0.42f || ChasePanicActive() || IsThreatVisible() ||
            (!pauseFirst && roomSurveyCooldown_ > 0.0f)) {
            return;
        }

        roomSurveyCenter_ = bodyYaw_;
        roomSurveySpan_ = std::clamp(0.52f + static_cast<float>(maze_.LocalOpenCount(cur, 2)) * 0.030f, 0.62f, 1.18f);
        roomSurveyDirection_ = RandRange(0.0f, 1.0f) < 0.5f ? -1.0f : 1.0f;
        roomSurveyYawCount_ = 0;
        roomSurveyPitchCount_ = 0;

        struct SurveyCandidate {
            float score;
            float yaw;
            float pitch;
        };
        std::vector<SurveyCandidate> candidates;
        candidates.reserve(8);
        Tile previous = HasPreviousMovementTile(cur) ? previousTile_ : Tile{-1000, -1000};
        Tile nextTarget = pathIndex_ < path_.size() ? path_[pathIndex_] : cur;
        const Tile dirs[] = {{1, 0}, {-1, 0}, {0, 1}, {0, -1}};
        for (Tile d : dirs) {
            Tile n{cur.x + d.x, cur.y + d.y};
            if (!maze_.IsOpen(n.x, n.y)) continue;
            XMFLOAT3 nw = maze_.WorldCenter(n, camera_.y);
            float branchYaw = std::atan2(nw.x - camera_.x, nw.z - camera_.z);
            float ray = ViewRayOpenDistance(branchYaw, std::clamp(settings_.fogEndMeters, 6.0f, 18.0f));
            if (ray < maze_.TileMinimum() * 0.92f) continue;
            float rel = std::abs(AngleWrap(branchYaw - bodyYaw_));
            float score = ray * 0.80f + static_cast<float>(maze_.LocalOpenCount(n, 2)) * 0.16f +
                SmoothStep(0.18f, 1.55f, rel) * 1.15f;
            if (n == nextTarget) score -= 1.25f;
            if (n == previous) score -= 0.70f;
            candidates.push_back({score + RandRange(-0.25f, 0.25f), branchYaw, -0.040f});
        }

        XMFLOAT3 propFocus{};
        if (FindRoomPropFocus(cur, propFocus) && RandRange(0.0f, 1.0f) < 0.72f) {
            candidates.push_back({7.0f + RandRange(-0.35f, 0.35f), YawToPoint(propFocus),
                std::clamp(PitchToPoint(propFocus), -0.32f, 0.20f)});
            lastPropLookTarget_ = propFocus;
            hasLastPropLookTarget_ = true;
        }

        std::sort(candidates.begin(), candidates.end(), [](const SurveyCandidate& a, const SurveyCandidate& b) {
            return a.score > b.score;
        });
        int maxTargets = pauseFirst ? 5 : 3;
        int selected = std::min<int>(static_cast<int>(candidates.size()), maxTargets);
        if (selected > 0) {
            std::sort(candidates.begin(), candidates.begin() + selected, [this](const SurveyCandidate& a, const SurveyCandidate& b) {
                return AngleWrap(a.yaw - bodyYaw_) < AngleWrap(b.yaw - bodyYaw_);
            });
            if (RandRange(0.0f, 1.0f) < 0.5f) {
                std::reverse(candidates.begin(), candidates.begin() + selected);
            }
            for (int i = 0; i < selected; ++i) {
                roomSurveyYaws_[static_cast<size_t>(i)] = candidates[static_cast<size_t>(i)].yaw;
                roomSurveyPitches_[static_cast<size_t>(i)] = candidates[static_cast<size_t>(i)].pitch;
            }
            roomSurveyYawCount_ = selected;
            roomSurveyPitchCount_ = selected;
        }

        float targetBonus = static_cast<float>(std::max(0, roomSurveyYawCount_ - 1)) * (pauseFirst ? 0.32f : 0.20f);
        roomSurveyDuration_ = (pauseFirst ? RandRange(1.95f, 2.70f) : RandRange(1.18f, 1.85f)) + targetBonus;
        roomSurveyTimer_ = roomSurveyDuration_;
        roomSurveyCooldown_ = RandRange(4.5f, 9.5f);
        branchLookTimer_ = 0.0f;
        branchLookPaused_ = false;
        if (pauseFirst) {
            stopTimer_ = std::max(stopTimer_, roomSurveyDuration_ * RandRange(0.66f, 0.86f));
            headScanTimer_ = 0.0f;
            headScanDuration_ = 0.0f;
            junctionScanActive_ = false;
            lookBack_ = false;
            propLookTimer_ = 0.0f;
        }
    }

    bool BeginBranchLook(Tile cur, Tile previous, Tile nextTarget, bool allowPause = false, bool allowRoomTile = false) {
        if (!maze_.IsOpen(cur.x, cur.y) || (!allowRoomTile && IsRoomLike(cur)) || DreadPressure() > 0.36f ||
            ChasePanicActive() || IsThreatVisible() || branchLookTimer_ > 0.0f || roomSurveyTimer_ > 0.0f ||
            branchLookCooldown_ > 0.0f || cur == lastBranchLookTile_) {
            return false;
        }

        const Tile dirs[] = {{1, 0}, {-1, 0}, {0, 1}, {0, -1}};
        Tile monsterTile = MonsterTile();
        float bestScore = -1.0e9f;
        float bestYaw = yaw_;
        int candidates = 0;
        for (Tile d : dirs) {
            Tile n{cur.x + d.x, cur.y + d.y};
            if (!maze_.IsOpen(n.x, n.y)) continue;
            if (n == previous || n == nextTarget) continue;
            XMFLOAT3 nw = maze_.WorldCenter(n, camera_.y);
            float branchYaw = std::atan2(nw.x - camera_.x, nw.z - camera_.z);
            float ray = ViewRayOpenDistance(branchYaw, std::clamp(settings_.fogEndMeters, 6.0f, 18.0f));
            if (ray < maze_.TileMinimum() * 0.95f) continue;
            bool possibleThreatLine = MonsterDistance() < 18.0f && maze_.LineClear(n, monsterTile);
            float score = ray + static_cast<float>(maze_.LocalOpenCount(n, 1)) * 0.18f +
                (possibleThreatLine ? 5.5f : 0.0f) - std::abs(AngleWrap(branchYaw - yaw_)) * 0.35f;
            ++candidates;
            if (score > bestScore) {
                bestScore = score;
                bestYaw = branchYaw;
            }
        }
        if (candidates <= 0) return false;
        branchLookYaw_ = bestYaw;
        branchLookPitch_ = -0.045f + RandRange(-0.012f, 0.018f);
        float yawDelta = std::abs(AngleWrap(branchLookYaw_ - yaw_));
        branchLookDuration_ = RandRange(1.34f, 2.02f) + SmoothStep(0.35f, 1.55f, yawDelta) * 0.50f;
        branchLookTimer_ = branchLookDuration_;
        branchLookCooldown_ = RandRange(2.2f, 5.4f);
        lastBranchLookTile_ = cur;
        branchLookPaused_ = allowPause && RandRange(0.0f, 1.0f) < 0.72f;
        if (branchLookPaused_) {
            stopTimer_ = std::max(stopTimer_, branchLookDuration_ * RandRange(0.42f, 0.66f));
            headScanTimer_ = 0.0f;
            headScanDuration_ = 0.0f;
            junctionScanActive_ = false;
            lookBack_ = false;
            propLookTimer_ = 0.0f;
        }
        return true;
    }

    bool StraightCorridorTravelYaw(Tile cameraTile, float& corridorYaw) const {
        corridorYaw = yaw_;
        if (!IsStraightCorridor(cameraTile) || pathIndex_ >= path_.size()) return false;

        size_t startIndex = pathIndex_;
        Tile next = path_[startIndex];
        if (next == cameraTile) {
            if (startIndex + 1 >= path_.size()) return false;
            ++startIndex;
            next = path_[startIndex];
        }
        if (!AdjacentTiles(cameraTile, next)) return false;

        int dirX = next.x - cameraTile.x;
        int dirY = next.y - cameraTile.y;
        Tile previous = cameraTile;
        int straightSteps = 0;
        const size_t limit = std::min(path_.size(), startIndex + 3);
        for (size_t i = startIndex; i < limit; ++i) {
            Tile step = path_[i];
            if (!AdjacentTiles(previous, step)) return false;
            int stepX = step.x - previous.x;
            int stepY = step.y - previous.y;
            if (stepX != dirX || stepY != dirY) return false;
            previous = step;
            ++straightSteps;
        }
        if (straightSteps < 2) return false;

        XMFLOAT3 a = maze_.WorldCenter(cameraTile, camera_.y);
        XMFLOAT3 b = maze_.WorldCenter(next, camera_.y);
        corridorYaw = std::atan2(b.x - a.x, b.z - a.z);
        return true;
    }

    bool FindUpcomingCorridorTurn(Tile cameraTile, float& turnYaw, float& turnWeight) const {
        turnYaw = yaw_;
        turnWeight = 0.0f;
        if (!IsCorridorLike(cameraTile) || pathIndex_ >= path_.size() || path_.size() < 2) return false;

        const float tile = std::max(maze_.TileMinimum(), 0.1f);
        const size_t lastCandidate = std::min(path_.size() - 2, pathIndex_ + 4);
        Tile previous = cameraTile;
        for (size_t i = pathIndex_; i <= lastCandidate; ++i) {
            Tile turn = path_[i];
            if (turn == previous) continue;
            if (!AdjacentTiles(previous, turn)) break;

            Tile next = path_[i + 1];
            if (!AdjacentTiles(turn, next)) break;

            int inX = turn.x - previous.x;
            int inY = turn.y - previous.y;
            int outX = next.x - turn.x;
            int outY = next.y - turn.y;
            if (inX == outX && inY == outY) {
                previous = turn;
                continue;
            }

            if (outX == -inX && outY == -inY) return false;

            XMFLOAT3 turnCenter = maze_.WorldCenter(turn, camera_.y);
            float dx = turnCenter.x - camera_.x;
            float dz = turnCenter.z - camera_.z;
            float distToTurn = std::sqrt(dx * dx + dz * dz);
            float weight = SmoothStep(tile * 1.42f, tile * 0.22f, distToTurn);
            float futureFade = 1.0f - std::min(static_cast<float>(i - pathIndex_), 3.0f) * 0.16f;
            turnWeight = weight * futureFade * 0.46f;
            if (turnWeight <= 0.001f) return false;

            XMFLOAT3 nextCenter = maze_.WorldCenter(next, camera_.y);
            turnYaw = std::atan2(nextCenter.x - turnCenter.x, nextCenter.z - turnCenter.z);
            return true;
        }
        return false;
    }

    bool BeginJunctionScan(Tile cur, Tile previous) {
        if (!IsTightTJunction(cur, previous)) return false;

        std::vector<std::pair<float, float>> branches;
        const int dirs[4][2] = {{1, 0}, {-1, 0}, {0, 1}, {0, -1}};
        Tile monsterTile = MonsterTile();
        for (auto& d : dirs) {
            Tile n{cur.x + d[0], cur.y + d[1]};
            if (!maze_.IsOpen(n.x, n.y)) continue;
            if (n == previous) continue;
            XMFLOAT3 nw = maze_.WorldCenter(n, camera_.y);
            float branchYaw = std::atan2(nw.x - camera_.x, nw.z - camera_.z);
            float rel = AngleWrap(branchYaw - bodyYaw_);
            bool branchThreat = maze_.LineClear(n, monsterTile) && MonsterDistance() < 18.0f;
            float order = branchThreat ? -10.0f + std::abs(rel) * 0.01f : rel;
            branches.push_back({order, branchYaw});
        }

        if (branches.size() < 2) return false;
        std::sort(branches.begin(), branches.end(), [](const auto& a, const auto& b) {
            return a.first < b.first;
        });

        junctionScanCount_ = std::min<int>(static_cast<int>(branches.size()), static_cast<int>(junctionScanYaws_.size()));
        for (int i = 0; i < junctionScanCount_; ++i) {
            junctionScanYaws_[static_cast<size_t>(i)] = branches[static_cast<size_t>(i)].second;
        }

        junctionScanActive_ = true;
        junctionScanTile_ = cur;
        float baseSeconds = std::max(0.72f, settings_.junctionScanBaseSeconds);
        float branchSeconds = std::max(0.96f, settings_.junctionScanBranchSeconds);
        stopTimer_ = baseSeconds +
            static_cast<float>(junctionScanCount_) * RandRange(branchSeconds * 0.92f, branchSeconds * 1.16f);
        headScanDuration_ = stopTimer_;
        headScanTimer_ = stopTimer_;
        headScanCenter_ = bodyYaw_;
        lookBack_ = false;
        path_.clear();
        pathIndex_ = 0;
        return true;
    }

    static float Frac(float v) {
        return v - std::floor(v);
    }

    static float LampHash(float x, float z) {
        float px = Frac(x * 123.34f);
        float pz = Frac(z * 456.21f);
        float d = px * (px + 45.32f) + pz * (pz + 45.32f);
        px += d;
        pz += d;
        return Frac(px * pz);
    }

    float LampSeed(int cellX, int cellZ) const {
        return LampHash(static_cast<float>(cellX), static_cast<float>(cellZ));
    }

    bool LampBrokenZone(int cellX, int cellZ) const {
        float zx = std::floor(static_cast<float>(cellX) / 3.0f);
        float zz = std::floor(static_cast<float>(cellZ) / 3.0f);
        return LampHash(zx, zz) >= 1.0f - settings_.brokenZoneRatio;
    }

    bool LampIsOn(int cellX, int cellZ) const {
        return !LampBrokenZone(cellX, cellZ) && LampSeed(cellX, cellZ) >= 1.0f - settings_.lampOnRatio;
    }

    bool VisibleInFront(Tile target) const {
        Tile cur = CameraTile();
        if (!maze_.LineClear(cur, target)) return false;
        XMFLOAT3 tw = maze_.WorldCenter(target, camera_.y);
        XMFLOAT3 f = Forward();
        float dx = tw.x - camera_.x;
        float dz = tw.z - camera_.z;
        float len = std::sqrt(dx * dx + dz * dz);
        if (len < 0.01f) return true;
        return (dx * f.x + dz * f.z) / len > 0.38f;
    }

    float ViewRayOpenDistance(float yaw, float maxMeters) const {
        XMFLOAT3 dir{std::sin(yaw), 0.0f, std::cos(yaw)};
        float step = std::clamp(maze_.TileMinimum() * 0.16f, 0.10f, 0.26f);
        float lastOpen = 0.0f;
        for (float d = step; d <= maxMeters; d += step) {
            float x = camera_.x + dir.x * d;
            float z = camera_.z + dir.z * d;
            if (!CameraFootprintOpen(x, z)) break;
            lastOpen = d;
        }
        return lastOpen;
    }

    void UpdatePropLook(float dt, bool threat) {
        if (threat || propLookPoints_.empty()) {
            propLookTimer_ = 0.0f;
            return;
        }

        propLookCooldown_ = std::max(0.0f, propLookCooldown_ - dt);
        if (propLookTimer_ > 0.0f) {
            propLookTimer_ = std::max(0.0f, propLookTimer_ - dt);
            return;
        }
        if (propLookCooldown_ > 0.0f) return;

        XMFLOAT3 viewDir = Normalize3(DirectionFromYawPitch(yaw_, lookPitch_), {0.0f, 0.0f, 1.0f});
        Tile cur = CameraTile();
        XMFLOAT3 best{};
        float bestScore = -1.0f;
        for (const XMFLOAT3& p : propLookPoints_) {
            float dx = p.x - camera_.x;
            float dy = p.y - camera_.y;
            float dz = p.z - camera_.z;
            float dist = std::sqrt(dx * dx + dz * dz);
            if (dist < 1.05f || dist > 8.8f) continue;
            XMFLOAT3 toProp = Normalize3({dx, dy, dz}, viewDir);
            float viewDot = Dot3(toProp, viewDir);
            if (viewDot < 0.88f) continue;
            float yawFromCamera = std::abs(AngleWrap(YawToPoint(p) - yaw_));
            float pitchFromCamera = std::abs(PitchToPoint(p) - lookPitch_);
            constexpr float kInnerYaw = 0.42f;
            constexpr float kInnerPitch = 0.27f;
            if (yawFromCamera > kInnerYaw || pitchFromCamera > kInnerPitch) continue;
            float rimPenalty = SmoothStep(0.58f, 1.0f, std::max(yawFromCamera / kInnerYaw, pitchFromCamera / kInnerPitch));
            Tile pt = maze_.TileFromWorld(p.x, p.z);
            if (!maze_.LineClear(cur, pt)) continue;
            float yawDelta = std::abs(AngleWrap(YawToPoint(p) - flashlightYaw_));
            float closeInterest = SmoothStep(8.0f, 1.4f, dist);
            float verticalInterest = 1.0f - Clamp01(std::abs(dy) / 2.0f) * 0.18f;
            float repeatPenalty = 0.0f;
            if (hasLastPropLookTarget_) {
                float ldx = p.x - lastPropLookTarget_.x;
                float ldz = p.z - lastPropLookTarget_.z;
                if (ldx * ldx + ldz * ldz < 0.70f) repeatPenalty = 1.45f;
            }
            float score = viewDot * 1.25f + closeInterest * 1.15f + verticalInterest -
                rimPenalty * 1.65f - std::min(yawDelta, 1.5f) * 0.18f - repeatPenalty + RandRange(-0.24f, 0.16f);
            if (score > bestScore) {
                bestScore = score;
                best = p;
            }
        }

        if (bestScore > 1.35f && RandRange(0.0f, 1.0f) < 0.48f) {
            propLookTarget_ = best;
            propLookDuration_ = RandRange(0.92f, 1.85f);
            propLookTimer_ = propLookDuration_;
            propLookCooldown_ = RandRange(1.15f, 3.35f);
            propLookScanSeed_ = RandRange(0.0f, 1.0f);
            lastPropLookTarget_ = best;
            hasLastPropLookTarget_ = true;
        } else {
            propLookCooldown_ = RandRange(0.90f, 2.25f);
        }
    }

    bool ExitRouteNotBlockedByMonster() const {
        XMFLOAT3 exitWorld = maze_.WorldCenter(maze_.exit, camera_.y);
        float ex = exitWorld.x - camera_.x;
        float ez = exitWorld.z - camera_.z;
        float mx = monster_.x - camera_.x;
        float mz = monster_.z - camera_.z;
        float exitDist = std::sqrt(ex * ex + ez * ez);
        float monsterDist = std::sqrt(mx * mx + mz * mz);
        if (exitDist < 0.2f) return true;
        if (monsterDist < 0.2f) return false;
        float alignment = (ex * mx + ez * mz) / (exitDist * monsterDist);
        return !(monsterDist < exitDist + maze_.TileAverage() * 0.8f && alignment > 0.66f);
    }

    float ExitAttentionWeight(XMFLOAT3& focus) const {
        focus = exitSignLightStrength_ > 0.001f
            ? exitSignLightPos_
            : Add3(exitDoorCenter_, {0.0f, 0.72f, 0.0f});
        if (exitTransitionActive_ || deathActive_) return 0.0f;
        Tile cur = CameraTile();
        if (!maze_.IsOpen(cur.x, cur.y) || !maze_.IsOpen(maze_.exit.x, maze_.exit.y)) return 0.0f;
        if (!maze_.LineClear(cur, maze_.exit)) return 0.0f;

        float tile = std::max(maze_.TileAverage(), 0.1f);
        XMFLOAT3 signFocus = focus;
        XMFLOAT3 doorFocus = Add3(exitDoorCenter_, {0.0f, 0.08f, 0.0f});
        float dx = signFocus.x - camera_.x;
        float dz = signFocus.z - camera_.z;
        float dist = std::sqrt(dx * dx + dz * dz);
        float maxDist = std::clamp(tile * 6.8f, 7.0f, 12.5f);
        if (dist > maxDist) return 0.0f;

        bool routingToExit = !path_.empty() && path_.back() == maze_.exit;
        float yawDelta = std::abs(AngleWrap(YawToPoint(signFocus) - yaw_));
        float yawWindow = routingToExit || exitSpotted_ ? 2.35f : 1.75f;
        float facing = SmoothStep(yawWindow, 0.22f, yawDelta);
        if (facing <= 0.001f) return 0.0f;

        float close = SmoothStep(maxDist, tile * 1.15f, dist);
        float doorBlend = SmoothStep(tile * 3.0f, tile * 0.70f, dist) * 0.72f;
        focus = Lerp3(signFocus, doorFocus, doorBlend);
        float lock = SmoothStep(tile * 2.7f, tile * 0.70f, dist);
        float routeScale = routingToExit ? 1.0f : (exitSpotted_ ? 0.88f : 0.72f);
        float maxWeight = Lerp(0.48f, 0.82f, lock);
        return std::clamp(close * facing * routeScale * maxWeight, 0.0f, maxWeight);
    }

    float TileDistanceSq(Tile a, Tile b) const {
        float dx = static_cast<float>(a.x - b.x);
        float dy = static_cast<float>(a.y - b.y);
        return dx * dx + dy * dy;
    }

    float PathExplorationScore(const std::vector<Tile>& path, bool fleeing) const {
        if (path.size() < 2) return 0.0f;
        float score = 0.0f;
        Tile start = path.front();
        Tile end = path.back();
        int limit = std::min<int>(static_cast<int>(path.size()), fleeing ? 14 : 22);
        for (int i = 1; i < limit; ++i) {
            Tile t = path[static_cast<size_t>(i)];
            uint16_t visits = VisitCount(t);
            float stepWeight = fleeing
                ? std::max(0.25f, 1.0f - static_cast<float>(i - 1) * 0.055f)
                : std::max(0.20f, 1.0f - static_cast<float>(i - 1) * 0.035f);
            score += stepWeight * (visits == 0 ? 34.0f : (visits == 1 ? 12.0f : -8.0f * std::min<int>(visits, 6)));
            score += static_cast<float>(maze_.LocalOpenCount(t, 1)) * (fleeing ? 1.85f : 1.25f) * stepWeight;
        }
        float exitProgress = TileDistanceSq(start, maze_.exit) - TileDistanceSq(end, maze_.exit);
        score += exitProgress * (fleeing ? 1.80f : 1.15f);
        if (path.size() > 1 && IsBacktrackingStep(start, path[1])) {
            score += fleeing ? 35.0f : -160.0f;
        }
        if (end == maze_.exit) score += fleeing ? 1200.0f : 1800.0f;
        return score;
    }

    static bool AdjacentTiles(Tile a, Tile b) {
        return std::abs(a.x - b.x) + std::abs(a.y - b.y) == 1;
    }

    bool HasPreviousMovementTile(Tile cur) const {
        return maze_.IsOpen(previousTile_.x, previousTile_.y) && AdjacentTiles(cur, previousTile_);
    }

    bool IsBacktrackingStep(Tile cur, Tile step) const {
        return HasPreviousMovementTile(cur) && step == previousTile_;
    }

    XMFLOAT3 NavigationForward(Tile cur) const {
        if (HasPreviousMovementTile(cur)) {
            XMFLOAT3 from = maze_.WorldCenter(previousTile_, camera_.y);
            XMFLOAT3 to = maze_.WorldCenter(cur, camera_.y);
            return Normalize3({to.x - from.x, 0.0f, to.z - from.z}, {std::sin(bodyYaw_), 0.0f, std::cos(bodyYaw_)});
        }
        if (pathIndex_ < path_.size()) {
            Tile target = path_[pathIndex_];
            if (maze_.IsOpen(target.x, target.y) && !(target == cur)) {
                XMFLOAT3 to = maze_.WorldCenter(target, camera_.y);
                return Normalize3({to.x - camera_.x, 0.0f, to.z - camera_.z}, {std::sin(bodyYaw_), 0.0f, std::cos(bodyYaw_)});
            }
        }
        return {std::sin(bodyYaw_), 0.0f, std::cos(bodyYaw_)};
    }

    bool BuildCorridorContinuationPath(Tile cur) {
        if (!maze_.IsOpen(cur.x, cur.y)) return false;
        if (cur == maze_.exit) return false;

        std::vector<Tile> neighbors = maze_.Neighbors(cur);
        if (neighbors.size() < 2 || maze_.OpenNeighborCount(cur) > 2) return false;

        Tile previous = previousTile_;
        bool hasPrevious = HasPreviousMovementTile(cur);
        std::vector<Tile> candidates;
        candidates.reserve(neighbors.size());
        for (Tile n : neighbors) {
            if (hasPrevious && n == previous) continue;
            candidates.push_back(n);
        }
        if (candidates.empty()) return false;

        Tile best = candidates.front();
        float bestScore = -1.0e9f;
        for (Tile n : candidates) {
            float score = static_cast<float>(maze_.LocalOpenCount(n, 1)) * 0.08f + RandRange(-0.03f, 0.03f);
            score += VisitCount(n) == 0 ? 5.5f : -static_cast<float>(std::min<int>(VisitCount(n), 5)) * 3.0f;
            score += (TileDistanceSq(cur, maze_.exit) - TileDistanceSq(n, maze_.exit)) * 0.55f;
            if (score > bestScore) {
                bestScore = score;
                best = n;
            }
        }

        std::vector<Tile> built;
        built.reserve(18);
        built.push_back(cur);
        Tile prev = cur;
        Tile next = best;
        for (int i = 0; i < 16; ++i) {
            built.push_back(next);
            if (next == maze_.exit) break;
            std::vector<Tile> ns = maze_.Neighbors(next);
            std::vector<Tile> onward;
            onward.reserve(ns.size());
            for (Tile n : ns) {
                if (!(n == prev)) onward.push_back(n);
            }
            if (maze_.OpenNeighborCount(next) != 2 || onward.size() != 1) break;
            prev = next;
            next = onward.front();
        }

        if (built.size() < 2) return false;
        path_ = std::move(built);
        pathIndex_ = 1;
        return true;
    }

    int FirstThreatLineBreakIndex(const std::vector<Tile>& path, Tile monsterTile, int limit = 9999) const {
        int count = std::min<int>(static_cast<int>(path.size()), limit + 1);
        for (int i = 1; i < count; ++i) {
            if (!maze_.LineClear(path[static_cast<size_t>(i)], monsterTile)) return i;
        }
        return -1;
    }

    int FirstBranchIndex(const std::vector<Tile>& path, int limit = 9999) const {
        int count = std::min<int>(static_cast<int>(path.size()), limit + 1);
        for (int i = 1; i < count; ++i) {
            Tile t = path[static_cast<size_t>(i)];
            if (maze_.OpenNeighborCount(t) >= 3 || maze_.LocalOpenCount(t, 2) >= 13) return i;
        }
        return -1;
    }

    float StepTowardMonsterAmount(Tile step) const {
        XMFLOAT3 sw = maze_.WorldCenter(step, camera_.y);
        float stepX = sw.x - camera_.x;
        float stepZ = sw.z - camera_.z;
        float monX = monster_.x - camera_.x;
        float monZ = monster_.z - camera_.z;
        float stepLen = std::sqrt(stepX * stepX + stepZ * stepZ);
        float monLen = std::sqrt(monX * monX + monZ * monZ);
        if (stepLen <= 0.001f || monLen <= 0.001f) return -1.0f;
        return (stepX * monX + stepZ * monZ) / (stepLen * monLen);
    }

    bool FleeStepRiskyTowardMonster(Tile cur, Tile step, Tile monsterTile) const {
        if (step == cur) return false;
        if (!maze_.IsOpen(step.x, step.y)) return true;

        float startDist = TileDistanceSq(cur, monsterTile);
        float nextDist = TileDistanceSq(step, monsterTile);
        bool visibleFromStep = maze_.LineClear(step, monsterTile);
        float toward = StepTowardMonsterAmount(step);

        if (nextDist < startDist - 0.01f) return true;
        if (visibleFromStep && nextDist <= startDist + 0.01f) return true;
        if (visibleFromStep && toward > -0.08f) return true;
        return false;
    }

    bool HasSaferImmediateFleeStep(Tile cur, Tile monsterTile) const {
        for (Tile n : maze_.Neighbors(cur)) {
            if (!FleeStepRiskyTowardMonster(cur, n, monsterTile)) return true;
        }
        return false;
    }

    bool ActiveThreatPathShouldRepath(Tile cur, Tile monsterTile) const {
        if (pathIndex_ >= path_.size()) return false;

        std::vector<Tile> remaining;
        remaining.reserve(std::min<size_t>(path_.size() - pathIndex_ + 1, 10));
        remaining.push_back(cur);
        size_t start = pathIndex_;
        while (start < path_.size() && path_[start] == cur) {
            ++start;
        }
        for (size_t i = start; i < path_.size() && remaining.size() < 10; ++i) {
            if (remaining.back() == path_[i]) continue;
            remaining.push_back(path_[i]);
        }
        if (remaining.size() < 2) return false;
        if (ThreatPathMovesTowardMonster(remaining, monsterTile)) return true;
        return HasSaferImmediateFleeStep(cur, monsterTile) &&
            FleeStepRiskyTowardMonster(cur, remaining[1], monsterTile);
    }

    float FleePathScore(const std::vector<Tile>& path, Tile monsterTile) const {
        if (path.size() < 2) return -1.0e9f;
        float startDist = TileDistanceSq(path.front(), monsterTile);
        float firstDist = TileDistanceSq(path[1], monsterTile);
        float minEarly = firstDist;
        int earlyCount = std::min<int>(static_cast<int>(path.size()), 6);
        for (int i = 1; i < earlyCount; ++i) {
            minEarly = std::min(minEarly, TileDistanceSq(path[static_cast<size_t>(i)], monsterTile));
        }
        Tile end = path.back();
        float endDist = TileDistanceSq(end, monsterTile);
        int lineBreak = FirstThreatLineBreakIndex(path, monsterTile, 9);
        int branch = FirstBranchIndex(path, 9);
        int endOpen = maze_.OpenNeighborCount(end);
        float score = endDist * 2.8f + minEarly * 3.8f + std::min<float>(static_cast<float>(path.size()), 42.0f) * 1.15f;
        score += PathExplorationScore(path, true);
        if (firstDist > startDist + 0.01f) score += 360.0f;
        else score -= 620.0f;
        if (lineBreak >= 0) score += 310.0f - static_cast<float>(lineBreak) * 30.0f;
        else score -= 170.0f;
        if (branch >= 0) score += 185.0f - static_cast<float>(branch) * 18.0f;
        score += static_cast<float>(maze_.LocalOpenCount(end, 2)) * 6.2f;
        if (endOpen <= 1 && !(end == maze_.exit)) score -= lineBreak >= 0 ? 120.0f : 390.0f;
        if (firstDist < startDist - 0.01f) score -= (lineBreak >= 0 && lineBreak <= 3) ? 35.0f : 210.0f;
        if (minEarly < startDist - 0.01f) score -= (lineBreak >= 0 && lineBreak <= 4) ? 70.0f : 320.0f;
        if (HasSaferImmediateFleeStep(path.front(), monsterTile) &&
            FleeStepRiskyTowardMonster(path.front(), path[1], monsterTile)) {
            score -= 1150.0f;
        }
        return score;
    }

    bool ThreatPathMovesTowardMonster(const std::vector<Tile>& path, Tile monsterTile) const {
        if (path.size() < 2) return true;
        Tile cur = path.front();
        float startDist = TileDistanceSq(cur, monsterTile);
        float firstDist = TileDistanceSq(path[1], monsterTile);
        int lineBreak = FirstThreatLineBreakIndex(path, monsterTile, 5);
        int branch = FirstBranchIndex(path, 5);
        bool earlyEscape = (lineBreak >= 0 && lineBreak <= 4) || (branch >= 0 && branch <= 3);
        bool hasSaferStep = HasSaferImmediateFleeStep(cur, monsterTile);
        bool riskyFirstStep = FleeStepRiskyTowardMonster(cur, path[1], monsterTile);
        if (hasSaferStep && riskyFirstStep) return true;
        if (riskyFirstStep && !earlyEscape) return true;
        if (MonsterLineOfSightToPlayer() && firstDist <= startDist + 0.01f) return true;
        if (firstDist < startDist - 1.01f && !earlyEscape) return true;

        float toward = StepTowardMonsterAmount(path[1]);
        bool visibleFromFirst = maze_.LineClear(path[1], monsterTile) || MonsterLineOfSightToPlayer();
        if (visibleFromFirst) {
            if (toward > -0.10f && (hasSaferStep || !earlyEscape)) return true;
            if (toward > 0.20f && !earlyEscape) return true;
        }
        return false;
    }

    bool ForceImmediateFleeStep(Tile cur, Tile monsterTile) {
        std::vector<Tile> neighbors = maze_.Neighbors(cur);
        Tile best = cur;
        float bestScore = -1.0e9f;
        bool hasSaferStep = HasSaferImmediateFleeStep(cur, monsterTile);
        for (Tile n : neighbors) {
            bool riskyStep = FleeStepRiskyTowardMonster(cur, n, monsterTile);
            if (hasSaferStep && riskyStep) continue;
            XMFLOAT3 nw = maze_.WorldCenter(n, camera_.y);
            float stepX = nw.x - camera_.x;
            float stepZ = nw.z - camera_.z;
            float monX = monster_.x - camera_.x;
            float monZ = monster_.z - camera_.z;
            float stepLen = std::sqrt(stepX * stepX + stepZ * stepZ);
            float monLen = std::sqrt(monX * monX + monZ * monZ);
            float toward = (stepLen > 0.001f && monLen > 0.001f) ? (stepX * monX + stepZ * monZ) / (stepLen * monLen) : 0.0f;
            int open = maze_.OpenNeighborCount(n);
            float score = TileDistanceSq(n, monsterTile) * 12.0f - toward * 28.0f + static_cast<float>(maze_.LocalOpenCount(n, 2)) * 6.0f;
            float startDist = TileDistanceSq(cur, monsterTile);
            float nextDist = TileDistanceSq(n, monsterTile);
            score += nextDist > startDist + 0.01f ? 360.0f : -260.0f;
            if (toward > -0.10f) score -= 260.0f + toward * 180.0f;
            if (riskyStep) score -= 430.0f;
            score += (VisitCount(n) == 0 ? 70.0f : -static_cast<float>(std::min<int>(VisitCount(n), 5)) * 24.0f);
            score += (TileDistanceSq(cur, maze_.exit) - TileDistanceSq(n, maze_.exit)) * 2.0f;
            if (IsBacktrackingStep(cur, n)) score += nextDist > startDist + 0.01f ? 120.0f : -35.0f;
            if (!maze_.LineClear(n, monsterTile)) score += 160.0f;
            if (open >= 3) score += 85.0f;
            if (open <= 1) score -= 150.0f;
            if (score > bestScore) {
                bestScore = score;
                best = n;
            }
        }
        if (best == cur) return false;
        path_.clear();
        path_.push_back(cur);
        path_.push_back(best);
        pathIndex_ = 1;
        return true;
    }

    std::vector<Tile> BuildThreatEscapePath(Tile cur, Tile monsterTile) {
        if (!maze_.IsOpen(cur.x, cur.y)) return {};
        int count = maze_.w * maze_.h;
        auto idx = [this](Tile t) { return t.y * maze_.w + t.x; };
        int startIndex = idx(cur);
        std::vector<int> parent(static_cast<size_t>(count), -1);
        std::vector<int> depth(static_cast<size_t>(count), -1);
        std::queue<Tile> q;
        parent[static_cast<size_t>(startIndex)] = startIndex;
        depth[static_cast<size_t>(startIndex)] = 0;
        q.push(cur);
        while (!q.empty()) {
            Tile t = q.front();
            q.pop();
            for (Tile n : maze_.Neighbors(t)) {
                int ni = idx(n);
                if (depth[static_cast<size_t>(ni)] >= 0) continue;
                parent[static_cast<size_t>(ni)] = idx(t);
                depth[static_cast<size_t>(ni)] = depth[static_cast<size_t>(idx(t))] + 1;
                q.push(n);
            }
        }

        auto pathTo = [&](Tile target) {
            std::vector<Tile> out;
            int at = idx(target);
            if (at < 0 || at >= count || depth[static_cast<size_t>(at)] < 0) return out;
            while (at != -1) {
                out.push_back({at % maze_.w, at / maze_.w});
                if (at == startIndex) break;
                at = parent[static_cast<size_t>(at)];
            }
            std::reverse(out.begin(), out.end());
            return out;
        };

        std::vector<Tile> bestPath;
        float bestScore = -1.0e9f;
        std::vector<Tile> fallbackPath;
        float fallbackScore = -1.0e9f;
        for (int y = 1; y < maze_.h - 1; ++y) {
            for (int x = 1; x < maze_.w - 1; ++x) {
                Tile t{x, y};
                int ti = idx(t);
                int d = depth[static_cast<size_t>(ti)];
                if (d <= 0) continue;
                if (TileDistanceSq(t, monsterTile) < 2.0f) continue;
                std::vector<Tile> p = pathTo(t);
                if (p.size() < 2) continue;
                float score = FleePathScore(p, monsterTile);
                int lineBreak = FirstThreatLineBreakIndex(p, monsterTile, 12);
                int branch = FirstBranchIndex(p, 12);
                if (t == maze_.exit && exitSpotted_ && ExitRouteNotBlockedByMonster()) score += 1800.0f;
                if (lineBreak >= 0 && lineBreak <= 5) score += 260.0f - lineBreak * 34.0f;
                if (branch >= 0 && branch <= 6) score += 170.0f - branch * 24.0f;
                if (d < 3 && maze_.OpenNeighborCount(t) <= 2) score -= 110.0f;
                score += RandRange(-4.0f, 4.0f);
                if (ThreatPathMovesTowardMonster(p, monsterTile)) {
                    score -= 950.0f;
                    if (score > fallbackScore) {
                        fallbackScore = score;
                        fallbackPath = std::move(p);
                    }
                    continue;
                }
                if (score > bestScore) {
                    bestScore = score;
                    bestPath = std::move(p);
                }
            }
        }
        return !bestPath.empty() ? bestPath : fallbackPath;
    }

    void BeginExitTransition() {
        if (exitTransitionActive_) return;
        exitTransitionActive_ = true;
        exitTransitionTimer_ = 0.0f;
        exitDoorAngle_ = 0.0f;
        exitStartCamera_ = camera_;
        exitStartYaw_ = yaw_;
        stopTimer_ = 0.0f;
        headScanTimer_ = 0.0f;
        path_.clear();
        pathIndex_ = 0;
        sparks_.clear();
    }

    void UpdateExitTransition(float dt) {
        exitTransitionTimer_ += dt;
        float doorOpenStart = std::min(0.22f, settings_.exitDoorOpenSeconds * 0.35f);
        float doorOpenEnd = std::max(doorOpenStart + 0.05f, settings_.exitDoorOpenSeconds);
        float stepStart = std::max(0.05f, settings_.exitDoorOpenSeconds * 0.68f);
        float stepEnd = stepStart + settings_.exitStepSeconds;
        float fadeEnd = stepEnd + settings_.exitFadeSeconds;
        float doorOpen = SmoothStep(doorOpenStart, doorOpenEnd, exitTransitionTimer_);
        exitDoorAngle_ = doorOpen * 1.38f;

        XMFLOAT3 outward = Scale3(exitDoorNormal_, -1.0f);
        float step = SmoothStep(stepStart, stepEnd, exitTransitionTimer_);
        float settle = std::sin(Clamp01(step) * kPi);
        XMFLOAT3 previousCamera = camera_;
        camera_ = Add3(exitStartCamera_, Scale3(outward, step * settings_.exitStepDistance));
        float moved = std::sqrt((camera_.x - previousCamera.x) * (camera_.x - previousCamera.x) +
            (camera_.z - previousCamera.z) * (camera_.z - previousCamera.z));
        AdvanceStepPhase(moved, dt > 0.0001f ? moved / dt : settings_.walkSpeed);
        camera_.y = 1.43f + std::abs(std::sin(stepPhase_)) * settings_.headBobAmount * (1.0f - step) + settle * 0.035f;

        XMFLOAT3 focus = Add3(exitDoorCenter_, Scale3(outward, 0.65f + step * 1.1f));
        float targetYaw = YawToPoint(focus);
        yaw_ += AngleWrap(targetYaw - yaw_) * std::min(1.0f, dt * 3.6f);
        bodyYaw_ = yaw_;
        float targetPitch = std::clamp(PitchToPoint(Add3(focus, {0.0f, 0.20f, 0.0f})), -0.22f, 0.12f);
        lookPitch_ += (targetPitch - lookPitch_) * std::min(1.0f, dt * 2.8f);

        if (exitTransitionTimer_ > fadeEnd + 0.35f) {
            RestartMaze();
        }
    }

    void BeginDeath() {
        if (deathActive_) return;
        deathActive_ = true;
        deathTimer_ = 0.0f;
        dangerLevel_ = 1.0f;
        dreadLevel_ = 1.0f;
        stopTimer_ = 0.0f;
        headScanTimer_ = 0.0f;
        chaseLookBackTimer_ = 0.0f;
        chaseLookBackCooldown_ = 0.0f;
        chaseLookBackYaw_ = yaw_;
        chaseLookBackPitch_ = lookPitch_;
        stumbleTimer_ = 0.0f;
        path_.clear();
        pathIndex_ = 0;
    }

    void UpdateDeath(float dt) {
        deathTimer_ += dt;
        XMFLOAT3 focus = MonsterEyeFocus();
        float targetYaw = YawToPoint(focus);
        float targetPitch = std::clamp(PitchToPoint(focus), -0.35f, 0.92f);
        float focusSpeed = deathTimer_ < 0.9f ? 9.5f : 5.5f;
        yaw_ += AngleWrap(targetYaw - yaw_) * std::min(1.0f, dt * focusSpeed);
        bodyYaw_ = yaw_;
        lookPitch_ += (targetPitch - lookPitch_) * std::min(1.0f, dt * focusSpeed);
        camera_.y = 1.42f + std::sin(time_ * 19.0f) * 0.012f * SmoothStep(0.0f, 1.2f, deathTimer_);
        if (deathTimer_ > 4.25f) {
            RestartMaze();
        }
    }

    void ChoosePath(bool force = false) {
        Tile cur = CameraTile();
        if (!maze_.IsOpen(cur.x, cur.y)) {
            RecoverCameraToOpenTile();
            return;
        }
        Tile monsterTile = MonsterTile();
        if (!force && pathIndex_ < path_.size()) {
            bool freeRunPath = ChasePanicActive() && OpenAreaAllowsFreeRun(cur);
            bool panicContext = IsThreatVisible() || ChasePanicActive();
            if (ActivePathValidForMode(cur, freeRunPath) &&
                (!panicContext || !ActiveThreatPathShouldRepath(cur, monsterTile))) {
                return;
            }
            path_.clear();
            pathIndex_ = 0;
        }
        if (VisibleInFront(maze_.exit)) exitSpotted_ = true;
        path_.clear();
        pathIndex_ = 0;

        float mdx = monster_.x - camera_.x;
        float mdz = monster_.z - camera_.z;
        float monsterDist = std::sqrt(mdx * mdx + mdz * mdz);
        bool threat = (monsterDist < 12.0f && maze_.LineClear(cur, monsterTile)) || ChasePanicActive();
        std::vector<Tile> localNeighbors = maze_.Neighbors(cur);
        bool hasNonBacktrackingNeighbor = false;
        for (Tile n : localNeighbors) {
            if (!IsBacktrackingStep(cur, n)) {
                hasNonBacktrackingNeighbor = true;
                break;
            }
        }
        auto startsByBacktracking = [&](const std::vector<Tile>& p) {
            return hasNonBacktrackingNeighbor && p.size() > 1 && IsBacktrackingStep(cur, p[1]);
        };

        if (threat) {
            if (exitSpotted_ && ExitRouteNotBlockedByMonster()) {
                auto exitPath = maze_.Path(cur, maze_.exit);
                if (!exitPath.empty() && !ThreatPathMovesTowardMonster(exitPath, monsterTile)) {
                    path_ = std::move(exitPath);
                    pathIndex_ = path_.size() > 1 ? 1 : 0;
                    return;
                }
            }

            path_ = BuildThreatEscapePath(cur, monsterTile);
            if (path_.empty()) {
                ForceImmediateFleeStep(cur, monsterTile);
                return;
            }
        } else if (VisibleInFront(maze_.exit)) {
            exitSpotted_ = true;
            path_ = maze_.Path(cur, maze_.exit);
        } else if (BuildCorridorContinuationPath(cur)) {
            return;
        } else {
            float bestScore = -1.0f;
            for (int n = 0; n < 80; ++n) {
                int x = 1 + static_cast<int>(rng_() % (maze_.w - 2));
                int y = 1 + static_cast<int>(rng_() % (maze_.h - 2));
                if (!maze_.IsOpen(x, y)) continue;
                Tile t{x, y};
                float dx = static_cast<float>(t.x - cur.x);
                float dy = static_cast<float>(t.y - cur.y);
                float forwardBias = 0.0f;
                XMFLOAT3 tw = maze_.WorldCenter(t, camera_.y);
                XMFLOAT3 f = NavigationForward(cur);
                float wx = tw.x - camera_.x;
                float wz = tw.z - camera_.z;
                float wl = std::sqrt(wx * wx + wz * wz);
                if (wl > 0.01f) forwardBias = (wx * f.x + wz * f.z) / wl;
                float score = dx * dx + dy * dy + forwardBias * 18.0f;
                auto p = maze_.Path(cur, t);
                if (!p.empty()) {
                    if (startsByBacktracking(p)) continue;
                    score += PathExplorationScore(p, false);
                    if (VisitCount(t) == 0) score += 110.0f;
                    else score -= static_cast<float>(std::min<int>(VisitCount(t), 8)) * 42.0f;
                    if (p.size() > 1) {
                        if (VisitCount(p[1]) == 0) score += 92.0f;
                        else score -= static_cast<float>(std::min<int>(VisitCount(p[1]), 6)) * 34.0f;
                        if (IsBacktrackingStep(cur, p[1])) score -= hasNonBacktrackingNeighbor ? 520.0f : 120.0f;
                    }
                    if (score > bestScore) {
                        path_ = std::move(p);
                        bestScore = score;
                    }
                }
            }
        }
        if (path_.empty()) {
            const std::vector<Tile>& neighbors = localNeighbors;
            if (!neighbors.empty()) {
                Tile best = neighbors.front();
                float bestScore = -1.0e9f;
                for (Tile n : neighbors) {
                    if (hasNonBacktrackingNeighbor && IsBacktrackingStep(cur, n)) continue;
                    float score = (VisitCount(n) == 0 ? 120.0f : -static_cast<float>(std::min<int>(VisitCount(n), 8)) * 35.0f);
                    score += static_cast<float>(maze_.LocalOpenCount(n, 1)) * 3.0f;
                    score += (TileDistanceSq(cur, maze_.exit) - TileDistanceSq(n, maze_.exit)) * 1.1f;
                    if (IsBacktrackingStep(cur, n) && hasNonBacktrackingNeighbor) score -= 420.0f;
                    score += RandRange(-1.0f, 1.0f);
                    if (score > bestScore) {
                        bestScore = score;
                        best = n;
                    }
                }
                path_.push_back(cur);
                path_.push_back(best);
            }
        }
        pathIndex_ = path_.size() > 1 ? 1 : 0;
    }

    void BeginChaseLookBack(float urgency) {
        XMFLOAT3 focus = MonsterEyeFocus();
        chaseLookBackYaw_ = YawToPoint(focus);
        chaseLookBackPitch_ = std::clamp(PitchToPoint(focus), -0.34f, 0.24f);
        chaseLookBackDuration_ = RandRange(0.56f, 0.88f) * (1.0f - urgency * 0.10f);
        chaseLookBackTimer_ = chaseLookBackDuration_;
        chaseLookBackCooldown_ = std::max(0.85f, RandRange(1.35f, 3.15f) * (1.0f - urgency * 0.24f));
        secondsSinceLookBack_ = 0.0f;
        stumbleTimer_ = std::min(stumbleTimer_, 0.08f);
    }

    float ChaseLookBackWeight() const {
        if (chaseLookBackTimer_ <= 0.0f || chaseLookBackDuration_ <= 0.001f) return 0.0f;
        float t = 1.0f - chaseLookBackTimer_ / chaseLookBackDuration_;
        return SmoothStep(0.0f, 0.22f, t) * (1.0f - SmoothStep(0.70f, 1.0f, t));
    }

    void UpdateChaseLookBackTarget(float dt) {
        if (chaseLookBackTimer_ <= 0.0f || chaseLookBackDuration_ <= 0.001f) return;
        if (!MonsterLineOfSightToPlayer()) return;
        XMFLOAT3 focus = MonsterEyeFocus();
        float liveYaw = YawToPoint(focus);
        float livePitch = std::clamp(PitchToPoint(focus), -0.34f, 0.24f);
        float follow = std::min(1.0f, dt * 2.35f);
        chaseLookBackYaw_ += AngleWrap(liveYaw - chaseLookBackYaw_) * follow;
        chaseLookBackPitch_ += (livePitch - chaseLookBackPitch_) * follow;
    }

    void UpdateManualPlayer(float dt) {
        dt = std::clamp(dt, 0.0f, 0.05f);
        if (!CameraFootprintOpen(camera_.x, camera_.z)) {
            RecoverCameraToOpenTile();
        }

        constexpr float kMouseYawScale = 0.0022f;
        constexpr float kMousePitchScale = 0.0018f;
        float mouseScale = std::clamp(settings_.mouseSensitivity, 0.1f, 5.0f);
        float pitchSign = settings_.invertMouseY ? 1.0f : -1.0f;
        yaw_ += gameInput_.lookDeltaX * kMouseYawScale * mouseScale;
        bodyYaw_ = yaw_;
        lookPitch_ = std::clamp(lookPitch_ + gameInput_.lookDeltaY * kMousePitchScale * mouseScale * pitchSign, -1.55334f, 1.55334f);

        float inputX = std::clamp(gameInput_.moveX, -1.0f, 1.0f);
        float inputZ = std::clamp(gameInput_.moveZ, -1.0f, 1.0f);
        float inputLen = std::sqrt(inputX * inputX + inputZ * inputZ);
        if (inputLen > 1.0f) {
            inputX /= inputLen;
            inputZ /= inputLen;
            inputLen = 1.0f;
        }

        bool wantsMove = inputLen > 0.001f;
        bool crouching = gameInput_.crouch;
        bool wantsSprint = gameInput_.sprint && wantsMove && !crouching && playerStamina_ > 10.0f;
        float walkSpeed = settings_.walkSpeed;
        float sprintSpeed = std::max(settings_.runSpeed, walkSpeed * 1.35f);
        float targetSpeed = wantsSprint ? sprintSpeed : walkSpeed;
        if (crouching) targetSpeed *= 0.52f;

        if (wantsSprint) {
            playerStamina_ = std::max(0.0f, playerStamina_ - 24.0f * dt);
            playerStaminaRegenDelay_ = 0.75f;
            if (playerStamina_ <= 0.0f) {
                wantsSprint = false;
                targetSpeed = walkSpeed;
            }
        } else {
            playerStaminaRegenDelay_ = std::max(0.0f, playerStaminaRegenDelay_ - dt);
            if (playerStaminaRegenDelay_ <= 0.0f) {
                playerStamina_ = std::min(100.0f, playerStamina_ + 18.0f * dt);
            }
        }

        smoothedMoveSpeed_ = MoveTowards(smoothedMoveSpeed_, wantsMove ? targetSpeed : 0.0f,
            (wantsMove ? 8.0f : 10.0f) * dt);
        float moveDistance = smoothedMoveSpeed_ * dt;
        if (wantsMove && moveDistance > 0.0001f) {
            XMFLOAT3 forward = Forward();
            XMFLOAT3 right{std::cos(yaw_), 0.0f, -std::sin(yaw_)};
            XMFLOAT3 moveDir = Normalize3(Add3(Scale3(right, inputX), Scale3(forward, inputZ)), forward);
            MoveCameraSafely(moveDir.x * moveDistance, moveDir.z * moveDistance,
                moveDistance, std::max(0.1f, smoothedMoveSpeed_), CameraTile(), true);
        }

        if (gameInput_.jump && playerGrounded_ && !crouching) {
            playerVerticalVelocity_ = 4.2f;
            playerGrounded_ = false;
        }
        if (!playerGrounded_) {
            playerVerticalVelocity_ -= 9.81f * dt;
            playerVerticalOffset_ += playerVerticalVelocity_ * dt;
            if (playerVerticalOffset_ <= 0.0f) {
                playerVerticalOffset_ = 0.0f;
                playerVerticalVelocity_ = 0.0f;
                playerGrounded_ = true;
            }
        }

        if (gameInput_.interact && !exitTransitionActive_) {
            Tile cur = CameraTile();
            float exitDistSq = TileDistanceSq(cur, maze_.exit);
            if (exitDistSq <= 2.0f && VisibleInFront(maze_.exit)) {
                BeginExitTransition();
            }
        }

        float moveBlend = Clamp01(smoothedMoveSpeed_ / std::max(0.1f, sprintSpeed));
        runIntensity_ += ((wantsSprint ? moveBlend : moveBlend * 0.45f) - runIntensity_) *
            std::min(1.0f, dt * 5.0f);
        runEffort_ += ((wantsSprint ? moveBlend : 0.0f) - runEffort_) *
            std::min(1.0f, dt * 3.0f);
        if (wantsMove) {
            AdvanceStepPhase(moveDistance, std::max(0.1f, smoothedMoveSpeed_));
        }
        breathPhase_ += dt * (1.15f + runEffort_ * 4.6f + runIntensity_ * 1.25f) * kPi;
        if (breathPhase_ > kPi * 128.0f) {
            breathPhase_ = std::fmod(breathPhase_, kPi * 2.0f);
        }

        float eyeTarget = crouching ? 1.12f : 1.45f;
        float bobAmount = settings_.headBobAmount * Lerp(0.24f, 1.85f, moveBlend) * (crouching ? 0.30f : 1.0f);
        float sideBob = crouching ? 0.0f : std::sin(stepPhase_ * 0.5f) * (0.008f + runEffort_ * 0.018f);
        float breathY = std::sin(breathPhase_) * (0.0035f + runIntensity_ * 0.015f + runEffort_ * 0.034f);
        float desiredY = eyeTarget + playerVerticalOffset_ + std::abs(std::sin(stepPhase_)) * bobAmount + sideBob + breathY;
        camera_.y += (desiredY - camera_.y) * std::min(1.0f, dt * 14.0f);
    }

    void UpdatePathFollower(float dt) {
        bool threat = IsThreatVisible();
        bool panicActive = threat || ChasePanicActive();
        bool sightFreezeActive = threat && MonsterSightingFreezeActive();
        if (panicActive) {
            bloodFocusTimer_ = 0.0f;
            ventReactionTimer_ = 0.0f;
            branchLookTimer_ = 0.0f;
            roomSurveyTimer_ = 0.0f;
            branchLookPaused_ = false;
        } else {
            bloodFocusTimer_ = std::max(0.0f, bloodFocusTimer_ - dt);
            ventReactionTimer_ = std::max(0.0f, ventReactionTimer_ - dt);
            branchLookTimer_ = std::max(0.0f, branchLookTimer_ - dt);
            roomSurveyTimer_ = std::max(0.0f, roomSurveyTimer_ - dt);
            branchLookCooldown_ = std::max(0.0f, branchLookCooldown_ - dt);
            roomSurveyCooldown_ = std::max(0.0f, roomSurveyCooldown_ - dt);
            if (branchLookTimer_ <= 0.0f) branchLookPaused_ = false;
        }
        bool bloodFocusActive = bloodFocusTimer_ > 0.0f;
        bool ventReactionActive = !panicActive && ventReactionTimer_ > 0.0f;
        bool softStopActive = !panicActive && (stopTimer_ > 0.0f || bloodFocusActive || ventReactionActive);
        float ventReactionElapsed = 0.0f;
        float ventLookWeight = 0.0f;
        float ventBackAwayWeight = 0.0f;
        if (ventReactionActive) {
            ventReactionElapsed = std::max(0.0f, ventReactionDuration_ - ventReactionTimer_);
            ventLookWeight = SmoothStep(0.0f, 0.16f, ventReactionElapsed - ventReactionLookDelay_);
            float backT = (ventReactionElapsed - ventReactionLookDelay_) / std::max(0.12f, ventReactionBackDuration_);
            ventBackAwayWeight = SmoothStep(0.0f, 0.18f, backT) * (1.0f - SmoothStep(0.76f, 1.0f, backT));
        }
        bool completedJunctionScan = false;
        bool roomSurveySpaceNow = IsRoomSurveySpot(CameraTile());
        if (stopTimer_ <= 0.0f && !bloodFocusActive) {
            secondsSinceLookBack_ += dt;
        }
        if (sightFreezeActive) {
            stopTimer_ = 0.0f;
            headScanTimer_ = 0.0f;
            headScanDuration_ = 0.0f;
            lookBack_ = false;
            junctionScanActive_ = false;
            propLookTimer_ = 0.0f;
            branchLookTimer_ = 0.0f;
            roomSurveyTimer_ = 0.0f;
            branchLookPaused_ = false;
            bloodFocusTimer_ = 0.0f;
            ventReactionTimer_ = 0.0f;
            chaseLookBackTimer_ = 0.0f;
            path_.clear();
            pathIndex_ = 0;
            threatRepath_ = 0.0f;
            smoothedMoveSpeed_ = 0.0f;
            runIntensity_ += (0.0f - runIntensity_) * std::min(1.0f, dt * 8.0f);
            runEffort_ += (0.36f - runEffort_) * std::min(1.0f, dt * 5.0f);

            XMFLOAT3 focus = MonsterEyeFocus();
            float targetYaw = YawToPoint(focus);
            float targetPitch = std::clamp(PitchToPoint(focus), -0.36f, 0.42f);
            yaw_ += AngleWrap(targetYaw - yaw_) * std::min(1.0f, dt * 10.5f);
            bodyYaw_ = yaw_;
            lookPitch_ += (targetPitch - lookPitch_) * std::min(1.0f, dt * 8.5f);
            turnLookBlend_ = 0.0f;
            turnLookYaw_ = yaw_;
            float startledY = 1.45f + std::sin(time_ * 11.0f) * 0.010f;
            camera_.y += (startledY - camera_.y) * std::min(1.0f, dt * 7.0f);
            return;
        }
        if (panicActive) {
            stopTimer_ = 0.0f;
            headScanTimer_ = 0.0f;
            headScanDuration_ = 0.0f;
            lookBack_ = false;
            junctionScanActive_ = false;
            propLookTimer_ = 0.0f;
            branchLookTimer_ = 0.0f;
            roomSurveyTimer_ = 0.0f;
            branchLookPaused_ = false;

            chaseLookBackCooldown_ = std::max(0.0f, chaseLookBackCooldown_ - dt);
            chaseLookBackTimer_ = std::max(0.0f, chaseLookBackTimer_ - dt);
            if (threat && chaseLookBackTimer_ <= 0.0f && chaseLookBackCooldown_ <= 0.0f && MonsterDistance() > 1.55f) {
                float urgency = Clamp01((7.8f - MonsterDistance()) / 6.4f);
                float elapsedPressure = Clamp01(secondsSinceLookBack_ / 9.0f);
                float chance = dt * (0.22f + urgency * 1.55f + elapsedPressure * 0.80f);
                if (RandRange(0.0f, 1.0f) < chance) {
                    BeginChaseLookBack(urgency);
                }
            }
            if (threat) UpdateChaseLookBackTarget(dt);

            stumbleTimer_ = std::max(0.0f, stumbleTimer_ - dt);
            if (threat && stumbleTimer_ <= 0.0f && chaseLookBackTimer_ <= 0.0f && RandRange(0.0f, 1.0f) < dt * 0.14f) {
                stumbleDuration_ = RandRange(0.18f, 0.34f);
                stumbleTimer_ = stumbleDuration_;
                stumbleYawOffset_ = RandRange(-0.20f, 0.20f);
            }
        } else if (roomSurveySpaceNow && !bloodFocusActive && !ventReactionActive && stopTimer_ <= 0.0f &&
            propLookTimer_ <= 0.0f && branchLookTimer_ <= 0.0f && roomSurveyTimer_ <= 0.0f &&
            time_ > nextLookBackTime_) {
            float elapsedPressure = Clamp01(secondsSinceLookBack_ / std::max(2.0f, settings_.lookBackMaxSeconds));
            float proximityPressure = Clamp01((10.0f - MonsterDistance()) / 8.0f) * 0.35f;
            float chance = dt * (0.08f + elapsedPressure * 0.62f + proximityPressure);
            if (RandRange(0.0f, 1.0f) < chance) {
                stopTimer_ = RandRange(1.05f, 1.85f);
                headScanDuration_ = stopTimer_;
                headScanTimer_ = stopTimer_;
                headScanCenter_ = bodyYaw_ + RandRange(-0.16f, 0.16f);
                lookBack_ = true;
                branchLookTimer_ = 0.0f;
                roomSurveyTimer_ = 0.0f;
                nextLookBackTime_ = time_ + RandRange(settings_.lookBackMinSeconds, settings_.lookBackMaxSeconds);
                secondsSinceLookBack_ = 0.0f;
            }
        }

        if (!panicActive && !ventReactionActive && headScanTimer_ > 0.0f) {
            headScanTimer_ = std::max(0.0f, headScanTimer_ - dt);
            float t = headScanDuration_ > 0.001f ? 1.0f - headScanTimer_ / headScanDuration_ : 1.0f;
            float scanAngle = settings_.scanAngleDegrees * kPi / 180.0f;
            float desired = headScanCenter_;
            float scanSpeed = 4.6f;
            Tile scanTile = CameraTile();
            bool corridorScanBlocked = !IsRoomSurveySpot(scanTile) && !junctionScanActive_;
            if (corridorScanBlocked) {
                headScanTimer_ = 0.0f;
                headScanDuration_ = 0.0f;
                lookBack_ = false;
                stopTimer_ = 0.0f;
            }
            if (!corridorScanBlocked && junctionScanActive_ && junctionScanCount_ > 0) {
                float segment = Clamp01(t) * static_cast<float>(junctionScanCount_);
                int scanIndex = std::min(junctionScanCount_ - 1, static_cast<int>(segment));
                float localT = segment - static_cast<float>(scanIndex);
                float settle = SmoothStep(0.10f, 0.42f, localT) * (1.0f - SmoothStep(0.82f, 1.0f, localT));
                float branchYaw = junctionScanYaws_[static_cast<size_t>(scanIndex)];
                desired = branchYaw + std::sin(time_ * 5.3f + scanIndex) * 0.035f * settle;
                scanSpeed = 7.8f;
            } else if (!corridorScanBlocked) {
                float sweep = std::sin(t * kPi * 2.0f);
                float rawDesired = lookBack_
                    ? headScanCenter_ + kPi + sweep * 0.22f
                    : headScanCenter_ + sweep * scanAngle;
                if (lookBack_) {
                    desired = rawDesired;
                    scanSpeed = 7.4f;
                } else {
                    desired = rawDesired;
                }
            }
            if (!corridorScanBlocked) {
                yaw_ += AngleWrap(desired - yaw_) * std::min(1.0f, dt * scanSpeed);
                lookPitch_ += (-0.035f - lookPitch_) * std::min(1.0f, dt * 3.0f);
            }
        }

        if (stopTimer_ > 0.0f) {
            stopTimer_ = std::max(0.0f, stopTimer_ - dt);
            float idleY = 1.47f + std::sin(time_ * 2.1f) * 0.008f;
            camera_.y += (idleY - camera_.y) * std::min(1.0f, dt * 3.0f);
            if (stopTimer_ <= 0.0f) {
                completedJunctionScan = junctionScanActive_;
                lookBack_ = false;
                junctionScanActive_ = false;
            }
        }

        if (ventReactionActive) {
            if (ventBackAwayWeight > 0.001f) {
                float backSpeed = std::max(settings_.walkSpeed * 1.10f, maze_.TileMinimum() * 0.42f);
                float nudge = backSpeed * ventBackAwayWeight * dt;
                Tile cur = CameraTile();
                MoveCameraSafely(ventReactionAway_.x * nudge, ventReactionAway_.z * nudge,
                    nudge, backSpeed, cur);
            }
        }

        if (!CameraFootprintOpen(camera_.x, camera_.z)) {
            RecoverCameraToOpenTile();
            return;
        }
        if (!panicActive && stopTimer_ > 0.0f && (lookBack_ || junctionScanActive_)) {
            return;
        }
        ChoosePath(completedJunctionScan);
        if (pathIndex_ >= path_.size()) return;
        bool freeRunPath = panicActive && OpenAreaAllowsFreeRun(CameraTile());
        if (!ActivePathValidForMode(CameraTile(), freeRunPath)) {
            ChoosePath(true);
            freeRunPath = panicActive && OpenAreaAllowsFreeRun(CameraTile());
            if (pathIndex_ >= path_.size() || !ActivePathValidForMode(CameraTile(), freeRunPath)) return;
        }
        XMFLOAT3 target = maze_.WorldCenter(path_[pathIndex_], camera_.y);
        float dx = target.x - camera_.x;
        float dz = target.z - camera_.z;
        float dist = std::sqrt(dx * dx + dz * dz);
        if (panicActive && dist > 0.001f) {
            float monX = monster_.x - camera_.x;
            float monZ = monster_.z - camera_.z;
            float monLen = std::sqrt(monX * monX + monZ * monZ);
            float toward = monLen > 0.001f ? (dx * monX + dz * monZ) / (dist * monLen) : -1.0f;
            Tile monsterTile = MonsterTile();
            Tile cur = CameraTile();
            bool visibleTurnToward = toward > -0.10f &&
                (maze_.LineClear(cur, monsterTile) || maze_.LineClear(path_[pathIndex_], monsterTile));
            bool saferStepAvailable = HasSaferImmediateFleeStep(cur, monsterTile);
            bool riskyTarget = saferStepAvailable && FleeStepRiskyTowardMonster(cur, path_[pathIndex_], monsterTile);
            if (visibleTurnToward || riskyTarget) {
                auto escape = BuildThreatEscapePath(cur, monsterTile);
                if (!escape.empty()) {
                    path_ = std::move(escape);
                    pathIndex_ = path_.size() > 1 ? 1 : 0;
                } else {
                    ForceImmediateFleeStep(cur, monsterTile);
                }
                if (pathIndex_ >= path_.size()) return;
                target = maze_.WorldCenter(path_[pathIndex_], camera_.y);
                dx = target.x - camera_.x;
                dz = target.z - camera_.z;
                dist = std::sqrt(dx * dx + dz * dz);
            }
        }

        Tile movementTile = CameraTile();
        size_t moveIndex = pathIndex_;
        Tile targetTileForMove = path_[moveIndex];
        bool freeRunMove = panicActive && OpenAreaAllowsFreeRun(movementTile);
        if (freeRunMove) {
            size_t directLimit = std::min(path_.size() - 1, pathIndex_ + 5);
            for (size_t i = pathIndex_ + 1; i <= directLimit; ++i) {
                if (!OpenAreaAllowsFreeRun(path_[i])) break;
                XMFLOAT3 directTarget = maze_.WorldCenter(path_[i], camera_.y);
                if (!CameraSegmentOpenThroughOpen(camera_.x, camera_.z, directTarget.x, directTarget.z, true)) break;
                moveIndex = i;
            }
            if (moveIndex > pathIndex_) {
                targetTileForMove = path_[moveIndex];
                target = maze_.WorldCenter(targetTileForMove, camera_.y);
                dx = target.x - camera_.x;
                dz = target.z - camera_.z;
                dist = std::sqrt(dx * dx + dz * dz);
            }
        }
        if (freeRunMove && panicActive && moveIndex > pathIndex_) {
            Tile monsterTile = MonsterTile();
            bool riskySkip = HasSaferImmediateFleeStep(movementTile, monsterTile) &&
                FleeStepRiskyTowardMonster(movementTile, targetTileForMove, monsterTile);
            if (riskySkip) {
                moveIndex = pathIndex_;
                targetTileForMove = path_[moveIndex];
                target = maze_.WorldCenter(targetTileForMove, camera_.y);
                dx = target.x - camera_.x;
                dz = target.z - camera_.z;
                dist = std::sqrt(dx * dx + dz * dz);
            }
        }
        float bodyTargetYaw = dist > 0.001f ? std::atan2(dx, dz) : bodyYaw_;
        if (!freeRunMove && AdjacentTiles(movementTile, targetTileForMove)) {
            XMFLOAT3 currentCenter = maze_.WorldCenter(movementTile, camera_.y);
            float alignTolerance = std::clamp(maze_.TileMinimum() * 0.055f, 0.045f, 0.11f);
            if (targetTileForMove.x != movementTile.x && std::abs(camera_.z - currentCenter.z) > alignTolerance) {
                target = {camera_.x, camera_.y, currentCenter.z};
                dx = target.x - camera_.x;
                dz = target.z - camera_.z;
                dist = std::sqrt(dx * dx + dz * dz);
            } else if (targetTileForMove.y != movementTile.y && std::abs(camera_.x - currentCenter.x) > alignTolerance) {
                target = {currentCenter.x, camera_.y, camera_.z};
                dx = target.x - camera_.x;
                dz = target.z - camera_.z;
                dist = std::sqrt(dx * dx + dz * dz);
            }
        }

        float pathAdvanceRadius = std::clamp(maze_.TileMinimum() * 0.045f, 0.045f, 0.085f);
        for (int advance = 0; !softStopActive && dist < pathAdvanceRadius && advance < 3; ++advance) {
            ++pathIndex_;
            if (CameraTile() == maze_.exit) {
                BeginExitTransition();
                return;
            }
            Tile cur = CameraTile();
            bool pauseStarted = false;
            if (!(cur == lastTile_)) {
                Tile previousTile = lastTile_;
                previousTile_ = previousTile;
                lastTile_ = cur;
                MarkVisited(cur);
                bool roomSurveySpot = IsRoomSurveySpot(cur);
                bool roomEntry = roomSurveySpot && !IsRoomSurveySpot(previousTile);
                Tile nextTarget = pathIndex_ < path_.size() ? path_[pathIndex_] : cur;
                bool nextIsAdjacent = AdjacentTiles(cur, nextTarget);
                int approachX = cur.x - previousTile.x;
                int approachY = cur.y - previousTile.y;
                int exitX = nextTarget.x - cur.x;
                int exitY = nextTarget.y - cur.y;
                bool continuingStraight = nextIsAdjacent && exitX == approachX && exitY == approachY;
                bool turningOrChoosing = nextIsAdjacent && !continuingStraight;
                bool hasSideBranch = PathSideBranchCount(cur, previousTile, nextTarget) > 0;
                if (!panicActive && turningOrChoosing && hasSideBranch && IsTightTJunction(cur, previousTile) &&
                    RandRange(0.0f, 1.0f) < settings_.junctionScanChance) {
                    pauseStarted = BeginJunctionScan(cur, previousTile);
                } else if (!panicActive && roomEntry && RandRange(0.0f, 1.0f) < settings_.roomPauseChance) {
                    ChoosePath(true);
                    BeginRoomSurvey(cur, true);
                    pauseStarted = roomSurveyTimer_ > 0.0f;
                } else if (!panicActive && roomEntry && RandRange(0.0f, 1.0f) < 0.72f) {
                    BeginRoomSurvey(cur, false);
                } else if (!panicActive && roomSurveySpot && !roomEntry && roomSurveyCooldown_ <= 0.0f &&
                    RandRange(0.0f, 1.0f) < 0.24f) {
                    BeginRoomSurvey(cur, RandRange(0.0f, 1.0f) < 0.38f);
                    pauseStarted = stopTimer_ > 0.0f && roomSurveyTimer_ > 0.0f;
                } else if (!panicActive && hasSideBranch) {
                    bool hallwaySidePeek = continuingStraight && !roomSurveySpot;
                    bool decisionPeek = turningOrChoosing && !roomSurveySpot;
                    bool roomEdgePeek = roomSurveySpot && RandRange(0.0f, 1.0f) < 0.42f;
                    if (hallwaySidePeek || decisionPeek || roomEdgePeek) {
                        float chance = decisionPeek ? 0.88f : (hallwaySidePeek ? 0.74f : 0.46f);
                        if (RandRange(0.0f, 1.0f) < chance) {
                            bool allowPause = hallwaySidePeek
                                ? RandRange(0.0f, 1.0f) < 0.34f
                                : RandRange(0.0f, 1.0f) < 0.18f;
                            bool startedPeek = BeginBranchLook(cur, previousTile, nextTarget, allowPause, roomSurveySpot);
                            pauseStarted = startedPeek && branchLookPaused_;
                        }
                    }
                }
            }
            softStopActive = !panicActive && (stopTimer_ > 0.0f || bloodFocusActive || ventReactionActive);
            if (pauseStarted || softStopActive) {
                if (pauseStarted && junctionScanActive_) {
                    return;
                }
                if (pathIndex_ >= path_.size()) ChoosePath();
                if (pathIndex_ >= path_.size() || !ActivePathValidForMode(CameraTile(), panicActive && OpenAreaAllowsFreeRun(CameraTile()))) return;
                target = maze_.WorldCenter(path_[pathIndex_], camera_.y);
                dx = target.x - camera_.x;
                dz = target.z - camera_.z;
                dist = std::sqrt(dx * dx + dz * dz);
                break;
            }
            if (pathIndex_ >= path_.size()) {
                ChoosePath(true);
                if (pathIndex_ >= path_.size() || !ActivePathValidForMode(CameraTile(), panicActive && OpenAreaAllowsFreeRun(CameraTile()))) return;
            }
            target = maze_.WorldCenter(path_[pathIndex_], camera_.y);
            dx = target.x - camera_.x;
            dz = target.z - camera_.z;
            dist = std::sqrt(dx * dx + dz * dz);
        }

        if (pathIndex_ < path_.size()) {
            Tile bodyTile = freeRunMove && moveIndex < path_.size() ? path_[moveIndex] : path_[pathIndex_];
            XMFLOAT3 bodyTarget = maze_.WorldCenter(bodyTile, camera_.y);
            float bdx = bodyTarget.x - camera_.x;
            float bdz = bodyTarget.z - camera_.z;
            if (bdx * bdx + bdz * bdz > 0.0001f) {
                bodyTargetYaw = std::atan2(bdx, bdz);
            }
        }
        bodyYaw_ += AngleWrap(bodyTargetYaw - bodyYaw_) * std::min(1.0f, dt * 5.2f);
        UpdatePropLook(dt, panicActive);

        Tile cameraTileForLook = CameraTile();
        float pathTurnWeight = 0.0f;
        float desiredYaw = bodyTargetYaw;
        float pathTurnTargetWeight = 0.0f;
        float pathTurnTargetYaw = desiredYaw;
        if (pathIndex_ < path_.size()) {
            bool roomLook = OpenAreaAllowsFreeRun(cameraTileForLook);
            float lookAheadTiles = roomLook
                ? std::clamp(settings_.roomLookAheadTiles, 0.0f, 10.0f)
                : std::clamp(settings_.turnLookAheadTiles, 0.0f, 8.0f);
            XMFLOAT3 lookTarget = PathLookAheadPoint(lookAheadTiles);
            float lx = lookTarget.x - camera_.x;
            float lz = lookTarget.z - camera_.z;
            if (lx * lx + lz * lz > 0.0001f) {
                desiredYaw = std::atan2(lx, lz);
            }

            size_t turnIndex = pathIndex_;
            if (path_[turnIndex] == cameraTileForLook && turnIndex + 1 < path_.size()) {
                ++turnIndex;
            }
            if (turnIndex + 1 < path_.size()) {
                Tile approach = AdjacentTiles(cameraTileForLook, path_[turnIndex])
                    ? cameraTileForLook
                    : (turnIndex > 0 ? path_[turnIndex - 1] : cameraTileForLook);
                Tile turn = path_[turnIndex];
                Tile after = path_[turnIndex + 1];
                if (AdjacentTiles(approach, turn) && AdjacentTiles(turn, after)) {
                    int inX = turn.x - approach.x;
                    int inY = turn.y - approach.y;
                    int outX = after.x - turn.x;
                    int outY = after.y - turn.y;
                    bool changedDirection = inX != outX || inY != outY;
                    bool reversed = outX == -inX && outY == -inY;
                    if (changedDirection && !reversed) {
                        XMFLOAT3 turnCenter = maze_.WorldCenter(turn, camera_.y);
                        XMFLOAT3 afterCenter = maze_.WorldCenter(after, camera_.y);
                        float distToTurn = std::sqrt((turnCenter.x - camera_.x) * (turnCenter.x - camera_.x) +
                            (turnCenter.z - camera_.z) * (turnCenter.z - camera_.z));
                        float tile = std::max(maze_.TileMinimum(), 0.1f);
                        float turnLeadTiles = std::clamp(settings_.turnLookAheadTiles, 0.0f, 8.0f);
                        float turnLeadStart = tile * std::clamp(0.86f + turnLeadTiles * 0.42f, 0.86f, 4.25f);
                        float turnLead = SmoothStep(turnLeadStart, tile * 0.18f, distToTurn);
                        turnLead *= turnLead;
                        pathTurnTargetWeight = turnLead * 0.68f;
                        float outgoingYaw = std::atan2(afterCenter.x - turnCenter.x, afterCenter.z - turnCenter.z);
                        pathTurnTargetYaw = outgoingYaw;
                    }
                }
            }
        }
        if (!panicActive && !softStopActive && pathTurnTargetWeight > 0.001f) {
            if (turnLookBlend_ < 0.012f) {
                turnLookYaw_ = pathTurnTargetYaw;
            } else {
                turnLookYaw_ += AngleWrap(pathTurnTargetYaw - turnLookYaw_) * std::min(1.0f, dt * 6.2f);
            }
            float response = pathTurnTargetWeight > turnLookBlend_
                ? (2.35f + pathTurnTargetWeight * 2.80f)
                : 5.8f;
            turnLookBlend_ += (pathTurnTargetWeight - turnLookBlend_) * std::min(1.0f, dt * response);
        } else {
            turnLookBlend_ += (0.0f - turnLookBlend_) * std::min(1.0f, dt * 5.2f);
            if (turnLookBlend_ < 0.001f) {
                turnLookBlend_ = 0.0f;
                turnLookYaw_ = desiredYaw;
            }
        }
        pathTurnWeight = turnLookBlend_;
        if (!panicActive && !softStopActive && pathTurnWeight > 0.001f) {
            desiredYaw += AngleWrap(turnLookYaw_ - desiredYaw) * pathTurnWeight;
        }
        XMFLOAT3 exitFocusTarget{};
        float exitLookTargetWeight = (!panicActive && !softStopActive) ? ExitAttentionWeight(exitFocusTarget) : 0.0f;
        if (exitLookTargetWeight > 0.001f) {
            exitLookFocus_ = exitLookBlend_ <= 0.001f
                ? exitFocusTarget
                : Lerp3(exitLookFocus_, exitFocusTarget, std::min(1.0f, dt * 5.4f));
        }
        float exitLookResponse = exitLookTargetWeight > exitLookBlend_
            ? (2.20f + exitLookTargetWeight * 2.75f)
            : 4.8f;
        exitLookBlend_ += (exitLookTargetWeight - exitLookBlend_) * std::min(1.0f, dt * exitLookResponse);
        if (exitLookBlend_ < 0.001f) exitLookBlend_ = 0.0f;

        float chaseLookBackWeight = threat ? ChaseLookBackWeight() : 0.0f;
        bool branchLookActive = !panicActive && branchLookTimer_ > 0.0f && branchLookDuration_ > 0.001f;
        float branchLookWeight = branchLookActive ? BranchLookWeight() : 0.0f;
        bool roomSurveyActive = !panicActive && roomSurveyTimer_ > 0.0f && roomSurveyDuration_ > 0.001f;
        float roomSurveyWeight = roomSurveyActive ? RoomSurveyWeight() : 0.0f;
        float stumbleAmount = 0.0f;
        if (stumbleTimer_ > 0.0f && stumbleDuration_ > 0.001f) {
            float t = 1.0f - stumbleTimer_ / stumbleDuration_;
            stumbleAmount = std::sin(Clamp01(t) * kPi);
        }
        if (panicActive) {
            desiredYaw = bodyYaw_;
            if (threat && chaseLookBackWeight > 0.0f) {
                desiredYaw += AngleWrap(chaseLookBackYaw_ - desiredYaw) * (chaseLookBackWeight * 0.96f);
            }
            desiredYaw += stumbleYawOffset_ * stumbleAmount * (1.0f - chaseLookBackWeight);
        } else if (softStopActive) {
            if (ventReactionActive) {
                if (ventLookWeight > 0.001f) {
                    float ventTargetYaw = YawToPoint(ventReactionTarget_);
                    float scanWeight = SmoothStep(0.10f, 1.0f, ventLookWeight);
                    float scanYaw = (std::sin(time_ * 7.9f + ventReactionScanSeed_) * 0.042f +
                        std::sin(time_ * 13.7f + ventReactionScanSeed_ * 1.7f) * 0.018f) * scanWeight;
                    desiredYaw = yaw_ + AngleWrap(ventTargetYaw - yaw_) * ventLookWeight + scanYaw;
                } else {
                    desiredYaw = yaw_ + std::sin(time_ * 18.0f + ventReactionScanSeed_) * 0.010f;
                }
            } else if (bloodFocusActive) desiredYaw = YawToPoint(bloodFocusTarget_);
            else if (branchLookActive) {
                float lock = std::max(0.58f, branchLookWeight);
                desiredYaw = yaw_ + AngleWrap(BranchLookTargetYaw() - yaw_) * lock;
            } else if (roomSurveyActive) desiredYaw = RoomSurveyYaw();
            else desiredYaw = yaw_;
        } else {
            Tile cameraTile = CameraTile();
            bool corridorLike = IsCorridorLike(cameraTile);
            if (branchLookActive &&
                ViewRayOpenDistance(branchLookYaw_, maze_.TileMinimum() * 2.2f) < maze_.TileMinimum() * 0.82f) {
                branchLookTimer_ = 0.0f;
                branchLookActive = false;
                branchLookWeight = 0.0f;
            }
            float idleYaw = std::sin(time_ * 0.73f) * 0.045f + std::sin(time_ * 1.17f) * 0.025f;
            float idleScale = 1.0f;
            if (branchLookActive) idleScale *= 0.35f;
            if (roomSurveyActive) idleScale *= 0.50f;
            if (corridorLike) idleScale *= 0.38f;
            if (pathTurnWeight > 0.001f) idleScale *= Lerp(1.0f, 0.18f, Clamp01(pathTurnWeight / 0.52f));
            desiredYaw += idleYaw * idleScale;
            if (roomSurveyActive) {
                desiredYaw += AngleWrap(RoomSurveyYaw() - desiredYaw) * (roomSurveyWeight * 0.92f);
            } else if (branchLookActive) {
                desiredYaw += AngleWrap(BranchLookTargetYaw() - desiredYaw) * std::min(1.0f, branchLookWeight * 1.08f);
            }
            if (exitLookBlend_ > 0.001f) {
                desiredYaw += AngleWrap(YawToPoint(exitLookFocus_) - desiredYaw) * exitLookBlend_;
            }
        }
        float turnSpeed = panicActive ? Lerp(4.2f, 6.2f, chaseLookBackWeight) : (bloodFocusActive ? 4.1f : 2.9f);
        if (ventReactionActive) turnSpeed = Lerp(2.6f, 7.8f, ventLookWeight);
        else if (branchLookActive) turnSpeed = branchLookPaused_ ? 5.25f : 4.65f;
        else if (roomSurveyActive) turnSpeed = softStopActive ? 3.65f : 2.85f;
        else if (!panicActive && pathTurnWeight > 0.001f) turnSpeed = Lerp(turnSpeed, 4.15f, Clamp01(pathTurnWeight / 0.68f));
        if (!panicActive && !softStopActive && exitLookBlend_ > 0.001f) {
            turnSpeed = std::max(turnSpeed, Lerp(3.45f, 4.85f, Clamp01(exitLookBlend_ / 0.82f)));
        }
        yaw_ += AngleWrap(desiredYaw - yaw_) * std::min(1.0f, dt * turnSpeed);
        float pitchTarget = panicActive ? -0.030f : -0.055f;
        if (ventReactionActive) {
            float ventTargetPitch = std::clamp(PitchToPoint(ventReactionTarget_), -0.58f, 0.24f);
            float scanWeight = SmoothStep(0.10f, 1.0f, ventLookWeight);
            float scanPitch = (std::sin(time_ * 8.6f + ventReactionScanSeed_ * 1.3f) * 0.024f +
                std::sin(time_ * 15.2f + ventReactionScanSeed_) * 0.010f) * scanWeight;
            pitchTarget = Lerp(lookPitch_, ventTargetPitch, ventLookWeight) + scanPitch;
        } else if (bloodFocusActive) {
            pitchTarget = std::clamp(PitchToPoint(bloodFocusTarget_), -0.34f, 0.22f);
        } else if (threat && chaseLookBackWeight > 0.0f) {
            pitchTarget = Lerp(pitchTarget, chaseLookBackPitch_, chaseLookBackWeight);
        } else if (branchLookActive) {
            pitchTarget = Lerp(pitchTarget, branchLookPitch_, branchLookWeight * (branchLookPaused_ ? 0.88f : 0.72f));
        } else if (roomSurveyActive) {
            pitchTarget = Lerp(pitchTarget, RoomSurveyPitch(), roomSurveyWeight * 0.78f);
        }
        if (!panicActive && !softStopActive && exitLookBlend_ > 0.001f) {
            float exitPitch = std::clamp(PitchToPoint(exitLookFocus_), -0.24f, 0.26f);
            pitchTarget = Lerp(pitchTarget, exitPitch, std::min(0.92f, exitLookBlend_ * 0.92f));
        }
        pitchTarget -= stumbleAmount * 0.035f * (1.0f - chaseLookBackWeight * 0.85f);
        lookPitch_ += (pitchTarget - lookPitch_) * std::min(1.0f, dt * (panicActive ? 3.5f : (ventReactionActive ? Lerp(2.0f, 7.2f, ventLookWeight) : 2.2f)));

        bool runningToExit = !path_.empty() && path_.back() == maze_.exit;
        float calmSpeed = IsRoomLike(CameraTile()) ? settings_.roomSpeed : settings_.walkSpeed;
        float runTarget = settings_.runSpeed * (runningToExit ? 1.08f : 0.98f);
        float panicBlend = SmoothStep(0.08f, 0.84f, chasePanic_);
        float speedTarget = Lerp(calmSpeed, runTarget, panicBlend);
        if (softStopActive) speedTarget = 0.0f;
        if (!panicActive && branchLookActive && !softStopActive) {
            speedTarget *= Lerp(1.0f, 0.44f, branchLookWeight);
        }
        if (!panicActive && roomSurveyActive && !softStopActive) {
            speedTarget *= Lerp(1.0f, 0.56f, roomSurveyWeight);
        }
        float dreadSpeedBoost = settings_.dreadEnabled
            ? dreadLevel_ * (panicActive ? settings_.dreadRunSpeedBoost * 0.34f : settings_.dreadWalkSpeedBoost * 0.55f)
            : 0.0f;
        speedTarget *= 1.0f + dreadSpeedBoost;
        speedTarget *= 1.0f - stumbleAmount * 0.16f;
        if (smoothedMoveSpeed_ <= 0.001f && speedTarget > 0.001f) smoothedMoveSpeed_ = 0.001f;
        float accel = panicActive ? (monsterRunLaunchActive_ ? 8.0f : 4.2f) : 0.90f;
        float decel = panicActive ? 0.42f : (ventReactionActive ? 1.35f : (softStopActive ? 0.86f : 0.70f));
        smoothedMoveSpeed_ = MoveTowards(smoothedMoveSpeed_, speedTarget,
            (speedTarget > smoothedMoveSpeed_ ? accel : decel) * dt);
        float speed = smoothedMoveSpeed_;
        float moveDistance = std::min(dist, speed * dt);
        float invDist = 1.0f / std::max(0.001f, dist);
        bool movedSafely = MoveCameraSafely(dx * invDist * moveDistance, dz * invDist * moveDistance,
            moveDistance, speed, targetTileForMove, freeRunMove);
        if (movedSafely) {
            if (monsterRunLaunchActive_) {
                monsterRunLaunchMeters_ = std::min(3.0f, monsterRunLaunchMeters_ + moveDistance);
                if (monsterRunLaunchMeters_ >= 3.0f) monsterRunLaunchActive_ = false;
            }
            if (freeRunMove && moveIndex > pathIndex_) {
                pathIndex_ = moveIndex;
            }
        }
        float runBob = Clamp01((speed - settings_.walkSpeed) / std::max(0.1f, settings_.runSpeed * 1.55f - settings_.walkSpeed));
        runIntensity_ += (runBob - runIntensity_) * std::min(1.0f, dt * (runBob > runIntensity_ ? (panicActive ? 7.4f : 4.2f) : 1.15f));
        float effortTarget = Clamp01(runIntensity_ * (panicActive ? 1.18f : 0.82f) + chasePanic_ * 0.46f);
        runEffort_ += (effortTarget - runEffort_) * std::min(1.0f, dt * (effortTarget > runEffort_ ? (panicActive ? 2.4f : 0.55f) : 0.24f));
        breathPhase_ += dt * (1.15f + runEffort_ * 4.8f + runIntensity_ * 1.45f) * kPi;
        if (breathPhase_ > kPi * 128.0f) {
            breathPhase_ = std::fmod(breathPhase_, kPi * 2.0f);
        }
        float bobAmount = settings_.headBobAmount * Lerp(0.92f, 2.35f, Clamp01(runBob * 0.72f + runEffort_ * 0.46f));
        float breathAmount = 0.0035f + runIntensity_ * 0.028f + runEffort_ * 0.065f;
        float breathY = std::sin(breathPhase_) * breathAmount;
        float walkY = 1.43f + std::abs(std::sin(stepPhase_)) * bobAmount +
            std::sin(stepPhase_ * 0.5f) * (0.012f + runEffort_ * 0.030f) +
            breathY - stumbleAmount * 0.055f;
        if (panicActive && monsterRunLaunchActive_) {
            float launchT = Clamp01(monsterRunLaunchMeters_ / 3.0f);
            float launchWeight = 1.0f - SmoothStep(0.0f, 1.0f, launchT);
            float heavyStep = std::sin(stepPhase_ * 1.08f + 0.45f);
            float impact = std::abs(std::sin(stepPhase_ * 0.54f));
            walkY += (heavyStep * 0.185f + impact * 0.105f) * launchWeight;
        }
        if (softStopActive) {
            float stopPose = SmoothStep(0.0f, 1.0f, 1.0f - Clamp01(speed / std::max(0.05f, calmSpeed * 0.72f)));
            float idleY = 1.47f + std::sin(time_ * 2.1f) * 0.008f;
            if (ventReactionActive) {
                float ventProgress = 1.0f - ventReactionTimer_ / std::max(0.001f, ventReactionDuration_);
                float highVent = SmoothStep(0.40f, 0.76f,
                    Clamp01((ventReactionTarget_.y - 0.36f) / std::max(0.20f, settings_.wallHeightMeters - 0.72f)));
                float duck = SmoothStep(0.0f, 0.18f, ventReactionElapsed - ventReactionLookDelay_ * 0.45f) *
                    (1.0f - SmoothStep(0.62f, 1.0f, ventProgress)) * highVent;
                float brace = ventBackAwayWeight * (1.0f - highVent) * 0.34f;
                idleY = Lerp(idleY, 1.10f + std::sin(time_ * 8.5f) * 0.006f, duck);
                idleY = Lerp(idleY, 1.40f + std::sin(time_ * 7.2f) * 0.005f, brace);
                stopPose = std::max(stopPose, std::max(duck, ventLookWeight * 0.36f));
            }
            camera_.y = Lerp(walkY, idleY, stopPose);
        } else {
            camera_.y = walkY;
        }
    }
