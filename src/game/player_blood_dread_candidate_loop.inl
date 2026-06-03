        Tile cameraTile = CameraTile();
        XMFLOAT3 flashlightDir = FlashlightForward();
        for (size_t i = 0; i < scareRuntime_.bloodScarePoints.size(); ++i) {
            BloodScarePoint& point = scareRuntime_.bloodScarePoints[i];
            XMFLOAT3 target = point.source.y > 0.01f ? point.source : point.pos;
            XMFLOAT3 visibilityTarget = target;
            if (point.requireFacing) {
                float inset = std::clamp(RenderMazeView().TileMinimum() * 0.32f, 0.22f, 0.58f);
                visibilityTarget = Add3(target, Scale3(point.normal, inset));
            }
            float dx = target.x - world.playerPosition.x;
            float dy = target.y - world.playerPosition.y;
            float dz = target.z - world.playerPosition.z;
            float floorDx = point.pos.x - world.playerPosition.x;
            float floorDz = point.pos.z - world.playerPosition.z;
            float horizontalDist = std::sqrt(floorDx * floorDx + floorDz * floorDz);
            Tile bloodTile = RenderMazeView().TileFromWorld(visibilityTarget.x, visibilityTarget.z);
            float tileMin = std::max(0.1f, RenderMazeView().TileMinimum());
            float tileAvg = std::max(0.1f, RenderMazeView().TileAverage());
            if (!point.triggered) {
                float revealDistance = point.waterLiquid ? tileAvg * 8.25f : tileAvg * 5.35f;
                float aheadLimit = point.waterLiquid ? -0.18f : -0.06f;
                int revealSteps = point.waterLiquid ? 12 : 8;
                if (!ScareSourceAhead(visibilityTarget,
                    tileMin * 0.36f,
                    revealDistance,
                    revealSteps,
                    aheadLimit)) continue;
            } else if (horizontalDist > tileAvg * (point.waterLiquid ? 8.25f : 5.35f)) {
                continue;
            }
            if (!RenderMazeView().LineClear(cameraTile, bloodTile)) continue;
            if (point.requireFacing) {
                XMFLOAT3 fromSurface{
                    world.playerPosition.x - target.x,
                    0.0f,
                    world.playerPosition.z - target.z
                };
                float facing = Dot3(fromSurface, point.normal);
                if (facing < 0.045f) continue;
                if (!PlayerCollisionSegmentOpenThroughOpen(world.playerPosition.x, world.playerPosition.z, visibilityTarget.x, visibilityTarget.z, false)) continue;
            }
