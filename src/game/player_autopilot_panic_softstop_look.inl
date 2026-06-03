        if (panicActive) {
            desiredYaw = gameWorld_.player.bodyYaw;
            if (threat && chaseLookBackWeight > 0.0f) {
                desiredYaw += AngleWrap(viewRuntime_.chaseLookBackYaw - desiredYaw) * (chaseLookBackWeight * 0.96f);
            }
            desiredYaw += viewRuntime_.stumbleYawOffset * stumbleAmount * (1.0f - chaseLookBackWeight);
        } else if (softStopActive) {
            if (ventReactionActive) {
                if (ventLookWeight > 0.001f) {
                    float ventTargetYaw = YawToPoint(viewRuntime_.ventReactionTarget);
                    float scanWeight = SmoothStep(0.10f, 1.0f, ventLookWeight);
                    float scanYaw = (std::sin(timeRuntime_.time * 7.9f + viewRuntime_.ventReactionScanSeed) * 0.042f +
                        std::sin(timeRuntime_.time * 13.7f + viewRuntime_.ventReactionScanSeed * 1.7f) * 0.018f) * scanWeight;
                    desiredYaw = gameWorld_.player.yaw + AngleWrap(ventTargetYaw - gameWorld_.player.yaw) * ventLookWeight + scanYaw;
                } else {
                    desiredYaw = gameWorld_.player.yaw + std::sin(timeRuntime_.time * 18.0f + viewRuntime_.ventReactionScanSeed) * 0.010f;
                }
            } else if (bloodFocusActive) desiredYaw = YawToPoint(scareRuntime_.bloodFocusTarget);
            else if (branchLookActive) {
                float lock = std::max(0.58f, branchLookWeight);
                desiredYaw = gameWorld_.player.yaw + AngleWrap(BranchLookTargetYaw() - gameWorld_.player.yaw) * lock;
            } else if (roomSurveyActive) desiredYaw = RoomSurveyYaw();
            else desiredYaw = gameWorld_.player.yaw;
