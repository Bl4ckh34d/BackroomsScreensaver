        if (gEffectDebugViewer && gDebugHideMonster) return;
        if (!MonsterActiveForCurrentMode()) return;
        float modelY = std::clamp(settingsRuntime_.live.monsterScale, 0.35f, 1.25f);
        float modelXZ = std::clamp(settingsRuntime_.live.monsterScale, 0.35f, 1.35f);
        float dist = MonsterDistance();
        bool debugEffectMonster = sessionRuntime_.mode == RendererRuntimeMode::DebugViewer && gDebugSliceEffect != DebugSliceEffect::Props;
        GameWorldRenderSnapshot world = gameWorld_.BuildRenderSnapshot();
        if (!world.maze) return;
        const Maze& maze = *world.maze;
        const XMFLOAT3 playerPosition = world.playerPosition;
        const XMFLOAT3 monsterPosition = world.monsterPosition;
        const float monsterYaw = world.monsterYaw;
        float tileScale = std::max(maze.TileAverage(), 0.1f);
        float bodyLengthMeters = std::min(11.5f, 5.9f + static_cast<float>(std::max(0, world.monsterKillCount)) * 0.72f);
        auto sampleTrail = [&](float targetDistance) {
            if (monsterPresentation_.trail.empty()) return monsterPosition;
            XMFLOAT3 prev = monsterPresentation_.trail.front();
            float travelled = 0.0f;
            for (size_t i = 1; i < monsterPresentation_.trail.size(); ++i) {
                XMFLOAT3 next = monsterPresentation_.trail[i];
                float dx = next.x - prev.x;
                float dz = next.z - prev.z;
                float len = std::sqrt(dx * dx + dz * dz);
                if (travelled + len >= targetDistance && len > 0.001f) {
                    float t = (targetDistance - travelled) / len;
                    return XMFLOAT3{Lerp(prev.x, next.x, t), 0.0f, Lerp(prev.z, next.z, t)};
                }
                travelled += len;
                prev = next;
            }
            float back = std::max(0.0f, targetDistance - travelled);
            return XMFLOAT3{prev.x - std::sin(monsterYaw) * back, 0.0f, prev.z - std::cos(monsterYaw) * back};
        };
        XMFLOAT3 toMonster = Sub3(monsterPosition, playerPosition);
        float planarMonsterDist = std::sqrt(toMonster.x * toMonster.x + toMonster.z * toMonster.z);
        XMFLOAT3 cameraForward = Forward();
        float forwardDot = planarMonsterDist > 0.001f
            ? (toMonster.x * cameraForward.x + toMonster.z * cameraForward.z) / planarMonsterDist
            : 1.0f;
        bool monsterInFront = forwardDot > -0.10f;
        bool canTrackPlayer = false;
        bool specialMonsterView = monsterPreview_.active || debugEffectMonster || world.deathActive;
        bool monsterTileVisible = specialMonsterView;
        bool monsterAnyPartVisible = specialMonsterView;
        bool monsterViewRelevant = specialMonsterView;
        bool monsterOccluded = false;
        if (!specialMonsterView) {
            Tile cameraTile = CameraTile();
            bool cameraTileOpen = maze.IsOpen(cameraTile.x, cameraTile.y);
            XMFLOAT3 viewForward = DirectionFromYawPitch(world.playerYaw, world.playerPitch);
            float renderRadius = std::max(tileScale * 0.58f, modelXZ * 0.85f);
            float maxRenderDistance = std::max(settingsRuntime_.live.fogEndMeters + bodyLengthMeters + tileScale * 2.0f,
                tileScale * 10.0f);
            auto sampleVisible = [&](const XMFLOAT3& sample, float radius) {
                if (!cameraTileOpen) return false;
                XMFLOAT3 toSample = Sub3(sample, playerPosition);
                float padded = maxRenderDistance + radius;
                if (Dot3(toSample, toSample) > padded * padded) return false;
                if (Dot3(toSample, viewForward) <= -radius * 2.0f) return false;
                Tile sampleTile = maze.TileFromWorld(sample.x, sample.z);
                return maze.IsOpen(sampleTile.x, sampleTile.y) && maze.LineClear(cameraTile, sampleTile);
            };

            // Walls already depth-test the pieces; the CPU cull only decides when every sampled body section is hidden.
            monsterTileVisible = sampleVisible(monsterPosition, renderRadius);
            monsterAnyPartVisible = monsterTileVisible;
            float sampleStep = std::max(tileScale * 0.82f, 0.72f);
            int sampleCount = std::clamp(static_cast<int>(std::ceil(bodyLengthMeters / sampleStep)) + 1, 3, 10);
            for (int i = 1; i < sampleCount && !monsterAnyPartVisible; ++i) {
                float t = static_cast<float>(i) / static_cast<float>(sampleCount - 1);
                monsterAnyPartVisible = sampleVisible(sampleTrail(bodyLengthMeters * t), renderRadius);
            }
            if (monsterAnyPartVisible) monsterPresentation_.renderVisibleUntil = timeRuntime_.time + 0.24f;
            monsterViewRelevant = monsterAnyPartVisible || timeRuntime_.time <= monsterPresentation_.renderVisibleUntil;
            monsterOccluded = !monsterAnyPartVisible;
            canTrackPlayer = monsterTileVisible && MonsterVisualEncounterPlayer();
        }
        if (!monsterViewRelevant) {
            if (!specialMonsterView && timeRuntime_.time > monsterPresentation_.renderVisibleUntil + 0.18f) {
                monsterPresentation_.bodySmoothTime = -1000.0f;
                monsterPresentation_.smoothedBodyPoints.clear();
                monsterPresentation_.smoothedBodyUps.clear();
                monsterPresentation_.limbAnchors.clear();
            }
            return;
        }
        bool highDetailMonster = monsterPreview_.active || world.deathActive || canTrackPlayer ||
            monsterPresentation_.headChaseBlend > 0.62f || (monsterAnyPartVisible && dist < tileScale * 4.2f);
        bool mediumDetailMonster = highDetailMonster || (!monsterOccluded && (world.monsterHasLastKnownTarget || world.monsterHasSoundTarget)) ||
            monsterPresentation_.headChaseBlend > 0.18f || (monsterInFront && dist < tileScale * 8.5f) ||
            monsterAnyPartVisible || monsterViewRelevant;
        int monsterDetail = debugEffectMonster ? 1 : (highDetailMonster ? 2 : (mediumDetailMonster ? 1 : 0));
        float faceYaw = monsterYaw;
        if (canTrackPlayer) {
            float cameraYaw = std::atan2(playerPosition.x - monsterPosition.x, playerPosition.z - monsterPosition.z);
            float turnIn = 0.035f + SmoothStep(0.0f, 1.0f, monsterPresentation_.headLockAmount) * 0.18f;
            faceYaw += AngleWrap(cameraYaw - faceYaw) * turnIn;
        }

        XMFLOAT3 right{std::cos(faceYaw), 0.0f, -std::sin(faceYaw)};
        XMFLOAT3 up{0.0f, 1.0f, 0.0f};
        XMFLOAT3 forward{std::sin(faceYaw), 0.0f, std::cos(faceYaw)};
        float hover = 0.30f + std::sin(timeRuntime_.time * 1.25f + monsterPosition.x * 0.07f + monsterPosition.z * 0.05f) * 0.030f;
        auto off = [&](float x, float y, float z) {
            return Add3(monsterPosition, OrientedOffset(right, up, forward, x * modelXZ, y * modelY + hover, z * modelXZ));
        };
        auto box = [&](float x, float y, float z, float hx, float hy, float hz, float material) {
            AppendDynamicBoxAxes(solidVerts, off(x, y, z), right, up, forward,
                {hx * modelXZ, hy * modelY, hz * modelXZ}, material);
        };
        auto seg = [&](XMFLOAT3 a, XMFLOAT3 b, float w, float d, float material) {
            AppendSegmentBox(solidVerts, a, b, w * modelXZ, d * modelXZ, material);
        };

        float twitch = std::sin(timeRuntime_.time * 9.4f + monsterPosition.x * 0.3f) * 0.018f;
        float breathe = std::sin(timeRuntime_.time * 2.3f) * 0.030f;
        float deathHeadLock = world.deathActive ? SmoothStep(0.0f, 0.22f, world.deathTimer) : 0.0f;
        constexpr float boneMat = 9.65f;
        constexpr float gutMat = 20.68f;
        constexpr float limbMat = 20.74f;
        constexpr float darkMat = 10.0f;
        constexpr float handprintMat = 25.35f;
