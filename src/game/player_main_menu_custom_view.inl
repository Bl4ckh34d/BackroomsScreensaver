        if (menuRuntime_.customViewActive) {
            XMFLOAT3 customCamera{};
            float customYaw = 0.0f;
            float customPitch = 0.0f;
            MainMenuCustomCameraPose(customCamera, customYaw, customPitch);
            if (menuRuntime_.customViewTarget) {
                menuRuntime_.customViewTimer += std::max(0.0f, dt);
                float t = menuRuntime_.customViewTimer;
                float turn = SmoothStep(0.02f, 1.15f, t);
                float walk = SmoothStep(0.42f, 2.10f, t);
                float settle = SmoothStep(2.00f, 2.72f, t);
                float bobAge = std::max(0.0f, t - 0.32f);
                float bob = (std::sin(bobAge * 8.9f) * 0.026f + std::sin(bobAge * 17.8f) * 0.009f) *
                    SmoothStep(0.42f, 0.80f, t) * (1.0f - SmoothStep(2.05f, 2.70f, t));
                float lean = std::sin(turn * kPi) * 0.030f;
                gameWorld_.player.position.x = Lerp(menuRuntime_.customStartCamera.x, customCamera.x, walk);
                gameWorld_.player.position.y = Lerp(menuRuntime_.customStartCamera.y, customCamera.y, walk) + bob;
                gameWorld_.player.position.z = Lerp(menuRuntime_.customStartCamera.z, customCamera.z, walk) + lean;
                gameWorld_.player.yaw = menuRuntime_.customStartYaw + AngleWrap(customYaw - menuRuntime_.customStartYaw) * Clamp01(turn + settle * 0.04f);
                gameWorld_.player.pitch = Lerp(menuRuntime_.customStartPitch, customPitch, SmoothStep(0.25f, 1.00f, turn));
                if (t >= 2.72f) {
                    gameWorld_.player.position = customCamera;
                    gameWorld_.player.yaw = customYaw;
                    gameWorld_.player.pitch = customPitch;
                }
            } else {
                menuRuntime_.customReturnTimer += std::max(0.0f, dt);
                float t = menuRuntime_.customReturnTimer;
                float turn = SmoothStep(0.02f, 0.95f, t);
                float walk = SmoothStep(0.20f, 1.46f, t);
                float bob = (std::sin(t * 9.8f) * 0.020f + std::sin(t * 19.6f) * 0.007f) *
                    SmoothStep(0.20f, 0.48f, t) * (1.0f - SmoothStep(1.22f, 1.62f, t));
                gameWorld_.player.position.x = Lerp(menuRuntime_.customReturnCamera.x, menuRuntime_.customStartCamera.x, walk);
                gameWorld_.player.position.y = Lerp(menuRuntime_.customReturnCamera.y, menuRuntime_.customStartCamera.y, walk) + bob;
                gameWorld_.player.position.z = Lerp(menuRuntime_.customReturnCamera.z, menuRuntime_.customStartCamera.z, walk);
                gameWorld_.player.yaw = menuRuntime_.customReturnYaw + AngleWrap(menuRuntime_.customStartYaw - menuRuntime_.customReturnYaw) * turn;
                gameWorld_.player.pitch = Lerp(menuRuntime_.customReturnPitch, menuRuntime_.customStartPitch, SmoothStep(0.16f, 1.0f, turn));
                if (t >= 1.68f) {
                    menuRuntime_.customViewActive = false;
                    gameWorld_.player.position = menuRuntime_.customStartCamera;
                    gameWorld_.player.yaw = menuRuntime_.customStartYaw;
                    gameWorld_.player.pitch = menuRuntime_.customStartPitch;
                }
            }
            float customIdle = menuRuntime_.customViewTarget
                ? SmoothStep(1.72f, 2.72f, menuRuntime_.customViewTimer)
                : (1.0f - SmoothStep(0.0f, 0.72f, menuRuntime_.customReturnTimer));
            if (customIdle > 0.001f) {
                float breathe = std::sin(timeRuntime_.time * 1.12f + 0.4f) * 0.0075f +
                    std::sin(timeRuntime_.time * 0.47f + 2.1f) * 0.0040f;
                float shoulder = std::sin(timeRuntime_.time * 0.66f + 1.6f) * 0.0065f;
                gameWorld_.player.position.y += breathe * customIdle;
                gameWorld_.player.position.z += shoulder * customIdle;
                gameWorld_.player.yaw += (std::sin(timeRuntime_.time * 0.71f + 0.3f) * 0.009f +
                    std::sin(timeRuntime_.time * 1.53f + 2.4f) * 0.0035f) * customIdle;
                gameWorld_.player.pitch = std::clamp(gameWorld_.player.pitch + (std::cos(timeRuntime_.time * 0.83f + 1.1f) * 0.0065f +
                    std::sin(timeRuntime_.time * 1.31f + 0.8f) * 0.0028f) * customIdle, -0.42f, 0.38f);
            }
            gameWorld_.player.bodyYaw = gameWorld_.player.yaw;
            targetYaw = gameWorld_.player.yaw;
            targetPitch = gameWorld_.player.pitch;
            if (hostRuntime_.width > 0 && hostRuntime_.height > 0) {
                MenuPlaquePlacement panel = MenuCustomPanelPlacement();
                XMFLOAT3 viewForward = Normalize3(DirectionFromYawPitch(gameWorld_.player.yaw, gameWorld_.player.pitch), {0.0f, 0.0f, 1.0f});
                XMFLOAT3 viewRight = Normalize3(Cross3({0.0f, 1.0f, 0.0f}, viewForward), {1.0f, 0.0f, 0.0f});
                XMFLOAT3 viewUp = Normalize3(Cross3(viewForward, viewRight), {0.0f, 1.0f, 0.0f});
                float aspect = static_cast<float>(std::max<LONG>(1, hostRuntime_.width)) / static_cast<float>(std::max<LONG>(1, hostRuntime_.height));
                float tanHalfFov = std::tan(44.0f * kPi / 180.0f);
                float ndcX = menuRuntime_.pointerX * 2.0f - 1.0f;
                float ndcY = 1.0f - menuRuntime_.pointerY * 2.0f;
                XMFLOAT3 ray = Normalize3(Add3(viewForward,
                    Add3(Scale3(viewRight, ndcX * aspect * tanHalfFov), Scale3(viewUp, ndcY * tanHalfFov))), viewForward);
                float denom = Dot3(ray, panel.inward);
                if (std::abs(denom) > 0.001f) {
                    float t = Dot3(Sub3(panel.center, gameWorld_.player.position), panel.inward) / denom;
                    if (t > 0.05f && t < 7.0f) {
                        XMFLOAT3 aimPoint = Add3(gameWorld_.player.position, Scale3(ray, t));
                        targetYaw = YawToPoint(aimPoint) + jitterYaw * 0.35f;
                        targetPitch = PitchToPoint(aimPoint) + jitterPitch * 0.35f;
                    }
                }
            }
            viewRuntime_.previousCameraYaw = gameWorld_.player.yaw;
            viewRuntime_.previousCameraPitch = gameWorld_.player.pitch;
        }
