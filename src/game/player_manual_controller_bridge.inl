    struct ManualControlServiceContext {
        Renderer* renderer = nullptr;
        float dt = 0.0f;
    };

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

    PlayerManualControlServices BuildManualControlServices(ManualControlServiceContext& serviceContext) {
        PlayerManualControlServices services{};
        services.context = &serviceContext;
        services.updatePosture = [](void* context, const PlayerManualPostureRequest& request) {
            ManualControlServiceContext* service = static_cast<ManualControlServiceContext*>(context);
            Renderer* self = service->renderer;
            self->UpdateTunnelCrouchLock(service->dt, request.moveDir, request.wantsMove);
            PlayerManualPostureResult result{};
            result.crouching = self->EffectivePlayerCrouch();
            self->UpdateTunnelCameraLean(service->dt);
            return result;
        };
        services.ensureFootprint = [](void* context) {
            ManualControlServiceContext* service = static_cast<ManualControlServiceContext*>(context);
            Renderer* self = service->renderer;
            GameWorldRenderSnapshot world = self->gameWorld_.BuildRenderSnapshot();
            if (!self->PlayerCollisionFootprintOpen(world.playerPosition.x, world.playerPosition.z)) {
                self->RecoverPlayerCollisionFootprint();
            }
        };
        services.move = [](void* context, const PlayerCollisionMoveRequest& request) {
            ManualControlServiceContext* service = static_cast<ManualControlServiceContext*>(context);
            Renderer* self = service->renderer;
            return self->MovePlayerThroughCollision(
                request.stepX,
                request.stepZ,
                request.distance,
                request.speed,
                self->CameraTile(),
                true);
        };
        return services;
    }

    void DispatchManualInteraction(bool interactPressed) {
        GameWorldRenderSnapshot world = gameWorld_.BuildRenderSnapshot();
        if (interactPressed && !world.exitTransitionActive && TryCollectiblePagePickup()) {
            interactPressed = false;
        }
        if (interactPressed && !world.exitTransitionActive && TrySavePointInteract()) {
            interactPressed = false;
        }
        if (interactPressed && !world.exitTransitionActive && CanTriggerExitTransition()) {
            BeginExitTransition();
        }
    }

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
