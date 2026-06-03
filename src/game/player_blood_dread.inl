    void UpdateBloodDread(float dt) {
        scareRuntime_.bloodFocusReactionCooldown = std::max(0.0f, scareRuntime_.bloodFocusReactionCooldown - dt);
        GameWorldRenderSnapshot world = gameWorld_.BuildRenderSnapshot();
        if (world.deathActive || world.exitTransitionActive || scareRuntime_.bloodScarePoints.empty()) return;
        if (IsThreatVisible() || ChasePanicActive()) {
            scareRuntime_.bloodFocusTimer = 0.0f;
            scareRuntime_.activeBloodScareIndex = -1;
            return;
        }
        if (sessionRuntime_.mode == RendererRuntimeMode::PlayableGame && sessionRuntime_.input.crouch) {
            scareRuntime_.bloodFocusTimer = 0.0f;
            scareRuntime_.activeBloodScareIndex = -1;
            return;
        }
        Tile cameraTile = CameraTile();
        XMFLOAT3 flashlightDir = FlashlightForward();
        for (size_t i = 0; i < scareRuntime_.bloodScarePoints.size(); ++i) {
            BloodScarePoint& point = scareRuntime_.bloodScarePoints[i];
            XMFLOAT3 target = point.source.y > 0.01f ? point.source : point.pos;
            XMFLOAT3 visibilityTarget = target;
            if (point.requireFacing) {
                float inset = std::clamp(RenderMazeView().TileMinimum() * 0.32f, 0.22f, 0.58f);
                visibilityTarget = Add3(target, Scale3(point.normal, inset));
            }
            float dx = target.x - world.playerPosition.x;
            float dy = target.y - world.playerPosition.y;
            float dz = target.z - world.playerPosition.z;
            float floorDx = point.pos.x - world.playerPosition.x;
            float floorDz = point.pos.z - world.playerPosition.z;
            float horizontalDist = std::sqrt(floorDx * floorDx + floorDz * floorDz);
            Tile bloodTile = RenderMazeView().TileFromWorld(visibilityTarget.x, visibilityTarget.z);
            float tileMin = std::max(0.1f, RenderMazeView().TileMinimum());
            float tileAvg = std::max(0.1f, RenderMazeView().TileAverage());
            if (!point.triggered) {
                float revealDistance = point.waterLiquid ? tileAvg * 8.25f : tileAvg * 5.35f;
                float aheadLimit = point.waterLiquid ? -0.18f : -0.06f;
                int revealSteps = point.waterLiquid ? 12 : 8;
                if (!ScareSourceAhead(visibilityTarget,
                    tileMin * 0.36f,
                    revealDistance,
                    revealSteps,
                    aheadLimit)) continue;
            } else if (horizontalDist > tileAvg * (point.waterLiquid ? 8.25f : 5.35f)) {
                continue;
            }
            if (!RenderMazeView().LineClear(cameraTile, bloodTile)) continue;
            if (point.requireFacing) {
                XMFLOAT3 fromSurface{
                    world.playerPosition.x - target.x,
                    0.0f,
                    world.playerPosition.z - target.z
                };
                float facing = Dot3(fromSurface, point.normal);
                if (facing < 0.045f) continue;
                if (!PlayerCollisionSegmentOpenThroughOpen(world.playerPosition.x, world.playerPosition.z, visibilityTarget.x, visibilityTarget.z, false)) continue;
            }

            float dist = std::sqrt(dx * dx + dy * dy + dz * dz);
            float aim = dist > 0.001f ? (dx * flashlightDir.x + dy * flashlightDir.y + dz * flashlightDir.z) / dist : 1.0f;
            if (point.triggered && aim < -0.18f) continue;

            if (!point.triggered) {
                point.triggered = true;
                if (point.waterLiquid) {
                    point.activationTime = timeRuntime_.time;
                    point.focusTaken = true;
                } else {
                    point.activationTime = timeRuntime_.time;
                }
                if (point.revealBlood) {
                    IncludeBloodReveal(point);
                }
                scareRuntime_.bloodScareActiveUntil = timeRuntime_.time + 150.0f;
                continue;
            }
            if (point.waterLiquid) continue;
            if (horizontalDist > tileAvg * 3.45f) continue;
            float visibleAge = timeRuntime_.time - point.activationTime;
            float minVisibleAge = point.requireFacing
                ? 1.10f
                : (target.y > settingsRuntime_.live.wallHeightMeters * 0.55f ? 0.92f : 0.76f);
            if (point.focusTaken || visibleAge < std::max(point.focusDelaySeconds, minVisibleAge)) continue;
            bool focusAllowed = scareRuntime_.bloodFocusReactionsTaken == 0 ||
                (scareRuntime_.bloodFocusReactionsTaken == 1 && scareRuntime_.bloodFocusReactionCooldown <= 0.0f);
            if (!focusAllowed) continue;

            scareRuntime_.activeBloodScareIndex = static_cast<int>(i);
            scareRuntime_.bloodScareActiveUntil = std::max(scareRuntime_.bloodScareActiveUntil, timeRuntime_.time + 150.0f);
            point.focusTaken = true;
            ++scareRuntime_.bloodFocusReactionsTaken;
            scareRuntime_.bloodFocusReactionCooldown = scareRuntime_.bloodFocusReactionsTaken == 1
                ? RandRange(5.0f, 15.0f)
                : 1000000.0f;
            scareRuntime_.bloodFocusDuration = RandRange(1.45f, 2.25f);
            scareRuntime_.bloodFocusTimer = scareRuntime_.bloodFocusDuration;
            scareRuntime_.bloodFocusTarget = {
                target.x,
                std::clamp(target.y, 0.16f, settingsRuntime_.live.wallHeightMeters - 0.05f),
                target.z
            };
            cameraRuntime_.stopTimer = 0.0f;
            cameraRuntime_.headScanTimer = 0.0f;
            cameraRuntime_.lookBack = false;
            cameraRuntime_.junctionScanActive = false;
            viewRuntime_.propLookTimer = 0.0f;
            float closeness = Clamp01((point.radius - horizontalDist) / std::max(0.001f, point.radius));
            float gaze = SmoothStep(0.42f, 0.88f, aim);
            float reactionScale = std::clamp(point.dreadScale, 0.20f, 1.35f);
            float spike = std::max(settingsRuntime_.live.dreadJumpscareGain * 1.15f, 0.42f + closeness * 0.24f + gaze * 0.24f) * reactionScale;
            AddDread(spike);
            gameWorld_.QueueAudioEvent(GameAudioEvent::PlayerNoise(
                {world.playerPosition.x, 0.08f, world.playerPosition.z},
                JumpscareHearingRadius(0.92f),
                1.10f,
                GameAudioEventCategory::Scare));
            viewRuntime_.flashlightAgitation = std::max(viewRuntime_.flashlightAgitation, (0.90f + closeness * 0.20f + gaze * 0.18f) * Lerp(0.62f, 1.0f, reactionScale));
            viewRuntime_.flashlightSnapCooldown = std::min(viewRuntime_.flashlightSnapCooldown, 0.08f);
            viewRuntime_.stumbleTimer = std::max(viewRuntime_.stumbleTimer, (0.10f + closeness * 0.08f) * reactionScale);
            viewRuntime_.stumbleDuration = std::max(viewRuntime_.stumbleDuration, 0.20f * reactionScale);
            viewRuntime_.secondsSinceLookBack = 0.0f;
        }
    }
