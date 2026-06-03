    void UpdateVentMonsterGroans(float dt) {
        if (monsterPreview_.active || !IsPlayableSimulationMode(sessionRuntime_.mode) || !MonsterActiveForCurrentMode() || gameWorld_.deathActive || gameWorld_.exitTransitionActive) {
            audioRuntime_.game.ventMonsterGroanTimer = std::min(audioRuntime_.game.ventMonsterGroanTimer, 2.0f);
            audioRuntime_.game.ventMonsterGroanCooldown = std::max(0.0f, audioRuntime_.game.ventMonsterGroanCooldown - dt);
            return;
        }

        float tile = std::max(0.1f, gameWorld_.maze.TileAverage());
        float monsterTiles = MonsterDistance() / tile;
        float aroundFourTiles = 1.0f - Clamp01(std::abs(monsterTiles - 4.0f) / 2.15f);
        bool eligible = aroundFourTiles > 0.0f && !IsThreatVisible() && !ChasePanicActive();
        if (!eligible) {
            audioRuntime_.game.ventMonsterGroanTimer = std::min(audioRuntime_.game.ventMonsterGroanTimer, RandRange(2.2f, 5.4f));
            audioRuntime_.game.ventMonsterGroanCooldown = std::max(0.0f, audioRuntime_.game.ventMonsterGroanCooldown - dt);
            return;
        }

        audioRuntime_.game.ventMonsterGroanTimer = std::max(0.0f, audioRuntime_.game.ventMonsterGroanTimer - dt);
        audioRuntime_.game.ventMonsterGroanCooldown = std::max(0.0f, audioRuntime_.game.ventMonsterGroanCooldown - dt);
        if (audioRuntime_.game.ventMonsterGroanTimer > 0.0f || audioRuntime_.game.ventMonsterGroanCooldown > 0.0f) return;

        float chance = Lerp(0.18f, 0.52f, aroundFourTiles);
        if (RandRange(0.0f, 1.0f) < chance) {
            PlayVentMonsterGroan();
            audioRuntime_.game.ventMonsterGroanCooldown = RandRange(18.0f, 42.0f);
        }
        audioRuntime_.game.ventMonsterGroanTimer = RandRange(7.0f, 18.0f);
    }
