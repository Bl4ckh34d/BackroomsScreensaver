RECT GameMenuExitDoorRect(const RECT& client) {
    if (gApp && gApp->rendererInitialized && gApp->gameState == GameState::MainMenu &&
        gApp->renderer.RuntimeMode() == RendererRuntimeMode::MainMenu) {
        RECT projected{};
        if (gApp->renderer.MenuExitDoorScreenRect(projected)) return projected;
    }
    int h = std::max<LONG>(1, client.bottom - client.top);
    return {client.right - 118, h / 2 - 92, client.right - 56, h / 2 + 118};
}
