// Player camera pose and flashlight aim presentation helpers. 
// Included inside Renderer's private section from player_camera_movement.inl.

    static XMFLOAT3 DirectionFromYawPitch(float yaw, float pitch) {
        float cp = std::cos(pitch);
        return {std::sin(yaw) * cp, std::sin(pitch), std::cos(yaw) * cp};
    }

    float YawToPoint(const XMFLOAT3& target) const {
        GameWorldRenderSnapshot world = gameWorld_.BuildRenderSnapshot();
        return std::atan2(target.x - world.playerPosition.x, target.z - world.playerPosition.z);
    }

    float PitchToPoint(const XMFLOAT3& target) const {
        GameWorldRenderSnapshot world = gameWorld_.BuildRenderSnapshot();
        float dx = target.x - world.playerPosition.x;
        float dy = target.y - world.playerPosition.y;
        float dz = target.z - world.playerPosition.z;
        float horizontal = std::sqrt(dx * dx + dz * dz);
        return std::atan2(dy, std::max(0.001f, horizontal));
    }

    XMFLOAT3 Forward() const {
        GameWorldRenderSnapshot world = gameWorld_.BuildRenderSnapshot();
        return {std::sin(world.playerYaw), 0.0f, std::cos(world.playerYaw)};
    }

    XMFLOAT3 FlashlightForward() const {
        return DirectionFromYawPitch(viewRuntime_.flashlightYaw, viewRuntime_.flashlightPitch);
    }

    XMFLOAT3 FlashlightOrigin() const {
        GameWorldRenderSnapshot world = gameWorld_.BuildRenderSnapshot();
        XMFLOAT3 forward = Normalize3(FlashlightForward(), {0.0f, 0.0f, 1.0f});
        XMFLOAT3 right = Normalize3(Cross3({0.0f, 1.0f, 0.0f}, forward), {1.0f, 0.0f, 0.0f});
        return Add3(world.playerPosition, Add3(Scale3(right, 0.16f), Add3(Scale3({0.0f, 1.0f, 0.0f}, -0.18f), Scale3(forward, 0.08f))));
    }

    float FlashlightFocusTargetDistance() const {
        GameWorldRenderSnapshot world = gameWorld_.BuildRenderSnapshot();
        float maxDist = std::clamp(settingsRuntime_.live.flashlightShadowDistanceMeters * 0.56f, 3.6f, 9.5f);
        XMFLOAT3 origin = world.playerPosition;
        XMFLOAT3 dir = Normalize3(DirectionFromYawPitch(world.playerYaw, world.playerPitch), {0.0f, 0.0f, 1.0f});
        float horizontalLen = std::sqrt(dir.x * dir.x + dir.z * dir.z);
        float target = maxDist;
        if (horizontalLen > 0.04f) {
            float yaw = std::atan2(dir.x, dir.z);
            target = std::min(target, ViewRayOpenDistance(yaw, maxDist * horizontalLen) / horizontalLen);
        }
        if (dir.y > 0.025f) {
            target = std::min(target, (settingsRuntime_.live.wallHeightMeters - origin.y - 0.08f) / dir.y);
        } else if (dir.y < -0.025f) {
            target = std::min(target, (0.08f - origin.y) / dir.y);
        }
        return std::clamp(target, 0.55f, maxDist);
    }

    void UpdateFlashlightAim(float dt) {
        GameWorldRenderSnapshot world = gameWorld_.BuildRenderSnapshot();
        dt = std::max(0.001f, dt);
        constexpr float kDartFrequencyDivisor = 4.0f;
        float yawDelta = AngleWrap(world.playerYaw - viewRuntime_.previousCameraYaw);
        float pitchDelta = world.playerPitch - viewRuntime_.previousCameraPitch;
        float turnRate = (std::abs(yawDelta) + std::abs(pitchDelta) * 0.65f) / dt;
        float fastTurn = Clamp01((turnRate - 2.1f) / 5.8f);
        float blurScale = monsterPreview_.active ? 0.0f : std::clamp(settingsRuntime_.live.motionBlurAmount, 0.0f, 2.0f);
        XMFLOAT2 blurTarget{
            std::clamp(-yawDelta * 0.36f * blurScale, -0.045f, 0.045f),
            std::clamp(pitchDelta * 0.48f * blurScale, -0.045f, 0.045f)
        };
        float blurFollow = std::min(1.0f, dt * (fastTurn > 0.20f ? 18.0f : 7.0f));
        viewRuntime_.cameraMotionBlur.x += (blurTarget.x - viewRuntime_.cameraMotionBlur.x) * blurFollow;
        viewRuntime_.cameraMotionBlur.y += (blurTarget.y - viewRuntime_.cameraMotionBlur.y) * blurFollow;
        bool calmExplorationTurn = !ChasePanicActive() && viewRuntime_.dangerLevel < 0.16f && DreadPressure() < 0.18f;
        if (fastTurn > 0.0f && !calmExplorationTurn) {
            viewRuntime_.flashlightAgitation = std::max(viewRuntime_.flashlightAgitation, fastTurn);
            viewRuntime_.flashlightDartCooldown = std::min(viewRuntime_.flashlightDartCooldown, RandRange(0.09f, 0.22f) * kDartFrequencyDivisor);
        }

        viewRuntime_.flashlightAgitation = std::max(0.0f, viewRuntime_.flashlightAgitation - dt * 0.42f);
        viewRuntime_.panicFlashlightTimer = std::max(0.0f, viewRuntime_.panicFlashlightTimer - dt);
        viewRuntime_.flashlightDartCooldown = std::max(0.0f, viewRuntime_.flashlightDartCooldown - dt);
        viewRuntime_.flashlightDartTimer = std::max(0.0f, viewRuntime_.flashlightDartTimer - dt);

        float dreadPressure = DreadPressure();
        float activeAgitation = std::max(viewRuntime_.flashlightAgitation, dreadPressure * 0.68f);
        auto mergeFlashlightHold = [&](float yawOffset, float pitchOffset, float carry) {
            constexpr float kHoldYawLimit = 0.24f;
            constexpr float kHoldPitchLimit = 0.15f;
            viewRuntime_.flashlightHoldYaw = std::clamp(viewRuntime_.flashlightHoldYaw * carry + yawOffset, -kHoldYawLimit, kHoldYawLimit);
            viewRuntime_.flashlightHoldPitch = std::clamp(viewRuntime_.flashlightHoldPitch * carry + pitchOffset, -kHoldPitchLimit, kHoldPitchLimit);
        };

        viewRuntime_.flashlightSnapCooldown = std::max(0.0f, viewRuntime_.flashlightSnapCooldown - dt);
        viewRuntime_.flashlightSnapTimer = std::max(0.0f, viewRuntime_.flashlightSnapTimer - dt);
        if (viewRuntime_.flashlightSnapTimer <= 0.0f && viewRuntime_.flashlightSnapCooldown <= 0.0f) {
            float pressure = Clamp01(activeAgitation * 0.72f + dreadPressure * 0.48f + fastTurn * 0.40f);
            viewRuntime_.flashlightSnapSharp = RandRange(0.0f, 1.0f) < (0.22f + pressure * 0.56f);
            viewRuntime_.flashlightSnapDuration = viewRuntime_.flashlightSnapSharp
                ? RandRange(0.055f, 0.14f)
                : RandRange(0.34f, 1.18f);
            viewRuntime_.flashlightSnapTimer = viewRuntime_.flashlightSnapDuration;
            float sharpBoost = viewRuntime_.flashlightSnapSharp ? 1.55f : 0.62f;
            float amount = (0.45f + pressure * 1.10f) * settingsRuntime_.live.flashlightSwayAmount * sharpBoost;
            viewRuntime_.flashlightSnapYaw = RandRange(-0.075f, 0.075f) * amount;
            viewRuntime_.flashlightSnapPitch = RandRange(-0.040f, 0.050f) * amount;
            mergeFlashlightHold(viewRuntime_.flashlightSnapYaw, viewRuntime_.flashlightSnapPitch, viewRuntime_.flashlightSnapSharp ? 0.42f : 0.64f);
            viewRuntime_.flashlightSnapCooldown = viewRuntime_.flashlightSnapSharp
                ? RandRange(0.34f, 0.95f) * (1.0f - pressure * 0.28f)
                : RandRange(0.80f, 2.60f) * (1.0f - pressure * 0.22f);
            viewRuntime_.flashlightSnapCooldown = std::max(0.18f, viewRuntime_.flashlightSnapCooldown);
        }

        if (activeAgitation > 0.14f && viewRuntime_.flashlightDartTimer <= 0.0f && viewRuntime_.flashlightDartCooldown <= 0.0f) {
            viewRuntime_.flashlightDartDuration = RandRange(0.105f, 0.24f);
            viewRuntime_.flashlightDartTimer = viewRuntime_.flashlightDartDuration;
            float amount = SmoothStep(0.10f, 1.0f, activeAgitation);
            float dreadJitter = 1.0f + dreadPressure * 0.75f;
            viewRuntime_.flashlightDartYaw = RandRange(-0.22f, 0.22f) * amount * dreadJitter * settingsRuntime_.live.flashlightPanicDartAmount;
            viewRuntime_.flashlightDartPitch = RandRange(-0.13f, 0.10f) * amount * dreadJitter * settingsRuntime_.live.flashlightPanicDartAmount;
            mergeFlashlightHold(std::clamp(viewRuntime_.flashlightDartYaw * 0.46f, -0.18f, 0.18f),
                std::clamp(viewRuntime_.flashlightDartPitch * 0.48f, -0.11f, 0.11f), 0.54f);
            float cooldown = RandRange(0.26f, 0.62f) * (1.0f - amount * 0.18f) * (1.0f - dreadPressure * 0.12f);
            viewRuntime_.flashlightDartCooldown = std::max(0.16f * kDartFrequencyDivisor, cooldown * kDartFrequencyDivisor);
        }

        float gentleScale = (1.0f + viewRuntime_.dangerLevel * 1.10f + activeAgitation * 1.05f + dreadPressure * 0.95f) * settingsRuntime_.live.flashlightSwayAmount;
        float gentleYaw =
            std::sin(timeRuntime_.time * 0.31f + std::sin(timeRuntime_.time * 0.13f) * 1.6f) * 0.026f * gentleScale +
            std::sin(timeRuntime_.time * 1.18f + std::sin(timeRuntime_.time * 0.41f) * 0.7f) * 0.044f * gentleScale +
            std::sin(timeRuntime_.time * 2.85f + 1.7f) * 0.020f * gentleScale;
        float gentlePitch =
            std::sin(timeRuntime_.time * 0.27f + 1.4f) * 0.014f * gentleScale +
            std::sin(timeRuntime_.time * 1.02f + 2.4f) * 0.022f * gentleScale +
            std::sin(timeRuntime_.time * 2.32f) * 0.011f * gentleScale;

        float tremorScale = (0.0035f + activeAgitation * 0.0075f + dreadPressure * 0.0105f) * settingsRuntime_.live.flashlightSwayAmount;
        float tremorYaw =
            std::sin(timeRuntime_.time * 12.7f + std::sin(timeRuntime_.time * 3.9f) * 1.4f) * 0.70f +
            std::sin(timeRuntime_.time * 28.9f + 2.1f) * 0.24f +
            std::sin(timeRuntime_.time * 61.0f + 0.7f) * 0.09f;
        float tremorPitch =
            std::sin(timeRuntime_.time * 10.6f + 1.8f) * 0.58f +
            std::sin(timeRuntime_.time * 24.4f + std::sin(timeRuntime_.time * 5.5f)) * 0.28f +
            std::sin(timeRuntime_.time * 57.0f + 2.6f) * 0.08f;
        gentleYaw += tremorYaw * tremorScale;
        gentlePitch += tremorPitch * tremorScale * 0.72f;

        float stepRun = Clamp01(world.playerRunIntensity * 0.78f + world.playerRunEffort * 0.62f);
        if (stepRun > 0.001f) {
            float stepWave = std::sin(world.playerStepPhase);
            float crossWave = std::sin(world.playerStepPhase * 0.5f + 0.65f);
            float runSwing = settingsRuntime_.live.flashlightSwayAmount * stepRun;
            gentleYaw += (stepWave * 0.040f + crossWave * 0.024f) * runSwing;
            gentlePitch += (-stepWave * 0.028f + std::abs(crossWave) * 0.018f) * runSwing;
        }

        if (viewRuntime_.panicFlashlightTimer > 0.0f && viewRuntime_.panicFlashlightDuration > 0.001f) {
            float t = 1.0f - viewRuntime_.panicFlashlightTimer / viewRuntime_.panicFlashlightDuration;
            float envelope = (1.0f - SmoothStep(0.48f, 1.0f, t)) * SmoothStep(0.0f, 0.10f, t);
            float swing = std::sin(t * kPi * 3.35f);
            float snap = std::sin(t * kPi * 7.10f) * 0.26f;
            float panic = envelope * (0.78f + world.monsterChasePanic * 0.42f) * settingsRuntime_.live.flashlightPanicDartAmount;
            gentleYaw += (swing * 0.080f + snap * 0.034f) * panic;
            gentlePitch += (swing * 0.052f - envelope * 0.030f) * panic;
        }

        bool impulseHolding = viewRuntime_.flashlightSnapTimer > 0.0f || viewRuntime_.flashlightDartTimer > 0.0f;
        float holdPressure = Clamp01(activeAgitation + dreadPressure * 0.7f);
        float holdReturnRate = impulseHolding
            ? Lerp(0.18f, 0.08f, holdPressure)
            : Lerp(1.10f, 0.42f, holdPressure);
        float holdReturn = 1.0f - std::exp(-dt * holdReturnRate);
        viewRuntime_.flashlightHoldYaw += (0.0f - viewRuntime_.flashlightHoldYaw) * holdReturn;
        viewRuntime_.flashlightHoldPitch += (0.0f - viewRuntime_.flashlightHoldPitch) * holdReturn;

        float targetYaw = world.playerYaw + gentleYaw + viewRuntime_.flashlightHoldYaw;
        float targetPitch = std::clamp(world.playerPitch + gentlePitch + viewRuntime_.flashlightHoldPitch, -0.48f, 0.34f);
        float propInspectWeight = 0.0f;
        if (!ChasePanicActive() && viewRuntime_.propLookTimer > 0.0f && viewRuntime_.propLookDuration > 0.001f) {
            float t = 1.0f - viewRuntime_.propLookTimer / viewRuntime_.propLookDuration;
            propInspectWeight = SmoothStep(0.0f, 0.22f, t) * (1.0f - SmoothStep(0.72f, 1.0f, t));
            float scan = std::sin((t * 1.25f + viewRuntime_.propLookScanSeed) * kPi * 2.0f);
            float fineScan = std::sin((t * 2.70f + viewRuntime_.propLookScanSeed * 1.7f) * kPi * 2.0f);
            float propYaw = YawToPoint(viewRuntime_.propLookTarget) + scan * 0.034f + fineScan * 0.010f;
            float propPitch = std::clamp(PitchToPoint(viewRuntime_.propLookTarget) + scan * 0.012f - fineScan * 0.006f, -0.40f, 0.26f);
            targetYaw += AngleWrap(propYaw - targetYaw) * (propInspectWeight * 0.72f);
            targetPitch += (propPitch - targetPitch) * (propInspectWeight * 0.68f);
        }
        float snapFollowBoost = viewRuntime_.flashlightSnapSharp && viewRuntime_.flashlightSnapTimer > 0.0f ? 0.55f : 0.0f;
        float dartFollowBoost = viewRuntime_.flashlightDartTimer > 0.0f ? 0.42f : 0.0f;
        float followSpeed = Lerp(3.7f, 17.5f, std::max(fastTurn,
            std::max(propInspectWeight * 0.26f, std::max(activeAgitation * 0.7f, std::max(snapFollowBoost, dartFollowBoost))))) * settingsRuntime_.live.flashlightFollowSpeed;
        viewRuntime_.flashlightYaw += AngleWrap(targetYaw - viewRuntime_.flashlightYaw) * std::min(1.0f, dt * followSpeed);
        viewRuntime_.flashlightPitch += (targetPitch - viewRuntime_.flashlightPitch) * std::min(1.0f, dt * followSpeed);

        viewRuntime_.previousCameraYaw = world.playerYaw;
        viewRuntime_.previousCameraPitch = world.playerPitch;
    }
