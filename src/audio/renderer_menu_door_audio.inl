    void UpdateMenuDoorAudio() {
        if (sessionRuntime_.mode != RendererRuntimeMode::MainMenu) {
            audioRuntime_.game.ResetMenuDoorTracking();
            return;
        }

        if (!audioRuntime_.game.menuDoorAudioPrimed) {
            audioRuntime_.game.PrimeMenuDoorTracking(menuRuntime_.doorOpen);
            return;
        }

        constexpr float kStartThreshold = 0.035f;
        constexpr float kCloseCreakMinOpen = 0.25f;
        constexpr float kLockMinOpen = 0.035f;
        constexpr float kLockThreshold = 0.10f;
        bool opening = menuRuntime_.doorOpen > audioRuntime_.game.previousMenuDoorAudioOpen + 0.0015f;
        bool closing = audioRuntime_.game.previousMenuDoorAudioOpen > menuRuntime_.doorOpen + 0.0015f;
        bool crossedLockThreshold = audioRuntime_.game.previousMenuDoorAudioOpen > kLockThreshold && menuRuntime_.doorOpen <= kLockThreshold;

        if (opening) {
            audioRuntime_.game.menuDoorCloseCreakPlayed = false;
            audioRuntime_.game.menuDoorCloseLockPlayed = false;
            if (!audioRuntime_.game.menuDoorAudioOpen && menuRuntime_.doorOpen >= kStartThreshold) {
                DispatchGameAudioEvent(GameAudioEvent::OneShot(
                    GameSound::DoorOpenCreak,
                    AudioBus::Effects,
                    exitDoorPresentation_.center,
                    0.72f,
                    true,
                    true).WithCategory(GameAudioEventCategory::Door));
                audioRuntime_.game.menuDoorAudioOpen = true;
            }
        }

        if (menuRuntime_.doorOpen > audioRuntime_.game.menuDoorAudioPeakOpen) {
            audioRuntime_.game.menuDoorAudioPeakOpen = menuRuntime_.doorOpen;
        }

        if (closing && audioRuntime_.game.menuDoorAudioPeakOpen >= kCloseCreakMinOpen && !audioRuntime_.game.menuDoorCloseCreakPlayed) {
            DispatchGameAudioEvent(GameAudioEvent::OneShot(
                GameSound::DoorCloseCreak,
                AudioBus::Effects,
                exitDoorPresentation_.center,
                0.66f,
                true,
                true).WithCategory(GameAudioEventCategory::Door));
            audioRuntime_.game.menuDoorCloseCreakPlayed = true;
            audioRuntime_.game.menuDoorAudioOpen = false;
        }

        if (closing && audioRuntime_.game.menuDoorAudioPeakOpen >= kLockMinOpen && crossedLockThreshold && !audioRuntime_.game.menuDoorCloseLockPlayed) {
            DispatchGameAudioEvent(GameAudioEvent::OneShot(
                GameSound::DoorCloseLock,
                AudioBus::Effects,
                exitDoorPresentation_.center,
                1.20f,
                false).WithCategory(GameAudioEventCategory::Door));
            audioRuntime_.game.menuDoorAudioOpen = false;
            audioRuntime_.game.menuDoorCloseCreakPlayed = false;
            audioRuntime_.game.menuDoorCloseLockPlayed = true;
            audioRuntime_.game.menuDoorAudioPeakOpen = 0.0f;
        }

        audioRuntime_.game.previousMenuDoorAudioOpen = menuRuntime_.doorOpen;
    }
