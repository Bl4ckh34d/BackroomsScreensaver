    void ApplyBenchmarkDemoCamera(float seconds) {
        const Maze& maze = RenderMazeView();
        constexpr float kTwoPi = kPi * 2.0f;
        const float cycle = std::fmod(std::max(0.0f, seconds), 48.0f);
        XMFLOAT3 eye{};
        XMFLOAT3 target{};

        auto world = [&](float tx, float ty, float y) {
            return maze.WorldCenter({std::clamp(static_cast<int>(std::round(tx)), 1, maze.w - 2),
                                      std::clamp(static_cast<int>(std::round(ty)), 1, maze.h - 2)}, y);
        };
        auto lerpWorld = [&](float ax, float ay, float bx, float by, float t, float y) {
            XMFLOAT3 a = world(ax, ay, y);
            XMFLOAT3 b = world(bx, by, y);
            return Lerp3(a, b, SmoothStep(0.0f, 1.0f, Clamp01(t)));
        };

        if (cycle < 12.0f) {
            float t = cycle / 12.0f;
            eye = lerpWorld(16.0f, 55.0f, 55.0f, 55.0f, t, 1.46f);
            target = world(42.0f + std::sin(t * kTwoPi) * 8.0f, 34.0f, 1.34f);
        } else if (cycle < 24.0f) {
            float t = (cycle - 12.0f) / 12.0f;
            eye = lerpWorld(56.0f, 55.0f, 58.0f, 22.0f, t, 1.52f);
            target = world(38.0f, 36.0f + std::cos(t * kTwoPi) * 9.0f, 1.24f);
        } else if (cycle < 36.0f) {
            float t = (cycle - 24.0f) / 12.0f;
            eye = lerpWorld(58.0f, 22.0f, 19.0f, 21.0f, t, 1.36f);
            target = world(36.0f, 38.0f, 1.95f);
        } else {
            float t = (cycle - 36.0f) / 12.0f;
            float orbit = t * kTwoPi;
            XMFLOAT3 center = world(37.0f, 37.0f, 1.38f);
            eye = {
                center.x + std::sin(orbit) * maze.TileAverage() * 8.5f,
                1.58f + std::sin(orbit * 2.0f) * 0.12f,
                center.z + std::cos(orbit) * maze.TileAverage() * 7.0f
            };
            target = {center.x, 1.18f, center.z};
        }

        ApplyDebugCameraLookAt(eye, target, -0.48f, 0.42f);
        viewRuntime_.cameraMotionBlur = {};
        gameWorld_.SetPlayerSmoothedMoveSpeed(1.4f);
        gameWorld_.RestorePlayerFullVitals();
    }
