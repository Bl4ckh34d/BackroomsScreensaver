        if (app.rendererInitialized &&
            (app.gameState == GameState::PlayGame || app.gameState == GameState::DebugScene)) {
            if (app.gameState == GameState::PlayGame) {
                GameInputSnapshot input = CollectGameInput();
                app.renderer.SetGameInput(input);
            } else {
                app.renderer.SetGameInput({});
            }
            app.renderer.TickFixed(dt);
            if (app.gameState == GameState::DebugScene) RedrawDebugSliceControls();
            limitGameFrameRate();
        } else if (app.gameState != GameState::MainMenu) {
            Sleep(10);
        }
    }
