    void QueueNeonFlickerStarterClickAt(XMFLOAT3 pos) {
        gameWorld_.QueueAudioEvent(GameAudioEvent::OneShot(
            GameSound::NeonFlickerStarterClick,
            AudioBus::Effects,
            pos,
            0.35f,
            true,
            true).WithCategory(GameAudioEventCategory::Lamp));
    }
