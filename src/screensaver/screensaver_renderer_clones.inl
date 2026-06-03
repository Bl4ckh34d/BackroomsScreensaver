bool InitializeCloneScreensaverRenderers(
    App& app,
    HWND hwnd,
    const Settings* rendererSettings) {
    for (size_t i = 0; i < app.clones.size(); ++i) {
        auto& clone = app.clones[i];
        if (!clone || !clone->hwnd) continue;
        if (app.loadingOverlay) {
            std::wstringstream detail;
            detail << L"Preparing cloned display " << (i + 2) << L"/" << (app.clones.size() + 1) << L".";
            SetLoadingOverlayStatus(app.loadingOverlay, L"Preparing displays", detail.str().c_str(), false);
        }
        StartupProgressSink cloneProgress{LoadingProgressCallback, clone->loadingOverlay};
        if (!clone->renderer.Initialize(clone->hwnd, rendererSettings, false, MonsterPreviewView::Orbit,
                clone->loadingOverlay ? &cloneProgress : nullptr)) {
            if (clone->loadingOverlay) {
                CloseLoadingOverlayWindow(clone->loadingOverlay);
                clone->loadingOverlay = nullptr;
            }
            std::wstring detail = clone->renderer.LastInitializeError();
            std::wstring message = detail.empty()
                ? L"Direct3D initialization failed on a cloned display."
                : L"Direct3D initialization failed on a cloned display.\r\n\r\n" + detail;
            MessageBoxW(hwnd, message.c_str(), L"Backrooms Maze", MB_OK | MB_ICONERROR);
            QuitScreensaver(hwnd);
            return false;
        }
        clone->renderer.StartScreensaverSession();
    }
    return true;
}
