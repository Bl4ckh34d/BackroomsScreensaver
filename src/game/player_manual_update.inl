    void UpdateManualPlayer(float dt) {
        dt = std::clamp(dt, 0.0f, 0.05f);

        PlayerManualControlInput controlInput = BuildManualControlInput(dt);
        ManualControlServiceContext serviceContext{this, dt};
        PlayerManualControlServices services = BuildManualControlServices(serviceContext);

        PlayerManualControlResult controlResult = gameWorld_.UpdatePlayerManualControl(
            controlInput,
            services);
        cameraRuntime_.manualLookYawDelta = controlResult.manualLookYawDelta;
        cameraRuntime_.manualLookPitchDelta = controlResult.manualLookPitchDelta;

        DispatchManualInteraction(controlResult.interactPressed);

        RevealVisibleMapTiles();
    }
