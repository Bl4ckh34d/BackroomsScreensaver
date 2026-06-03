        menuRuntime_.pointerX += (menuRuntime_.pointerTargetX - menuRuntime_.pointerX) * std::min(1.0f, dt * 7.5f);
        menuRuntime_.pointerY += (menuRuntime_.pointerTargetY - menuRuntime_.pointerY) * std::min(1.0f, dt * 7.5f);

        float handheldYaw = std::sin(timeRuntime_.time * 0.73f) * 0.018f + std::sin(timeRuntime_.time * 1.37f + 1.7f) * 0.011f +
            std::sin(timeRuntime_.time * 2.91f + 0.3f) * 0.004f;
        float handheldPitch = std::cos(timeRuntime_.time * 0.61f + 0.8f) * 0.011f + std::sin(timeRuntime_.time * 1.19f + 2.2f) * 0.007f;
        float jitterYaw = std::sin(timeRuntime_.time * 2.41f) * 0.012f + std::sin(timeRuntime_.time * 7.17f) * 0.005f;
        float jitterPitch = std::cos(timeRuntime_.time * 2.03f) * 0.007f + std::sin(timeRuntime_.time * 5.83f) * 0.004f;
        gameWorld_.player.yaw = menuRuntime_.baseYaw + handheldYaw;
        gameWorld_.player.bodyYaw = gameWorld_.player.yaw;
        gameWorld_.player.pitch = std::clamp(menuRuntime_.basePitch + handheldPitch, -0.42f, 0.42f);

        float hoverFlicker = (menuRuntime_.darkLayerOneRun && menuRuntime_.singlePlayerHover) ? (std::sin(timeRuntime_.time * 36.0f) * 0.012f + std::sin(timeRuntime_.time * 81.0f) * 0.006f) : 0.0f;
        float targetYaw = menuRuntime_.baseYaw + (menuRuntime_.pointerX - 0.5f) * 1.12f + jitterYaw + hoverFlicker;
        float targetPitch = menuRuntime_.basePitch + (0.5f - menuRuntime_.pointerY) * 0.72f + jitterPitch - std::abs(hoverFlicker) * 0.35f;
        if (hostRuntime_.width > 0 && hostRuntime_.height > 0) {
            XMFLOAT3 viewForward = Normalize3(DirectionFromYawPitch(gameWorld_.player.yaw, gameWorld_.player.pitch), {0.0f, 0.0f, 1.0f});
            XMFLOAT3 viewRight = Normalize3(Cross3({0.0f, 1.0f, 0.0f}, viewForward), {1.0f, 0.0f, 0.0f});
            XMFLOAT3 viewUp = Normalize3(Cross3(viewForward, viewRight), {0.0f, 1.0f, 0.0f});
            float aspect = static_cast<float>(std::max<LONG>(1, hostRuntime_.width)) / static_cast<float>(std::max<LONG>(1, hostRuntime_.height));
            float tanHalfFov = std::tan(44.0f * kPi / 180.0f);
            float ndcX = menuRuntime_.pointerX * 2.0f - 1.0f;
            float ndcY = 1.0f - menuRuntime_.pointerY * 2.0f;
            XMFLOAT3 ray = Normalize3(Add3(viewForward,
                Add3(Scale3(viewRight, ndcX * aspect * tanHalfFov), Scale3(viewUp, ndcY * tanHalfFov))), viewForward);
            XMFLOAT3 c = gameWorld_.maze.WorldCenter(gameWorld_.maze.start, 0.0f);
            float wallZ = c.z + gameWorld_.maze.tileD * 0.5f - 0.074f;
            if (std::abs(ray.z) > 0.001f) {
                float t = (wallZ - gameWorld_.player.position.z) / ray.z;
                if (t > 0.05f && t < 7.0f) {
                    XMFLOAT3 aimPoint = Add3(gameWorld_.player.position, Scale3(ray, t));
                    targetYaw = YawToPoint(aimPoint) + jitterYaw + hoverFlicker;
                    targetPitch = PitchToPoint(aimPoint) + jitterPitch - std::abs(hoverFlicker) * 0.35f;
                }
            }
        }
