    float ExitAttentionWeight(XMFLOAT3& focus) const {
        focus = exitDoorPresentation_.signLightStrength > 0.001f
            ? exitDoorPresentation_.signLightPosition
            : Add3(exitDoorPresentation_.center, {0.0f, 0.72f, 0.0f});
        if (gameWorld_.exitTransitionActive || gameWorld_.deathActive) return 0.0f;
        Tile cur = CameraTile();
        if (!gameWorld_.maze.IsOpen(cur.x, cur.y) || !gameWorld_.maze.IsOpen(gameWorld_.maze.exit.x, gameWorld_.maze.exit.y)) return 0.0f;
        if (!gameWorld_.maze.LineClear(cur, gameWorld_.maze.exit)) return 0.0f;

        float tile = std::max(gameWorld_.maze.TileAverage(), 0.1f);
        XMFLOAT3 signFocus = focus;
        XMFLOAT3 doorFocus = Add3(exitDoorPresentation_.center, {0.0f, 0.08f, 0.0f});
        float dx = signFocus.x - gameWorld_.player.position.x;
        float dz = signFocus.z - gameWorld_.player.position.z;
        float dist = std::sqrt(dx * dx + dz * dz);
        float maxDist = std::clamp(tile * 6.8f, 7.0f, 12.5f);
        if (dist > maxDist) return 0.0f;

        bool routingToExit = !cameraRuntime_.path.empty() && cameraRuntime_.path.back() == gameWorld_.maze.exit;
        float yawDelta = std::abs(AngleWrap(YawToPoint(signFocus) - gameWorld_.player.yaw));
        float yawWindow = routingToExit || viewRuntime_.exitSpotted ? 2.35f : 1.75f;
        float facing = SmoothStep(yawWindow, 0.22f, yawDelta);
        if (facing <= 0.001f) return 0.0f;

        float close = SmoothStep(maxDist, tile * 1.15f, dist);
        float doorBlend = SmoothStep(tile * 3.0f, tile * 0.70f, dist) * 0.72f;
        focus = Lerp3(signFocus, doorFocus, doorBlend);
        float lock = SmoothStep(tile * 2.7f, tile * 0.70f, dist);
        float routeScale = routingToExit ? 1.0f : (viewRuntime_.exitSpotted ? 0.88f : 0.72f);
        float maxWeight = Lerp(0.48f, 0.82f, lock);
        return std::clamp(close * facing * routeScale * maxWeight, 0.0f, maxWeight);
    }
