    void ApplyDebugSliceCamera() {
        const Maze& maze = RenderMazeView();
        int tiles = std::clamp(gDebugSliceTiles, 1, 5);
        float ox = -static_cast<float>(maze.w) * maze.tileW * 0.5f;
        float oz = -static_cast<float>(maze.h) * maze.tileD * 0.5f;
        float centerX = ox + (1.0f + static_cast<float>(tiles) * 0.5f) * maze.tileW;
        float centerZ = oz + (1.0f + static_cast<float>(tiles) * 0.5f) * maze.tileD;
        float southInsideZ = oz + (static_cast<float>(tiles) + 0.72f) * maze.tileD;
        southInsideZ = std::min(southInsideZ, oz + (static_cast<float>(maze.h) - 1.12f) * maze.tileD);

        XMFLOAT3 position{centerX, 1.42f, southInsideZ};
        XMFLOAT3 target{centerX, settingsRuntime_.live.wallHeightMeters * 0.46f, centerZ - maze.tileD * 0.30f};
        if (gDebugSliceEffect == DebugSliceEffect::FloorWater) {
            position.y = 1.68f;
            target = {centerX, 0.08f, centerZ};
        } else if (gDebugSliceEffect == DebugSliceEffect::CeilingWater ||
                   gDebugSliceEffect == DebugSliceEffect::CeilingLamps ||
                   gDebugSliceEffect == DebugSliceEffect::BrokenLamps) {
            position.y = 1.12f;
            target = {centerX, settingsRuntime_.live.wallHeightMeters - 0.10f, centerZ};
        } else if (gDebugSliceEffect == DebugSliceEffect::WallWater ||
                   gDebugSliceEffect == DebugSliceEffect::AirVents) {
            position.y = 1.36f;
            target = {centerX, settingsRuntime_.live.wallHeightMeters * 0.56f, oz + maze.tileD + 0.020f};
        } else if (gDebugSliceEffect == DebugSliceEffect::Blood) {
            position.y = 1.82f;
            target = {centerX, settingsRuntime_.live.wallHeightMeters * 0.34f, centerZ + maze.tileD * 0.12f};
        } else if (gDebugSliceEffect == DebugSliceEffect::Props) {
            float distanceScale = DebugPropCameraDistanceScale(gDebugPropIndex);
            float requestedZ = centerZ + maze.tileD * distanceScale;
            float safeSouthZ = southInsideZ - maze.tileD * 0.08f;
            position = {centerX, 1.18f, std::min(requestedZ, safeSouthZ)};
            target = {centerX, DebugPropCameraTargetY(gDebugPropIndex), centerZ};
        }

        ApplyDebugCameraLookAt(position, target, -0.62f, 0.62f);
    }
