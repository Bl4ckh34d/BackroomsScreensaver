    PlayerManualControlInput BuildManualControlInput(float dt) const {
        PlayerManualControlInput controlInput{};
        controlInput.dt = dt;
        controlInput.input = sessionRuntime_.input;
        controlInput.mouseSensitivity = settingsRuntime_.live.mouseSensitivity;
        controlInput.invertMouseY = settingsRuntime_.live.invertMouseY;
        controlInput.infiniteStamina = settingsRuntime_.live.debugInfiniteStamina;
        controlInput.headBobAmount = settingsRuntime_.live.headBobAmount;
        controlInput.movement = CurrentPlayerMovementTuning();
        controlInput.manualLookYawDelta = cameraRuntime_.manualLookYawDelta;
        controlInput.manualLookPitchDelta = cameraRuntime_.manualLookPitchDelta;
        return controlInput;
    }
