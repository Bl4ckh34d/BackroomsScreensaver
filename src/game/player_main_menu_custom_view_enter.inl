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
