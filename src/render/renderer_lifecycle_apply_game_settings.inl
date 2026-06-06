    void ApplyGameSettings(const Settings& settings) {
        int oldRenderScalePercent = settingsRuntime_.live.renderScalePercent;
        int oldAntiAliasing = settingsRuntime_.live.antiAliasing;
        int oldTextureAnisotropy = settingsRuntime_.live.textureAnisotropy;
        auto applyLive = [&](Settings& target) {
            target.gameFullscreen = settings.gameFullscreen;
            target.gameResolutionWidth = settings.gameResolutionWidth;
            target.gameResolutionHeight = settings.gameResolutionHeight;
            target.gameFrameRateLimit = settings.gameFrameRateLimit;
            target.allowWarpFallback = settings.allowWarpFallback;
            target.renderScalePercent = settings.renderScalePercent;
            target.antiAliasing = NormalizeAntiAliasingMode(settings.antiAliasing);
            target.fxaaEnabled = AntiAliasingUsesFxaa(target.antiAliasing);
            target.textureAnisotropy = NormalizeTextureAnisotropy(settings.textureAnisotropy);
            target.mapOverlay = settings.mapOverlay;
            target.debugAiMapOverlay = settings.debugAiMapOverlay;
            target.debugInfiniteStamina = settings.debugInfiniteStamina;
            target.debugInvincible = settings.debugInvincible;
            target.monsterIgnorePlayer = settings.monsterIgnorePlayer;
            target.exposure = settings.exposure;
            target.bloomAmount = settings.bloomAmount;
            target.motionBlurAmount = settings.motionBlurAmount;
            target.airParticleDensity = settings.airParticleDensity;
            target.mouseSensitivity = settings.mouseSensitivity;
            target.invertMouseY = settings.invertMouseY;
            target.gameKeyBindings = settings.gameKeyBindings;
            target.audioMuted = settings.audioMuted;
            target.audioMasterVolume = settings.audioMasterVolume;
            target.audioMusicVolume = settings.audioMusicVolume;
            target.audioEffectsVolume = settings.audioEffectsVolume;
            target.audioAmbienceVolume = settings.audioAmbienceVolume;
            target.audioMonsterVolume = settings.audioMonsterVolume;
        };
        applyLive(sessionRuntime_.gameplaySettings);
        applyLive(settingsRuntime_.live);
        if (d3dRuntime_.device && oldTextureAnisotropy != settingsRuntime_.live.textureAnisotropy) {
            CreateStates();
        }
        if (d3dRuntime_.device &&
            (oldRenderScalePercent != settingsRuntime_.live.renderScalePercent ||
             oldAntiAliasing != settingsRuntime_.live.antiAliasing)) {
            Resize(hostRuntime_.width, hostRuntime_.height);
        }
        audioRuntime_.engine.ApplySettings(MakeAudioEngineSettings(settingsRuntime_.live));
    }
