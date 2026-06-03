        UpdateMainMenuLampOverrides();
        if (menuRuntime_.startTransitionActive) {
            menuRuntime_.startTransitionTimer += std::max(0.0f, dt);
            float t = menuRuntime_.startTransitionTimer;
            constexpr float kMenuEyeHeight = 1.45f;
            Tile hallEnd{std::clamp(gameWorld_.maze.start.x + 1, 1, gameWorld_.maze.w - 2), 1};
            XMFLOAT3 hallCenter = gameWorld_.maze.WorldCenter(hallEnd, 0.0f);
            float turn = SmoothStep(0.08f, 1.62f, t);
            float plantedTurn = SmoothStep(0.06f, 0.92f, t) * (1.0f - SmoothStep(1.10f, 1.82f, t));
            float walkAge = std::max(0.0f, t - 1.48f);
            float align = SmoothStep(1.45f, 2.45f, t);
            constexpr float kMenuWalkSpeed = 1.18f;
            float walkedMeters = walkAge * kMenuWalkSpeed;
            XMFLOAT3 walkPos = menuRuntime_.startCamera;
            if (menuRuntime_.startTransitionFromCustomView) {
                walkPos.x = Lerp(menuRuntime_.startCamera.x, hallCenter.x, SmoothStep(1.02f, 1.82f, t));
                walkPos.z = menuRuntime_.startCamera.z - walkedMeters * 1.08f;
            } else {
                walkPos.x = Lerp(menuRuntime_.startCamera.x, hallCenter.x, align);
                walkPos.z = menuRuntime_.startCamera.z - walkedMeters;
            }
            float stepBob = std::sin(walkedMeters * 4.65f) * 0.028f + std::sin(walkedMeters * 9.30f) * 0.010f;
            float bobWeight = SmoothStep(1.52f, 2.08f, t);
            float turnBreath = std::sin(t * 8.6f) * 0.006f * plantedTurn;
            float turnLean = std::sin(turn * kPi) * 0.035f;
            gameWorld_.player.position = walkPos;
            float leanWeight = menuRuntime_.startTransitionFromCustomView ? (1.0f - SmoothStep(0.88f, 1.64f, t)) : (1.0f - align);
            gameWorld_.player.position.x += std::sin(menuRuntime_.startYaw + kPi * 0.5f) * turnLean * leanWeight;
            gameWorld_.player.position.z += std::cos(menuRuntime_.startYaw + kPi * 0.5f) * turnLean * leanWeight;
            gameWorld_.player.position.y = kMenuEyeHeight + turnBreath + stepBob * bobWeight;
            XMFLOAT3 lookPoint{hallCenter.x, 1.34f, gameWorld_.player.position.z - gameWorld_.maze.tileD * 5.0f};
            float targetYaw = YawToPoint(lookPoint);
            float targetPitch = std::clamp(PitchToPoint(lookPoint), -0.28f, 0.22f);
            float yawEase = menuRuntime_.startTransitionFromCustomView
                ? SmoothStep(0.02f, 1.05f, t) + std::sin(turn * kPi) * 0.025f
                : turn + std::sin(turn * kPi) * 0.035f;
            gameWorld_.player.yaw = menuRuntime_.startYaw + AngleWrap(targetYaw - menuRuntime_.startYaw) * Clamp01(yawEase);
            gameWorld_.player.bodyYaw = menuRuntime_.startYaw + AngleWrap(gameWorld_.player.yaw - menuRuntime_.startYaw) * SmoothStep(0.0f, 1.0f, turn);
            gameWorld_.player.pitch = Lerp(menuRuntime_.startPitch, targetPitch, SmoothStep(0.18f, 1.0f, turn)) + turnBreath * 0.18f;
            viewRuntime_.flashlightYaw += AngleWrap(gameWorld_.player.yaw - viewRuntime_.flashlightYaw) * std::min(1.0f, dt * 9.5f);
            viewRuntime_.flashlightPitch += (gameWorld_.player.pitch - viewRuntime_.flashlightPitch) * std::min(1.0f, dt * 9.5f);
            viewRuntime_.previousCameraYaw = gameWorld_.player.yaw;
            viewRuntime_.previousCameraPitch = gameWorld_.player.pitch;
            menuRuntime_.startTransitionFade = SmoothStep(2.55f, 4.05f, t);
            if (menuRuntime_.darkLayerOneRun && t > 1.10f && !menuRuntime_.lampBurstPlayed) menuRuntime_.lampBurstPending = true;
            if (t >= 4.80f) {
                menuRuntime_.startTransitionActive = false;
                menuRuntime_.startTransitionComplete = true;
                menuRuntime_.startTransitionFade = 1.0f;
            }
            UpdateMainMenuAmbientSparks(dt);
            UpdateBrokenRuntimeLampSparks(dt, 1.65f, 5.8f, 0.22f);
            UpdateSparks(dt);
            UpdateAirParticles(dt);
            UpdateAirParticleFocus(dt);
            UpdateDreadMeterDisplay(dt);
            return;
        }
