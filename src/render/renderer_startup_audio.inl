    const std::wstring& LastInitializeError() const {
        return startupRuntime_.lastInitializeError;
    }

    bool PrepareAudio(const Settings& settings) {
        audioRuntime_.ready = audioRuntime_.engine.Initialize(MakeAudioEngineSettings(settings));
        if (audioRuntime_.ready && !audioRuntime_.samplesLoaded) {
            audioRuntime_.engine.LoadAll(MakeAudioEngineAssets(settings));
            audioRuntime_.samplesLoaded = true;
        }
        if (audioRuntime_.ready) EnsureTitleThemeAudio();
        return audioRuntime_.ready;
    }
