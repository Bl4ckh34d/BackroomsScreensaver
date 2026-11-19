    PlayerManualControlServices BuildManualControlServices(ManualControlServiceContext& serviceContext) {
        PlayerManualControlServices services{};
        services.context = &serviceContext;
        services.planVault = [](void* context, const XMFLOAT3& start, float yaw, PlayerWindowVault& path) {
            Renderer* self = static_cast<ManualControlServiceContext*>(context)->renderer;
            const Maze& maze = self->RenderMazeView();
            PlayerWindowVault candidate{};
            const float eyeHeight = self->sessionRuntime_.input.crouch ? kPlayerCrouchedEyeHeight : kPlayerStandingEyeHeight;
            if (!PlanWindowVault(maze, start, yaw, self->settingsRuntime_.live.wallHeightMeters, eyeHeight, candidate)) {
                return false;
            }
            const Tile landing = maze.TileFromWorld(candidate.landing.x, candidate.landing.z);
            if (self->MonsterBodyOccupiesTile(landing) ||
                !self->PlayerCollisionSegmentOpenThroughOpen(candidate.exit.x, candidate.exit.z, candidate.landing.x, candidate.landing.z, false) ||
                !self->PlayerCollisionSegmentOpenThroughOpen(start.x, start.z, candidate.entry.x, candidate.entry.z, false)) {
                return false;
            }
            path = candidate;
            return true;
        };
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
