    void QueueLightBulbBreakSoundAt(XMFLOAT3 pos, float intensity = 1.0f, bool emitPlayerNoise = true) {
        bool spatial = sessionRuntime_.mode != RendererRuntimeMode::MainMenu;
        gameWorld_.QueueAudioEvent(GameAudioEvent::OneShotWithPlayerNoise(
            GameSound::LightBulbBreak,
            AudioBus::Effects,
            pos,
            std::clamp(1.80f + intensity * 0.70f, 1.25f, 3.20f),
            spatial,
            emitPlayerNoise ? LightBulbBreakHearingRadius() : 0.0f,
            1.70f,
            false).WithCategory(GameAudioEventCategory::Lamp));
    }
