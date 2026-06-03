    void ApplyBenchmarkDemoCamera(float seconds) {
        const Maze& maze = RenderMazeView();
        struct BenchmarkCameraNode {
            float x;
            float y;
            float height;
        };
        static constexpr BenchmarkCameraNode kNodes[] = {
            {13.0f, 55.0f, 1.48f},
            {13.0f, 42.0f, 1.50f},
            {24.0f, 38.0f, 1.46f},
            {35.0f, 31.0f, 1.52f},
            {43.0f, 38.0f, 1.48f},
            {57.0f, 38.0f, 1.50f},
            {57.0f, 56.0f, 1.47f},
            {65.0f, 58.0f, 1.45f},
            {54.0f, 54.0f, 1.50f},
            {54.0f, 36.0f, 1.48f},
            {56.0f, 23.0f, 1.52f},
            {43.0f, 17.0f, 1.46f},
            {19.0f, 17.0f, 1.50f},
            {16.0f, 30.0f, 1.47f}
        };
        constexpr float kSegmentSeconds = 4.25f;
        constexpr int kNodeCount = static_cast<int>(std::size(kNodes));
        const float cycleSeconds = kSegmentSeconds * static_cast<float>(kNodeCount);

        auto worldAt = [&](const BenchmarkCameraNode& node, float heightOffset) {
            XMFLOAT3 p = maze.WorldCenter({std::clamp(static_cast<int>(std::round(node.x)), 1, maze.w - 2),
                                           std::clamp(static_cast<int>(std::round(node.y)), 1, maze.h - 2)}, node.height + heightOffset);
            return p;
        };
        auto samplePath = [&](float tSeconds, float heightOffset) {
            float wrapped = std::fmod(std::max(0.0f, tSeconds), cycleSeconds);
            int index = static_cast<int>(wrapped / kSegmentSeconds);
            int next = (index + 1) % kNodeCount;
            float localT = SmoothStep(0.0f, 1.0f, (wrapped - static_cast<float>(index) * kSegmentSeconds) / kSegmentSeconds);
            return Lerp3(worldAt(kNodes[index], heightOffset), worldAt(kNodes[next], heightOffset), localT);
        };

        XMFLOAT3 eye = samplePath(seconds, 0.0f);
        XMFLOAT3 target = samplePath(seconds + 2.6f, -0.12f);
        float dx = target.x - eye.x;
        float dz = target.z - eye.z;
        float invLen = 1.0f / std::max(0.001f, std::sqrt(dx * dx + dz * dz));
        float sideLook = std::sin(seconds * 0.42f) * maze.TileAverage() * 0.34f;
        target.x += -dz * invLen * sideLook;
        target.z += dx * invLen * sideLook;
        target.y += std::sin(seconds * 0.31f) * 0.045f;

        ApplyDebugCameraLookAt(eye, target, -0.48f, 0.42f);
        viewRuntime_.cameraMotionBlur = {};
        gameWorld_.SetPlayerSmoothedMoveSpeed(1.25f);
        gameWorld_.RestorePlayerFullVitals();
    }
