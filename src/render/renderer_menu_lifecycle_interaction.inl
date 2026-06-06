    void SetMenuInteraction(float pointerX, float pointerY, bool buttonHover, bool exitHover, bool singlePlayerHover) {
        if (menuRuntime_.startTransitionActive || menuRuntime_.startTransitionComplete) return;
        menuRuntime_.pointerTargetX = Clamp01(pointerX);
        menuRuntime_.pointerTargetY = Clamp01(pointerY);
        menuRuntime_.buttonHover = buttonHover;
        menuRuntime_.exitHover = exitHover;
        menuRuntime_.singlePlayerHover = buttonHover && singlePlayerHover;
        menuRuntime_.hoverButtonIndex = -1;
        if (buttonHover) {
            if (menuRuntime_.singlePlayerHover) menuRuntime_.hoverButtonIndex = 0;
        }
    }

    void TriggerMainMenuLampBurst() {
        if (!menuRuntime_.darkLayerOneRun) return;
        if (menuRuntime_.lampBurstPlayed) return;
        menuRuntime_.lampBurstPending = true;
    }

    void BeginMainMenuStartTransition() {
        if (sessionRuntime_.mode != RendererRuntimeMode::MainMenu) return;
        menuRuntime_.startTransitionActive = true;
        menuRuntime_.startTransitionComplete = false;
        menuRuntime_.startTransitionFromCustomView = menuRuntime_.customViewActive || menuRuntime_.customViewTarget;
        menuRuntime_.startTransitionTimer = 0.0f;
        menuRuntime_.startTransitionFade = 0.0f;
        GameWorldRenderSnapshot world = gameWorld_.BuildRenderSnapshot();
        menuRuntime_.startCamera = world.playerPosition;
        menuRuntime_.startYaw = world.playerYaw;
        menuRuntime_.startPitch = world.playerPitch;
        menuRuntime_.customViewTarget = false;
        menuRuntime_.singlePlayerHover = true;
        menuRuntime_.buttonHover = false;
        menuRuntime_.hoverButtonIndex = -1;
        if (menuRuntime_.darkLayerOneRun && !menuRuntime_.lampBurstPlayed) menuRuntime_.lampBurstPending = true;
    }

    bool MainMenuStartTransitionComplete() const {
        return menuRuntime_.startTransitionComplete;
    }

    bool MenuButtonScreenRect(int index, RECT& out) const {
        if (menuRuntime_.startTransitionActive || menuRuntime_.startTransitionComplete) return false;
        if (sessionRuntime_.mode != RendererRuntimeMode::MainMenu || index < 0 || index >= menuRuntime_.buttonCount) return false;
        MenuPlaquePlacement plaque = MenuButtonPlacement(index);
        return ProjectMenuQuadToScreen(plaque.center, plaque.right, {0.0f, 1.0f, 0.0f}, plaque.halfW, plaque.halfH, out);
    }

    bool CustomGamePanelScreenRect(RECT& out) const {
        if (sessionRuntime_.mode != RendererRuntimeMode::MainMenu) return false;
        MenuPlaquePlacement panel = MenuCustomPanelPlacement();
        return ProjectMenuQuadToScreen(panel.center, panel.right, {0.0f, 1.0f, 0.0f}, panel.halfW, panel.halfH, out);
    }

    bool CustomGameControlScreenRect(int control, RECT& out) const {
        if (sessionRuntime_.mode != RendererRuntimeMode::MainMenu) return false;
        MenuPlaquePlacement placement{};
        if (!CustomGameControlPlacement(static_cast<CustomGameMenuControl>(control), placement)) return false;
        return ProjectMenuQuadToScreen(placement.center, placement.right, {0.0f, 1.0f, 0.0f}, placement.halfW, placement.halfH, out);
    }

    bool SettingsBoardControlScreenRect(int control, RECT& out) const {
        if (sessionRuntime_.mode != RendererRuntimeMode::MainMenu) return false;
        MenuPlaquePlacement placement{};
        if (!SettingsBoardControlPlacement(static_cast<SettingsBoardControl>(control), placement)) return false;
        return ProjectMenuQuadToScreen(placement.center, placement.right, {0.0f, 1.0f, 0.0f}, placement.halfW, placement.halfH, out);
    }

    bool MenuExitDoorScreenRect(RECT& out) const {
        if (sessionRuntime_.mode != RendererRuntimeMode::MainMenu) return false;
        XMFLOAT3 center = Add3(exitDoorPresentation_.center, Scale3(exitDoorPresentation_.normal, 0.035f));
        return ProjectMenuQuadToScreen(center, exitDoorPresentation_.right, {0.0f, 1.0f, 0.0f}, 0.71f, 1.12f, out);
    }
