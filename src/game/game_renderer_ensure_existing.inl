
    if (!gApp || !gApp->gameShell) return false;
    if (gApp->rendererInitialized) {
        if (mode == RendererRuntimeMode::MainMenu) {
            gApp->renderer.EnterMainMenuScene();
        } else {
