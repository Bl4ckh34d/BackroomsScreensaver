    void QueueAirVentDustPuffAt(XMFLOAT3 pos, float intensity = 1.0f) {
        gameWorld_.QueueAudioEvent(GameAudioEvent::OneShot(
            GameSound::AirVentDustPuff,
            AudioBus::Effects,
            pos,
            std::clamp(0.48f + intensity * 0.20f, 0.42f, 0.86f),
            true,
            true).WithCategory(GameAudioEventCategory::Vent));
    }
