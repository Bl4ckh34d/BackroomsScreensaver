        if (moved) {
            gameWorld_.monster.position = pos;
            RecordMonsterTrailPoint(gameWorld_.monster.position);
            float moveYaw = std::atan2(gameWorld_.monster.position.x - start.x, gameWorld_.monster.position.z - start.z);
            if (std::isfinite(moveYaw)) {
                gameWorld_.monster.yaw += AngleWrap(moveYaw - gameWorld_.monster.yaw) * 0.22f;
            }
        }
