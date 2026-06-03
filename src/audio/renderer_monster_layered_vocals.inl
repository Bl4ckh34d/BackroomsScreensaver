    bool MonsterAlertAudioActive() const {
        return gameWorld_.monster.AlertAudioActive();
    }

    void PlayLayeredMonsterSound(GameSound sound, XMFLOAT3 pos, float volume, float pitchMin = 0.86f, float pitchMax = 1.10f) {
        size_t first = audioRuntime_.engine.PickRandomSample(sound);
        if (first == static_cast<size_t>(-1)) return;
        size_t second = audioRuntime_.engine.PickRandomSampleExcept(sound, first);
        float firstPitch = RandRange(pitchMin, pitchMax);
        float initialOcclusion = AudioOcclusionFor(pos);
        audioRuntime_.engine.PlaySample(first, AudioBus::Monster, pos, volume * RandRange(0.84f, 1.08f), true, firstPitch, sound,
            initialOcclusion);
        if (second != static_cast<size_t>(-1)) {
            audioRuntime_.engine.PlaySample(second, AudioBus::Monster, pos, volume * RandRange(0.52f, 0.78f), true,
                std::clamp(firstPitch * RandRange(0.82f, 1.18f), 0.50f, 2.0f), sound, initialOcclusion);
        }
    }

    void ScheduleLayeredMonsterSound(GameSound sound, XMFLOAT3 pos, float volume, float delay, float pitchMin = 0.86f, float pitchMax = 1.10f) {
        size_t first = audioRuntime_.engine.PickRandomSample(sound);
        if (first == static_cast<size_t>(-1)) return;
        size_t second = audioRuntime_.engine.PickRandomSampleExcept(sound, first);
        float firstPitch = RandRange(pitchMin, pitchMax);
        ScheduleDelayedAudio(first, sound, AudioBus::Monster, pos, volume * RandRange(0.84f, 1.08f), delay, firstPitch,
            true, AudioToneProfile::Normal, GameAudioEventCategory::MonsterVocal);
        if (second != static_cast<size_t>(-1)) {
            ScheduleDelayedAudio(second, sound, AudioBus::Monster, pos, volume * RandRange(0.52f, 0.78f),
                delay + RandRange(0.015f, 0.055f), std::clamp(firstPitch * RandRange(0.82f, 1.18f), 0.50f, 2.0f),
                true, AudioToneProfile::Normal, GameAudioEventCategory::MonsterVocal);
        }
    }

    void PlayMonsterAlertGroan(float volume = 0.82f) {
        PlayLayeredMonsterSound(GameSound::MonsterGrowl, MonsterSoundOrigin(),
            std::clamp(volume * RandRange(0.86f, 1.12f), 0.18f, 1.25f), 0.82f, 1.12f);
    }

    float MonsterAlertVocalVolume(bool visibleChase, float closePressure) const {
        if (visibleChase) return Lerp(0.92f, 1.18f, closePressure);
        if (gameWorld_.monster.hasLastKnown || gameWorld_.monster.hasSound || gameWorld_.monster.chaseMemoryTimer > 0.0f) return Lerp(0.52f, 0.78f, closePressure);
        return Lerp(0.28f, 0.44f, closePressure);
    }
