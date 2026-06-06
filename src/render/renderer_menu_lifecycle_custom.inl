    void SetMainMenuCustomGameView(bool open) {
        if (sessionRuntime_.mode != RendererRuntimeMode::MainMenu) return;
        if (open == menuRuntime_.customViewTarget && open == menuRuntime_.customViewActive) return;
        menuRuntime_.customViewTarget = open;
        if (open) {
            menuRuntime_.customViewActive = true;
            menuRuntime_.customViewTimer = 0.0f;
            GameWorldRenderSnapshot world = gameWorld_.BuildRenderSnapshot();
            menuRuntime_.customStartCamera = world.playerPosition;
            menuRuntime_.customStartYaw = world.playerYaw;
            menuRuntime_.customStartPitch = world.playerPitch;
            menuRuntime_.buttonHover = false;
            menuRuntime_.exitHover = false;
            menuRuntime_.singlePlayerHover = false;
            menuRuntime_.hoverButtonIndex = -1;
        } else {
            menuRuntime_.customReturnTimer = 0.0f;
            GameWorldRenderSnapshot world = gameWorld_.BuildRenderSnapshot();
            menuRuntime_.customReturnCamera = world.playerPosition;
            menuRuntime_.customReturnYaw = world.playerYaw;
            menuRuntime_.customReturnPitch = world.playerPitch;
            menuRuntime_.customTextureDirty = true;
        }
    }

    void SetMainMenuSettingsBoardView(bool open) {
        menuRuntime_.settingsBoardMode = open;
        SetMainMenuCustomGameView(open);
        menuRuntime_.customTextureDirty = true;
    }

    bool MainMenuCustomGameViewVisible() const {
        return sessionRuntime_.mode == RendererRuntimeMode::MainMenu && (menuRuntime_.customViewActive || menuRuntime_.customViewTarget);
    }

    void SetCustomGameMenuState(const CustomGameSpec& spec, int hoverControl, int selectedScare = -1) {
        menuRuntime_.settingsBoardMode = false;
        int clampedHover = std::clamp(hoverControl, 0, static_cast<int>(CustomGameMenuControl::Back));
        int clampedSelectedScare = std::clamp(selectedScare, -2, CustomGameSpec::kScareTypeCount - 1);
        bool changed =
            menuRuntime_.customSpec.layer != spec.layer ||
            menuRuntime_.customSpec.mazeWidth != spec.mazeWidth ||
            menuRuntime_.customSpec.mazeHeight != spec.mazeHeight ||
            menuRuntime_.customSpec.roomCount != spec.roomCount ||
            menuRuntime_.customSpec.brokenLampScares != spec.brokenLampScares ||
            menuRuntime_.customSpec.airVentScares != spec.airVentScares ||
            menuRuntime_.customSpec.waterScares != spec.waterScares ||
            menuRuntime_.customSpec.bloodWorldScares != spec.bloodWorldScares ||
            menuRuntime_.customSpec.fleshWorldScares != spec.fleshWorldScares ||
            menuRuntime_.customSpec.omukadeBoss != spec.omukadeBoss ||
            menuRuntime_.customSpec.eightPages != spec.eightPages ||
            menuRuntime_.customSpec.mapDirtPercent != spec.mapDirtPercent ||
            menuRuntime_.customSpec.paperDensityPercent != spec.paperDensityPercent ||
            menuRuntime_.customSpec.propDensityPercent != spec.propDensityPercent ||
            menuRuntime_.customSpec.lampOnPercent != spec.lampOnPercent ||
            menuRuntime_.customSpec.lampFlickerPercent != spec.lampFlickerPercent ||
            menuRuntime_.customSpec.lampSparkPercent != spec.lampSparkPercent ||
            menuRuntime_.customSpec.fogStartMeters != spec.fogStartMeters ||
            menuRuntime_.customSpec.fogEndMeters != spec.fogEndMeters ||
            menuRuntime_.customSpec.fogDarknessPercent != spec.fogDarknessPercent ||
            menuRuntime_.customSpec.jumpscareChancePercent != spec.jumpscareChancePercent ||
            menuRuntime_.customSpec.jumpscareStartMinSeconds != spec.jumpscareStartMinSeconds ||
            menuRuntime_.customSpec.jumpscareStartMaxSeconds != spec.jumpscareStartMaxSeconds ||
            menuRuntime_.customSpec.scareChancePercent != spec.scareChancePercent ||
            menuRuntime_.customSpec.scareStartMinSeconds != spec.scareStartMinSeconds ||
            menuRuntime_.customSpec.scareStartMaxSeconds != spec.scareStartMaxSeconds ||
            menuRuntime_.customHoverControl != clampedHover ||
            menuRuntime_.customSelectedScare != clampedSelectedScare;
        menuRuntime_.customSpec = spec;
        menuRuntime_.customHoverControl = clampedHover;
        menuRuntime_.customSelectedScare = clampedSelectedScare;
        menuRuntime_.customTextureDirty = menuRuntime_.customTextureDirty || changed;
    }

    void SetSettingsBoardState(const Settings& settings, int hoverControl, int tab, int captureAction) {
        int clampedTab = std::clamp(tab, 0, 4);
        int clampedCapture = std::clamp(captureAction, -1, kGameInputActionCount - 1);
        bool changed =
            !menuRuntime_.settingsBoardMode ||
            menuRuntime_.settingsBoardSettings.gameFullscreen != settings.gameFullscreen ||
            menuRuntime_.settingsBoardSettings.gameResolutionWidth != settings.gameResolutionWidth ||
            menuRuntime_.settingsBoardSettings.gameResolutionHeight != settings.gameResolutionHeight ||
            menuRuntime_.settingsBoardSettings.gameFrameRateLimit != settings.gameFrameRateLimit ||
            menuRuntime_.settingsBoardSettings.allowWarpFallback != settings.allowWarpFallback ||
            menuRuntime_.settingsBoardSettings.renderScalePercent != settings.renderScalePercent ||
            menuRuntime_.settingsBoardSettings.fxaaEnabled != settings.fxaaEnabled ||
            menuRuntime_.settingsBoardSettings.antiAliasing != settings.antiAliasing ||
            menuRuntime_.settingsBoardSettings.textureAnisotropy != settings.textureAnisotropy ||
            menuRuntime_.settingsBoardSettings.exposure != settings.exposure ||
            menuRuntime_.settingsBoardSettings.bloomAmount != settings.bloomAmount ||
            menuRuntime_.settingsBoardSettings.motionBlurAmount != settings.motionBlurAmount ||
            menuRuntime_.settingsBoardSettings.airParticleDensity != settings.airParticleDensity ||
            menuRuntime_.settingsBoardSettings.monsterIgnorePlayer != settings.monsterIgnorePlayer ||
            menuRuntime_.settingsBoardSettings.debugInfiniteStamina != settings.debugInfiniteStamina ||
            menuRuntime_.settingsBoardSettings.debugInvincible != settings.debugInvincible ||
            menuRuntime_.settingsBoardSettings.mouseSensitivity != settings.mouseSensitivity ||
            menuRuntime_.settingsBoardSettings.invertMouseY != settings.invertMouseY ||
            menuRuntime_.settingsBoardSettings.gameKeyBindings != settings.gameKeyBindings ||
            menuRuntime_.settingsBoardSettings.audioMuted != settings.audioMuted ||
            menuRuntime_.settingsBoardSettings.audioMasterVolume != settings.audioMasterVolume ||
            menuRuntime_.settingsBoardSettings.audioMusicVolume != settings.audioMusicVolume ||
            menuRuntime_.settingsBoardSettings.audioEffectsVolume != settings.audioEffectsVolume ||
            menuRuntime_.settingsBoardSettings.audioAmbienceVolume != settings.audioAmbienceVolume ||
            menuRuntime_.settingsBoardSettings.audioMonsterVolume != settings.audioMonsterVolume ||
            menuRuntime_.settingsHoverControl != hoverControl ||
            menuRuntime_.settingsBoardTab != clampedTab ||
            menuRuntime_.settingsCaptureAction != clampedCapture;
        menuRuntime_.settingsBoardMode = true;
        menuRuntime_.settingsBoardSettings = settings;
        menuRuntime_.settingsHoverControl = hoverControl;
        menuRuntime_.settingsBoardTab = clampedTab;
        menuRuntime_.settingsCaptureAction = clampedCapture;
        menuRuntime_.customTextureDirty = menuRuntime_.customTextureDirty || changed;
    }
