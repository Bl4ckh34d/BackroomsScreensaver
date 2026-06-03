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
