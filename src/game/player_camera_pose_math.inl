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

