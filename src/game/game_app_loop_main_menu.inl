        if (app.gameState == GameState::MainMenu) {
            PushGameMenuInteractionToRenderer(hwnd);
            bool rendererMenuScene = app.rendererInitialized &&
                app.renderer.RuntimeMode() == RendererRuntimeMode::MainMenu;
            if (rendererMenuScene) {
                app.renderer.TickFixed(dt);
            }
            HDC dc = GetDC(hwnd);
            if (dc) {
                PaintGameMainMenu(hwnd, dc);
                ReleaseDC(hwnd, dc);
            }
            UpdateGameMenuTransition(hwnd);
            limitGameFrameRate();
        }
