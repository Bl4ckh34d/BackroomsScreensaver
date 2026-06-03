RECT GameMenuButtonRect(const RECT& client, int index) {
    int w = std::max<LONG>(1, client.right - client.left);
    int h = std::max<LONG>(1, client.bottom - client.top);
    bool rendererScene = gApp && gApp->rendererInitialized && gApp->gameState == GameState::MainMenu &&
        gApp->renderer.RuntimeMode() == RendererRuntimeMode::MainMenu;
    if (rendererScene) {
        RECT projected{};
        if (gApp->renderer.MenuButtonScreenRect(index, projected)) return projected;
    }
    int buttonW = std::clamp(w * 34 / 100, 260, 420);
    int buttonH = 48;
    int gap = 13;
    int left = std::clamp(w / 2 - buttonW / 2, 34, std::max(34, w - buttonW - 34));
    int top = std::max(178, h / 2 - 82);
    int count = std::max(1, static_cast<int>(ActiveGameMenuButtons().size()));
    top = std::min(top, std::max(118, h - 78 - (buttonH + gap) * count));
    return {left, top + index * (buttonH + gap), left + buttonW, top + index * (buttonH + gap) + buttonH};
}
