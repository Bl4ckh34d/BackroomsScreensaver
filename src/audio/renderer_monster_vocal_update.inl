    void UpdateMonsterVocalAudio(float dt) {
        audioRuntime_.game.monsterSpottedScreamCooldown = std::max(0.0f, audioRuntime_.game.monsterSpottedScreamCooldown - dt);
        if (monsterPreview_.active || !IsPlayableSimulationMode(sessionRuntime_.mode) || !MonsterActiveForCurrentMode()) {
            audioRuntime_.game.monsterAlertAudioActive = false;
            audioRuntime_.game.monsterAlertVocalTimer = 0.0f;
            return;
        }

        bool alert = MonsterAlertAudioActive();
        if (!alert) {
            audioRuntime_.game.monsterAlertAudioActive = false;
            audioRuntime_.game.monsterAlertVocalTimer = 0.0f;
            audioRuntime_.game.nextMonsterGrowlSeconds -= dt;
            if (audioRuntime_.game.nextMonsterGrowlSeconds <= 0.0f) {
                PlayMonsterAlertGroan(RandRange(0.20f, 0.32f));
                audioRuntime_.game.nextMonsterGrowlSeconds = RandRange(12.0f, 36.0f);
            }
            return;
        }

        float distance = MonsterDistance();
        float closePressure = 1.0f - SmoothStep(2.0f, 10.0f, distance);
        bool visibleChase = gameWorld_.monster.chasingVisible || gameWorld_.monster.recognizedForChase;
        float alertVolume = MonsterAlertVocalVolume(visibleChase, closePressure);

        if (!audioRuntime_.game.monsterAlertAudioActive && visibleChase && audioRuntime_.game.monsterSpottedScreamCooldown <= 0.0f) {
            PlayLayeredMonsterSound(GameSound::MonsterSpottedScream, MonsterSoundOrigin(), alertVolume, 0.88f, 1.08f);
            ScheduleLayeredMonsterSound(GameSound::MonsterGrowl, MonsterSoundOrigin(),
                alertVolume * RandRange(0.74f, 0.92f), RandRange(0.55f, 0.95f), 0.82f, 1.10f);
            audioRuntime_.game.monsterSpottedScreamCooldown = RandRange(5.2f, 7.4f);
            audioRuntime_.game.monsterAlertVocalTimer = RandRange(1.0f, 1.8f);
        }

        audioRuntime_.game.monsterAlertAudioActive = true;
        audioRuntime_.game.monsterAlertVocalTimer = std::max(0.0f, audioRuntime_.game.monsterAlertVocalTimer - dt);
        if (audioRuntime_.game.monsterAlertVocalTimer > 0.0f) return;

        if (visibleChase && audioRuntime_.game.monsterSpottedScreamCooldown <= 0.0f && RandRange(0.0f, 1.0f) < 0.34f) {
            PlayLayeredMonsterSound(GameSound::MonsterSpottedScream, MonsterSoundOrigin(), alertVolume * RandRange(0.88f, 1.06f), 0.88f, 1.08f);
            audioRuntime_.game.monsterSpottedScreamCooldown = RandRange(5.2f, 8.0f);
            audioRuntime_.game.monsterAlertVocalTimer = RandRange(1.2f, 2.1f);
            return;
        }

        PlayMonsterAlertGroan(alertVolume);
        audioRuntime_.game.monsterAlertVocalTimer = visibleChase
            ? RandRange(1.05f, 2.35f)
            : RandRange(4.0f, 8.5f);
    }
