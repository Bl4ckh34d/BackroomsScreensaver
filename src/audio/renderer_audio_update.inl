    void UpdateAudio(float dt) {
        if (!audioRuntime_.ready) {
            DrainGameAudioEvents(false);
            return;
        }
        audioRuntime_.engine.ApplySettings(MakeAudioEngineSettings(settingsRuntime_.live));
        audioRuntime_.engine.SetListener(gameWorld_.player.position, DirectionFromYawPitch(gameWorld_.player.yaw, gameWorld_.player.pitch));
        if (sessionRuntime_.IsMainMenu()) EnsureTitleThemeAudio();
        StopExpiredVisionFlashAudio();
        DrainGameAudioEvents(true);
        UpdatePlayerAudibleSoundPulses(dt);
        UpdateMenuDoorAudio();
        UpdatePersistentLampHumVoices(dt);
        UpdateLampFlickerStarterClicks(dt);
        UpdateDelayedAudio(dt);
        UpdateWetDripAudio(dt);
        UpdateVentMonsterGroans(dt);

        UpdateMonsterVocalAudio(dt);

        bool footstepMoving = IsPlayableSimulationMode(sessionRuntime_.mode) && gameWorld_.player.smoothedMoveSpeed > 0.05f;
        if (audioRuntime_.game.ConsumeFootstepTrigger(footstepMoving, gameWorld_.player.stepPhase, kPi)) PlayFootstepSound();

        if (gameWorld_.exitTransitionActive) {
            if (!audioRuntime_.game.exitDoorOpenSoundPlayed && exitDoorPresentation_.angle > 0.02f) {
                DispatchGameAudioEvent(GameAudioEvent::OneShot(
                    GameSound::DoorOpenCreak,
                    AudioBus::Effects,
                    exitDoorPresentation_.center,
                    0.90f,
                    true,
                    true).WithCategory(GameAudioEventCategory::Door));
                audioRuntime_.game.exitDoorOpenSoundPlayed = true;
            }
            if (!audioRuntime_.game.exitDoorCloseCreakSoundPlayed && gameWorld_.exitTransitionTimer > settingsRuntime_.live.exitDoorOpenSeconds + settingsRuntime_.live.exitStepSeconds * 0.15f) {
                DispatchGameAudioEvent(GameAudioEvent::OneShot(
                    GameSound::DoorCloseCreak,
                    AudioBus::Effects,
                    exitDoorPresentation_.center,
                    0.76f,
                    true,
                    true).WithCategory(GameAudioEventCategory::Door));
                audioRuntime_.game.exitDoorCloseCreakSoundPlayed = true;
            }
            if (!audioRuntime_.game.exitDoorCloseSoundPlayed && gameWorld_.exitTransitionTimer > settingsRuntime_.live.exitDoorOpenSeconds + settingsRuntime_.live.exitStepSeconds * 0.55f) {
                DispatchGameAudioEvent(GameAudioEvent::OneShot(
                    GameSound::DoorCloseLock,
                    AudioBus::Effects,
                    exitDoorPresentation_.center,
                    0.78f,
                    true,
                    true).WithCategory(GameAudioEventCategory::Door));
                audioRuntime_.game.exitDoorCloseSoundPlayed = true;
            }
        }

        audioRuntime_.engine.Update(dt, [&](XMFLOAT3 pos) {
            return AudioOcclusionFor(pos);
        });
    }
