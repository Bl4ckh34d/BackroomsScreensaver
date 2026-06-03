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
