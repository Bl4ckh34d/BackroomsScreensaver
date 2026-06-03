    void ScheduleDelayedAudio(size_t sampleIndex, GameSound sound, AudioBus bus, XMFLOAT3 pos, float volume, float delay,
                              float frequencyRatio = 1.0f, bool spatial = true,
                              AudioToneProfile toneProfile = AudioToneProfile::Normal,
                              GameAudioEventCategory category = GameAudioEventCategory::Generic) {
        audioRuntime_.game.QueueDelayedEvent(sampleIndex, sound, bus, pos, volume, delay, frequencyRatio, spatial, toneProfile, category);
    }

    void UpdateDelayedAudio(float dt) {
        audioRuntime_.game.DrainReadyDelayedEvents(dt, [&](const DelayedAudioEvent& e) {
            audioRuntime_.engine.PlaySample(e.sampleIndex, e.bus, e.pos, e.volume, e.spatial, e.frequencyRatio, e.sound,
                e.spatial ? AudioOcclusionFor(e.pos) : 0.0f, e.toneProfile);
        });
    }
