        UpdateMonsterHeadAnimation(dt, seesPlayer);
        bool passiveRoam = !seesPlayer && !gameWorld_.monster.hasLastKnown && !gameWorld_.monster.hasSound;
        if (passiveRoam && gameWorld_.monster.roamPauseTimer > 0.0f) {
            float turn = (std::sin(timeRuntime_.time * 1.37f + gameWorld_.monster.position.x * 0.21f) * 0.78f +
                std::sin(timeRuntime_.time * 2.21f - gameWorld_.monster.position.z * 0.17f) * 0.34f);
            gameWorld_.monster.yaw += turn * dt * 0.72f;
            return makeOutput(seesPlayer, heardPlayer, ValidMonsterTile(gameWorld_.monster.goal), moved);
        }
        if (MonsterCuriosityActive()) return makeOutput(seesPlayer, heardPlayer, ValidMonsterTile(gameWorld_.monster.goal), moved);
        if (!ValidMonsterTile(gameWorld_.monster.goal)) return makeOutput(seesPlayer, heardPlayer, false, moved);
