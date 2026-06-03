void StartScreensaverLoadingWarmup(App& app) {
    app.loadingWarmupPending = app.loadingOverlay != nullptr;
    if (app.loadingWarmupPending) {
        SetLoadingOverlayStatus(app.loadingOverlay, L"Warming first frame",
            L"Starting the first GPU frame.", false);
    }
    for (auto& clone : app.clones) {
        if (clone && clone->loadingOverlay) {
            clone->loadingWarmupPending = true;
            SetLoadingOverlayStatus(clone->loadingOverlay, L"Warming first frame",
                L"Starting the first GPU frame.", false);
        }
    }
}
