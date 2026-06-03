    void ApplyDebugCameraPose(const XMFLOAT3& position, float yaw, float pitch, bool updatePrevious = true) {
        gameWorld_.SetPlayerCameraPose(position, yaw, yaw, pitch);
        viewRuntime_.flashlightYaw = yaw;
        viewRuntime_.flashlightPitch = pitch;
        if (updatePrevious) {
            viewRuntime_.previousCameraYaw = yaw;
            viewRuntime_.previousCameraPitch = pitch;
        }
    }

    void ApplyDebugCameraLookAt(const XMFLOAT3& position, const XMFLOAT3& target, float minPitch, float maxPitch, bool updatePrevious = true) {
        float dx = target.x - position.x;
        float dy = target.y - position.y;
        float dz = target.z - position.z;
        float yaw = std::atan2(dx, dz);
        float pitch = std::clamp(std::atan2(dy, std::max(0.001f, std::sqrt(dx * dx + dz * dz))), minPitch, maxPitch);
        ApplyDebugCameraPose(position, yaw, pitch, updatePrevious);
    }

    void ApplyBloodDebugCamera() {
        if (gEffectDebugViewer) {
            ApplyDebugSliceCamera();
            return;
        }
        const Maze& maze = RenderMazeView();
        Tile t{std::min(3, std::max(1, maze.w - 3)), maze.start.y};
        if (!maze.IsOpen(t.x, t.y)) t = maze.start;
        XMFLOAT3 c = maze.WorldCenter(t, 0.0f);
        ApplyDebugCameraPose({c.x - maze.tileW * 0.34f, 1.36f, c.z}, kPi * 0.5f, -0.045f);
    }
