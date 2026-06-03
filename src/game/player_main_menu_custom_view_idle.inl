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
