        scareRuntime_.visionFlashTimer = std::max(0.0f, scareRuntime_.visionFlashTimer - std::max(0.0f, dt));

        if (gameWorld_.deathActive) {
            UpdateDeath(dt);
            UpdateDreadMeterDisplay(dt);
            UpdateFlashlightAim(dt);
            UpdateAirParticles(dt);
            UpdateAirParticleFocus(dt);
            return;
        }

        PlayerFlashlightInputResult flashlightInput = PlayerController::UpdateFlashlightInput(
            gameWorld_.player,
            sessionRuntime_.IsPlayableGame(),
            sessionRuntime_.input.flashlight);
        if (flashlightInput.toggled) {
            gameWorld_.QueueAudioEvent(GameAudioEvent::OneShotWithPlayerNoise(
                GameSound::FlashlightStutter,
                AudioBus::Effects,
                gameWorld_.player.position,
                0.41f,
                false,
                FlashlightClickHearingRadius(),
                0.62f).WithCategory(GameAudioEventCategory::PlayerInteraction));
        }

        if (sessionRuntime_.IsPlayableGame() && gameWorld_.PlayableScoreScreenActive()) {
            bool anyKeyPressed = sessionRuntime_.input.anyKey && !viewRuntime_.scoreContinuePressedLastFrame;
            viewRuntime_.scoreContinuePressedLastFrame = sessionRuntime_.input.anyKey;
            bool autoplayContinue = sessionRuntime_.UsesAutopilotInput() && viewRuntime_.exitScoreContinueReady;
            if (viewRuntime_.exitScoreContinueReady && (anyKeyPressed || autoplayContinue)) {
                ContinueAfterScoreScreen();
            }
            UpdateFlashlightAim(dt);
            UpdateAirParticles(dt);
            UpdateAirParticleFocus(dt);
            if (!gameWorld_.exitTransitionActive) return;
        }
