    void EnsureTitleThemeAudio() {
        if (!audioRuntime_.ready) return;
        if (audioRuntime_.titleThemeStarted) return;
        if (audioRuntime_.engine.HasTaggedVoice(kTitleThemeAudioTag)) return;
        audioRuntime_.engine.PlayTagged(
            GameSound::TitleTheme,
            AudioBus::Music,
            {0.0f, 0.0f, 0.0f},
            1.0f,
            false,
            kTitleThemeAudioTag);
        audioRuntime_.titleThemeStarted = audioRuntime_.engine.HasTaggedVoice(kTitleThemeAudioTag);
    }

    void StopExpiredVisionFlashAudio() {
        if (!audioRuntime_.ready) return;
        if (scareRuntime_.visionFlashTimer > 0.0f) return;
        audioRuntime_.engine.StopTaggedVoice(kVisionFlashAudioTag);
    }
