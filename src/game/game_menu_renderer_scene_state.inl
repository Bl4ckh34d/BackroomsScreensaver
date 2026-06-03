bool GameMenuUsesRendererScene() {
    return gApp && gApp->gameShell && gApp->rendererInitialized &&
        gApp->gameState == GameState::MainMenu &&
        gApp->renderer.RuntimeMode() == RendererRuntimeMode::MainMenu;
}
