    void SetupPersistentAudioEmitters() {
        if (audioRuntime_.engine.HasTaggedVoice(kTitleThemeAudioTag)) {
            audioRuntime_.engine.StopAllExceptTag(kTitleThemeAudioTag);
        } else {
            audioRuntime_.engine.StopAll();
        }
        audioRuntime_.game.ResetForScene(
            RandRange(12.0f, 36.0f),
            RandRange(7.0f, 18.0f),
            RandRange(14.0f, 32.0f),
            gameWorld_.player.stepPhase);
        if (!audioRuntime_.ready) return;
        UpdatePersistentLampHumVoices(0.0f, true);
    }
