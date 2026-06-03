// Screensaver renderer initialization and loading warmup setup.
// Included from screensaver_app.inl after window helpers.

struct ScreensaverRendererSettings {
    Settings fullscreenSettings{};
    const Settings* rendererSettings = nullptr;
};

ScreensaverRendererSettings BuildScreensaverRendererSettings(RunMode mode) {
    ScreensaverRendererSettings settings{};
    if (mode == RunMode::Fullscreen) {
        settings.fullscreenSettings = LoadSettings();
        settings.fullscreenSettings.mazeSeed = ResolveRuntimeSeed(settings.fullscreenSettings.mazeSeed);
        settings.rendererSettings = &settings.fullscreenSettings;
    }
    return settings;
}
