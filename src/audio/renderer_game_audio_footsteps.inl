    void PlayFootstepSound() {
        if (monsterPreview_.active || sessionRuntime_.mode == RendererRuntimeMode::MainMenu) return;
        constexpr float kFootstepVolumeScale = 1.80f;
        constexpr float kWetFootstepVolumeScale = 0.25125f;
        XMFLOAT3 pos{gameWorld_.player.position.x, 0.08f, gameWorld_.player.position.z};
        bool wetFootstep = IsWetFootstepAtPlayer();
        GameSound sound = wetFootstep ? GameSound::SoakedCarpetStep : GameSound::CarpetStep;
        float walkSpeed = std::max(0.1f, settingsRuntime_.live.walkSpeed);
        float runSpeed = std::max(walkSpeed + 0.1f, settingsRuntime_.live.runSpeed);
        float walkT = Clamp01(gameWorld_.player.smoothedMoveSpeed / walkSpeed);
        float runT = Clamp01((gameWorld_.player.smoothedMoveSpeed - walkSpeed) / std::max(0.1f, runSpeed - walkSpeed));
        float runAudioBlend = std::max(runT, gameWorld_.player.runEffort * 0.72f);
        if (sessionRuntime_.mode == RendererRuntimeMode::PlayableGame && sessionRuntime_.input.crouch) {
            float volume = Lerp(0.32f, 0.68f, walkT);
            volume *= 0.90f * RandRange(0.88f, 1.06f);
            if (wetFootstep) volume *= kWetFootstepVolumeScale;
            volume *= kFootstepVolumeScale;
            DispatchGameAudioEvent(GameAudioEvent::OneShotWithPlayerNoise(
                sound,
                AudioBus::Effects,
                pos,
                volume,
                false,
                FootstepHearingRadius(walkT, runT, true, wetFootstep),
                0.72f).WithCategory(GameAudioEventCategory::Footstep));
            return;
        }
        float volume = Lerp(0.74f, 1.14f, walkT);
        volume = Lerp(volume, 2.08f, runAudioBlend);
        volume *= 0.90f * RandRange(0.92f, 1.08f);
        if (wetFootstep) volume *= kWetFootstepVolumeScale;
        volume *= kFootstepVolumeScale;
        DispatchGameAudioEvent(GameAudioEvent::OneShotWithPlayerNoise(
            sound,
            AudioBus::Effects,
            pos,
            volume,
            false,
            FootstepHearingRadius(walkT, runT, false, wetFootstep),
            0.86f).WithCategory(GameAudioEventCategory::Footstep));
    }
