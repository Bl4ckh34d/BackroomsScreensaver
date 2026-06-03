// Debug and benchmark camera placement helpers. 
// Included inside Renderer's private section from player_camera_movement.inl.

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

    Tile FindBloodStudyTile() const {
        const Maze& maze = RenderMazeView();
        auto hasWallWithOpenBack = [&](Tile t) {
            if (!maze.IsOpen(t.x, t.y)) return false;
            return (!maze.IsOpen(t.x, t.y - 1) && maze.IsOpen(t.x, t.y + 1)) ||
                (!maze.IsOpen(t.x, t.y + 1) && maze.IsOpen(t.x, t.y - 1)) ||
                (!maze.IsOpen(t.x - 1, t.y) && maze.IsOpen(t.x + 1, t.y)) ||
                (!maze.IsOpen(t.x + 1, t.y) && maze.IsOpen(t.x - 1, t.y));
        };
        for (int y = 1; y < maze.h - 1; ++y) {
            for (int x = 1; x < maze.w - 1; ++x) {
                Tile t{x, y};
                if (hasWallWithOpenBack(t)) return t;
            }
        }
        for (int y = 0; y < maze.h; ++y) {
            for (int x = 0; x < maze.w; ++x) {
                if (maze.IsOpen(x, y)) return {x, y};
            }
        }
        return maze.start;
    }

    void ApplyBloodStudyCamera() {
        const Maze& maze = RenderMazeView();
        scareRuntime_.bloodStudyTile = FindBloodStudyTile();
        XMFLOAT3 c = maze.WorldCenter(scareRuntime_.bloodStudyTile, 0.0f);
        int side = 0;
        struct StudySide { int side; Tile wall; Tile back; };
        StudySide sides[] = {
            {0, {scareRuntime_.bloodStudyTile.x, scareRuntime_.bloodStudyTile.y - 1}, {scareRuntime_.bloodStudyTile.x, scareRuntime_.bloodStudyTile.y + 1}},
            {1, {scareRuntime_.bloodStudyTile.x, scareRuntime_.bloodStudyTile.y + 1}, {scareRuntime_.bloodStudyTile.x, scareRuntime_.bloodStudyTile.y - 1}},
            {2, {scareRuntime_.bloodStudyTile.x - 1, scareRuntime_.bloodStudyTile.y}, {scareRuntime_.bloodStudyTile.x + 1, scareRuntime_.bloodStudyTile.y}},
            {3, {scareRuntime_.bloodStudyTile.x + 1, scareRuntime_.bloodStudyTile.y}, {scareRuntime_.bloodStudyTile.x - 1, scareRuntime_.bloodStudyTile.y}}
        };
        for (const StudySide& candidate : sides) {
            if (!maze.IsOpen(candidate.wall.x, candidate.wall.y) && maze.IsOpen(candidate.back.x, candidate.back.y)) {
                side = candidate.side;
                break;
            }
        }
        XMFLOAT3 forward{0.0f, 0.0f, -1.0f};
        float yaw = kPi;
        if (side == 0) {
            yaw = kPi;
            forward = {0.0f, 0.0f, -1.0f};
        } else if (side == 1) {
            yaw = 0.0f;
            forward = {0.0f, 0.0f, 1.0f};
        } else if (side == 2) {
            yaw = -kPi * 0.5f;
            forward = {-1.0f, 0.0f, 0.0f};
        } else {
            yaw = kPi * 0.5f;
            forward = {1.0f, 0.0f, 0.0f};
        }
        float axisLength = (side == 0 || side == 1) ? maze.tileD : maze.tileW;
        XMFLOAT3 position = Add3({c.x, 1.08f, c.z}, Scale3(forward, -axisLength * 0.86f));
        XMFLOAT3 wallTarget = Add3({c.x, settingsRuntime_.live.wallHeightMeters * 0.52f, c.z}, Scale3(forward, axisLength * 0.50f));
        float dx = wallTarget.x - position.x;
        float dy = wallTarget.y - position.y;
        float dz = wallTarget.z - position.z;
        float pitch = std::clamp(std::atan2(dy, std::max(0.001f, std::sqrt(dx * dx + dz * dz))), -0.10f, 0.18f);
        ApplyDebugCameraPose(position, yaw, pitch, false);
    }
