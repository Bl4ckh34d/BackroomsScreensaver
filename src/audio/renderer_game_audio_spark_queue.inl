    void QueueSparkSoundAt(XMFLOAT3 pos, float intensity = 1.0f) {
        gameWorld_.QueueAudioEvent(GameAudioEvent::OneShot(
            GameSound::ElectricCrackle,
            AudioBus::Effects,
            pos,
            std::clamp(0.40f + intensity * 0.24f, 0.32f, 1.25f),
            true,
            true).WithCategory(GameAudioEventCategory::Lamp));
    }
