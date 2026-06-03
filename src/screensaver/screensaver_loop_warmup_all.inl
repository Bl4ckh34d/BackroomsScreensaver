bool WarmupScreensaverOutputs(App& app, HWND hwnd) {
    bool hadWarmup = app.loadingWarmupPending;
    bool warmupStillPending = WarmupScreensaverOutput(app.renderer, hwnd, app.loadingOverlay,
        app.loadingWarmupPending, app.loadingWarmupStart, app.loadingWarmupAttempts);
    for (auto& clone : app.clones) {
        if (!clone || !clone->hwnd) continue;
        hadWarmup = hadWarmup || clone->loadingWarmupPending;
        warmupStillPending = WarmupScreensaverOutput(clone->renderer, clone->hwnd, clone->loadingOverlay,
            clone->loadingWarmupPending, clone->loadingWarmupStart, clone->loadingWarmupAttempts) || warmupStillPending;
    }
    return hadWarmup || warmupStillPending;
}
