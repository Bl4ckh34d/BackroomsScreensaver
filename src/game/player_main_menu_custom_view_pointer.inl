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
