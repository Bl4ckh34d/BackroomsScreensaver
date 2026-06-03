    void TriggerVisionFlashJumpscare(bool bloodWorld) {
        scareRuntime_.visionFlashDuration = 0.16f;
        scareRuntime_.visionFlashTimer = scareRuntime_.visionFlashDuration;
        viewRuntime_.flashlightAgitation = std::max(viewRuntime_.flashlightAgitation, bloodWorld ? 0.82f : 0.74f);
        gameWorld_.QueueAudioEvent(GameAudioEvent::OneShot(
            GameSound::VisionFlash,
            AudioBus::Effects,
            gameWorld_.player.position,
            bloodWorld ? 1.08f : 0.98f,
            false).WithCategory(GameAudioEventCategory::Scare));
    }
