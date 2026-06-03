    void MainMenuCustomCameraPose(XMFLOAT3& outCamera, float& outYaw, float& outPitch) const {
        MenuPlaquePlacement panel = MenuCustomPanelPlacement();
        XMFLOAT3 target = Add3(panel.center, {0.0f, -0.02f, 0.0f});
        outCamera = Add3(panel.center, Add3(Scale3(panel.inward, 2.00f), {0.0f, 0.08f, 0.06f}));
        outYaw = YawToPoint(target);
        outPitch = std::clamp(PitchToPoint(target), -0.42f, 0.38f);
    }

    bool MainMenuCustomPanelLampTile(Tile lampTile) const {
        if (sessionRuntime_.mode != RendererRuntimeMode::MainMenu) return false;
        GameWorldRenderSnapshot world = gameWorld_.BuildRenderSnapshot();
        if (!world.maze) return false;
        const Maze& maze = *world.maze;
        const int panelX = std::clamp(maze.start.x + 2, 1, maze.w - 2);
        const int panelY = std::max(1, maze.start.y - 1);
        return lampTile.x == panelX && lampTile.y == panelY;
    }

    bool MainMenuExitLampTile(Tile lampTile) const {
        GameWorldRenderSnapshot world = gameWorld_.BuildRenderSnapshot();
        return sessionRuntime_.mode == RendererRuntimeMode::MainMenu && world.maze && lampTile == world.maze->start;
    }

    bool MainMenuPrimaryLampTile(Tile lampTile) const {
        if (sessionRuntime_.mode != RendererRuntimeMode::MainMenu) return false;
        GameWorldRenderSnapshot world = gameWorld_.BuildRenderSnapshot();
        if (!world.maze) return false;
        const Maze& maze = *world.maze;
        const int primaryX = std::clamp(maze.start.x + 1, 1, maze.w - 2);
        return lampTile.x == primaryX && lampTile.y == maze.start.y;
    }

    bool MainMenuAlwaysLitLampTile(Tile lampTile) const {
        if (sessionRuntime_.mode != RendererRuntimeMode::MainMenu) return true;
        GameWorldRenderSnapshot world = gameWorld_.BuildRenderSnapshot();
        if (!world.maze) return false;
        const Maze& maze = *world.maze;
        const int hallX = std::clamp(maze.start.x + 1, 1, maze.w - 2);
        const int hallEndY = std::max(1, maze.start.y - 2);
        return lampTile.x == hallX && lampTile.y >= 1 && lampTile.y <= hallEndY;
    }

    bool MainMenuAllowedLampTile(Tile lampTile) const {
        if (sessionRuntime_.mode != RendererRuntimeMode::MainMenu) return true;
        return MainMenuExitLampTile(lampTile) || MainMenuAlwaysLitLampTile(lampTile) ||
            MainMenuCustomPanelLampTile(lampTile);
    }

    bool MainMenuLampShouldBeLit(Tile lampTile) const {
        if (sessionRuntime_.mode != RendererRuntimeMode::MainMenu) return true;
        if (MainMenuExitLampTile(lampTile)) return !(menuRuntime_.darkLayerOneRun && menuRuntime_.lampBurstPlayed);
        if (MainMenuAlwaysLitLampTile(lampTile)) return true;
        if (MainMenuCustomPanelLampTile(lampTile)) return true;
        if (menuRuntime_.startTransitionActive || menuRuntime_.startTransitionComplete) return false;
        if (MainMenuPrimaryLampTile(lampTile)) return false;
        return false;
    }

    uint8_t MainMenuLampDamageValue(Tile lampTile) const {
        if (!MainMenuLampShouldBeLit(lampTile)) return 255;
        constexpr uint8_t kMenuCustomPanelFlickerDamage = 126;
        if (MainMenuCustomPanelLampTile(lampTile)) return kMenuCustomPanelFlickerDamage;
        return 0;
    }
