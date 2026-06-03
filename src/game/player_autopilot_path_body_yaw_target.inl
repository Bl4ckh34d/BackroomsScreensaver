        float bodyTargetYaw = dist > 0.001f ? std::atan2(dx, dz) : gameWorld_.player.bodyYaw;
        if (!freeRunMove && AdjacentTiles(movementTile, targetTileForMove)) {
            XMFLOAT3 currentCenter = gameWorld_.maze.WorldCenter(movementTile, gameWorld_.player.position.y);
            float alignTolerance = std::clamp(gameWorld_.maze.TileMinimum() * 0.055f, 0.045f, 0.11f);
            if (targetTileForMove.x != movementTile.x && std::abs(gameWorld_.player.position.z - currentCenter.z) > alignTolerance) {
                target = {gameWorld_.player.position.x, gameWorld_.player.position.y, currentCenter.z};
                dx = target.x - gameWorld_.player.position.x;
                dz = target.z - gameWorld_.player.position.z;
                dist = std::sqrt(dx * dx + dz * dz);
            } else if (targetTileForMove.y != movementTile.y && std::abs(gameWorld_.player.position.x - currentCenter.x) > alignTolerance) {
                target = {currentCenter.x, gameWorld_.player.position.y, gameWorld_.player.position.z};
                dx = target.x - gameWorld_.player.position.x;
                dz = target.z - gameWorld_.player.position.z;
                dist = std::sqrt(dx * dx + dz * dz);
            }
        }
