        bool visualPlayerLock = canTrackPlayer && MonsterLineOfSightToPlayer();
        float liveLock = canTrackPlayer ? monsterPresentation_.headLockAmount : 0.0f;
        float headLock = std::max(deathHeadLock, liveLock);
        float scanWeight = 1.0f - SmoothStep(0.0f, 1.0f, Clamp01(headLock));
        float headYaw = faceYaw + monsterPresentation_.headYawOffset * 0.24f * scanWeight +
            twitch * 0.32f * (1.0f - deathHeadLock * 0.85f) * scanWeight;
        XMFLOAT3 hRight{std::cos(headYaw), 0.0f, -std::sin(headYaw)};
        XMFLOAT3 hUp = bodyUps[0];
        XMFLOAT3 hForward{std::sin(headYaw), 0.0f, std::cos(headYaw)};
        float uprightBlend = blobSurfaceUp.y < 0.20f ? 0.82f : 0.58f;
        hUp = Normalize3(Lerp3(blobSurfaceUp, {0.0f, 1.0f, 0.0f}, uprightBlend), {0.0f, 1.0f, 0.0f});
        hForward = Normalize3(Sub3(hForward, Scale3(hUp, Dot3(hForward, hUp))), monsterForward);
        if (Length3(hForward) < 0.001f) hForward = Normalize3(Cross3(hUp, monsterRight), monsterForward);
        hRight = Normalize3(Cross3(hUp, hForward), hRight);
        hUp = Normalize3(Cross3(hForward, hRight), hUp);
        auto slerpDirection = [&](XMFLOAT3 from, XMFLOAT3 to, float t) {
            from = Normalize3(from, to);
            to = Normalize3(to, from);
            t = Clamp01(t);
            float dot = std::clamp(Dot3(from, to), -0.9995f, 0.9995f);
            if (dot > 0.9990f) return Normalize3(Lerp3(from, to, t), to);
            float theta = std::acos(dot) * t;
            XMFLOAT3 rel = Normalize3(Sub3(to, Scale3(from, dot)), to);
            return Normalize3(Add3(Scale3(from, std::cos(theta)), Scale3(rel, std::sin(theta))), to);
        };
        auto keepHeadOnSurface = [&](bool projectForward = true) {
            if (projectForward) {
                hForward = Normalize3(Sub3(hForward, Scale3(hUp, Dot3(hForward, hUp))), monsterForward);
            } else {
                hForward = Normalize3(hForward, monsterForward);
            }
            hRight = Normalize3(Cross3(hUp, hForward), hRight);
            hUp = Normalize3(Cross3(hForward, hRight), hUp);
        };
        keepHeadOnSurface();
        float headSurfaceLift = bodyRadii[0] * 0.46f + MonsterHeadBobOffset() * 0.14f * modelY;
        float headForwardLift = bodyRadii[0] * 0.72f + kMonsterHeadForwardOffset * 0.28f * modelXZ;
        XMFLOAT3 headRoot = Add3(bodyPoints[0], OrientedOffset(hRight, hUp, hForward,
            0.0f, bodyRadii[0] * 0.24f, bodyRadii[0] * 0.18f));
        XMFLOAT3 skull = Add3(bodyPoints[0], OrientedOffset(hRight, hUp, hForward,
            0.0f, headSurfaceLift, headForwardLift));
        skull = Add3(skull, Scale3(hUp, curiosityPose * 0.24f * modelY));
        float headAwareness = SmoothStep(0.18f, 1.0f, MonsterAwarenessAmount());
        if (headAwareness > 0.001f) {
            float targetY = std::clamp(playerPosition.y + 0.035f, 1.22f * modelY, settingsRuntime_.live.wallHeightMeters - 0.24f);
            float raise = (targetY - skull.y) * (0.62f * headAwareness);
            skull.y += raise;
            headRoot.y += raise * 0.82f;
        }
        Tile headTile = maze.TileFromWorld(bodyPoints[0].x, bodyPoints[0].z);
        XMFLOAT3 headTileCenter = maze.WorldCenter(headTile, 0.0f);
        float headHalfW = std::max(0.12f, maze.tileW * 0.5f - bodyRadii[0] * 1.35f);
        float headHalfD = std::max(0.12f, maze.tileD * 0.5f - bodyRadii[0] * 1.35f);
        float wallDodgeX = Clamp01(std::abs(bodyPoints[0].x - headTileCenter.x) / headHalfW);
        float wallDodgeZ = Clamp01(std::abs(bodyPoints[0].z - headTileCenter.z) / headHalfD);
        float corridorDodge = std::max(wallDodgeX, wallDodgeZ);
        float dodgeWander = std::sin(timeRuntime_.time * (1.70f + monsterPresentation_.headChaseBlend * 1.60f) + monsterPresentation_.headScanPhase) * 0.022f;
        skull = Add3(skull, Add3(Scale3(hRight, dodgeWander * (1.0f - headLock * 0.55f)),
            Scale3(hUp, corridorDodge * 0.040f * modelY)));
        float headPitch = monsterPresentation_.headPitchOffset * 0.32f * scanWeight;
        if (std::abs(headPitch) > 0.0005f) {
            hForward = Normalize3(Add3(Scale3(hForward, std::cos(headPitch)), Scale3(hUp, std::sin(headPitch))), hForward);
            hUp = Normalize3(Cross3(hForward, hRight), hUp);
        }
        keepHeadOnSurface();
        if (headLock > 0.001f || visualPlayerLock) {
            XMFLOAT3 cameraFocus{playerPosition.x, playerPosition.y + 0.04f, playerPosition.z};
            XMFLOAT3 lookForward = Normalize3(Sub3(cameraFocus, skull), hForward);
            float trackBlend = SmoothStep(0.0f, 1.0f, SmoothStep(0.0f, 1.0f, Clamp01(headLock)));
            float focusBlend = visualPlayerLock
                ? Lerp(0.30f, 0.92f, SmoothStep(0.0f, 1.0f, std::max(headLock, monsterPresentation_.headChaseBlend)))
                : trackBlend * 0.12f;
            hForward = slerpDirection(hForward, lookForward, focusBlend);
            hRight = Normalize3(Cross3(hUp, hForward), hRight);
            hUp = Normalize3(Cross3(hForward, hRight), hUp);
        }
        if (curiosityPose > 0.001f && visualPlayerLock) {
            float tilt = std::sin(timeRuntime_.time * 3.45f) * 0.035f * curiosityPose;
            hRight = Normalize3(Add3(Scale3(hRight, std::cos(tilt)), Scale3(hUp, std::sin(tilt))), hRight);
            hUp = Normalize3(Cross3(hForward, hRight), hUp);
        }
        keepHeadOnSurface(!visualPlayerLock);
        if (canTrackPlayer && headLock > 0.55f && visualPlayerLock) {
            XMFLOAT3 cameraFocus{playerPosition.x, playerPosition.y + 0.04f, playerPosition.z};
            XMFLOAT3 toPlayer = Normalize3(Sub3(cameraFocus, skull), hForward);
            float centered = SmoothStep(std::cos(7.0f * kPi / 180.0f), std::cos(1.2f * kPi / 180.0f), Dot3(hForward, toPlayer));
            float rage = centered * SmoothStep(0.55f, 1.0f, headLock);
            if (rage > 0.001f) {
                float yawJitter = (std::sin(timeRuntime_.time * 24.0f + monsterPosition.x * 3.1f) * 0.005f +
                    std::sin(timeRuntime_.time * 39.0f + monsterPosition.z * 1.7f) * 0.003f) * rage;
                float pitchJitter = (std::sin(timeRuntime_.time * 31.0f + monsterPosition.z * 2.4f) * 0.004f +
                    std::sin(timeRuntime_.time * 47.0f + monsterPosition.x * 1.6f) * 0.002f) * rage;

                hForward = Normalize3(Add3(Scale3(hForward, std::cos(yawJitter)), Scale3(hRight, std::sin(yawJitter))), hForward);
                hRight = Normalize3(Cross3(hUp, hForward), hRight);
                hForward = Normalize3(Add3(Scale3(hForward, std::cos(pitchJitter)), Scale3(hUp, std::sin(pitchJitter))), hForward);
                hUp = Normalize3(Cross3(hForward, hRight), hUp);
                hRight = Normalize3(Cross3(hUp, hForward), hRight);
                hUp = Normalize3(Cross3(hForward, hRight), hUp);
                skull = Add3(skull, Add3(Scale3(hRight, std::sin(timeRuntime_.time * 41.0f) * 0.006f * rage),
                    Scale3(hUp, std::sin(timeRuntime_.time * 53.0f + 1.7f) * 0.005f * rage)));
            }
        }
        keepHeadOnSurface(!visualPlayerLock);
        if (renderAssets_.skullMesh.empty() && !renderAssets_.monsterMeshLoaded) {
            LoadOmukadeMaskMesh();
        }
        bool externalSkull = !renderAssets_.skullMesh.empty();
        bool nativeMaskMesh = externalSkull && renderAssets_.monsterSkullNativeMaskAxes;
        XMFLOAT3 externalMaskCenter = skull;
        if (externalSkull) {
            float frontPulse = 1.0f + std::sin(timeRuntime_.time * 2.18f + monsterPosition.x * 0.11f - monsterPosition.z * 0.07f) * 0.030f;
            XMFLOAT3 maskFlesh = Add3(skull, OrientedOffset(hRight, hUp, hForward,
                0.0f, -0.045f * modelY, -0.070f * modelXZ));
            XMFLOAT3 throatFlesh = Add3(headRoot, OrientedOffset(hRight, hUp, hForward,
                0.0f, 0.070f * modelY, 0.135f * modelXZ));
            XMFLOAT3 jawFlesh = Add3(skull, OrientedOffset(hRight, hUp, hForward,
                0.0f, -0.250f * modelY, -0.035f * modelXZ));
            AppendDynamicEllipsoid(solidVerts, throatFlesh, hRight, hUp, hForward,
                {0.445f * modelXZ * frontPulse, 0.345f * modelY * frontPulse, 0.385f * modelXZ * frontPulse},
                debugEffectMonster ? 16 : 24, debugEffectMonster ? 8 : 12, gutMat + 0.032f);
            AppendDynamicEllipsoid(solidVerts, maskFlesh, hRight, hUp, hForward,
                {0.390f * modelXZ * frontPulse, 0.440f * modelY * frontPulse, 0.235f * modelXZ * frontPulse},
                debugEffectMonster ? 16 : 24, debugEffectMonster ? 8 : 12, gutMat + 0.046f);
            AppendDynamicEllipsoid(solidVerts, jawFlesh, hRight, hUp, hForward,
                {0.295f * modelXZ * frontPulse, 0.175f * modelY * frontPulse, 0.185f * modelXZ * frontPulse},
                debugEffectMonster ? 12 : 18, debugEffectMonster ? 6 : 9, gutMat + 0.058f);
            XMFLOAT3 maskCenter = Add3(skull, Scale3(hForward, 0.030f * modelXZ));
            maskCenter = Add3(maskCenter, Scale3(hUp, -0.035f * modelY));
            if (headAwareness > 0.001f) {
                XMFLOAT3 awareMask = Add3(skull, Scale3(hForward, 0.075f * modelXZ));
                maskCenter = Lerp3(maskCenter, awareMask, headAwareness * 0.54f);
            }
            externalMaskCenter = maskCenter;
            AppendExternalSkullMesh(solidVerts, maskCenter, hRight, hUp, hForward,
                1.10f * modelXZ, 1.10f * modelY);
        } else {
            XMFLOAT3 headMeat = Add3(headRoot, Scale3(hForward, bodyRadii[0] * 0.34f + 0.030f * modelXZ));
            AppendDynamicEllipsoid(solidVerts, headMeat, hRight, hUp, hForward,
                {0.420f * modelXZ, 0.400f * modelY, 0.135f * modelXZ}, 20, 10, gutMat + 0.04f);
        }

        if (false && !externalSkull) {
            auto hornPoint = [&](float x, float y, float z) {
                return Add3(skull, OrientedOffset(hRight, hUp, hForward, x * modelXZ, y * modelY, z * modelXZ));
            };
            for (int side = -1; side <= 1; side += 2) {
                XMFLOAT3 h0 = hornPoint(0.10f * side, 0.11f, -0.035f);
                XMFLOAT3 h1 = hornPoint(0.30f * side, 0.38f, -0.080f);
                XMFLOAT3 h2 = hornPoint(0.56f * side, 0.55f, -0.120f);
                XMFLOAT3 h3 = hornPoint(0.75f * side, 0.61f, -0.080f);
                seg(h0, h1, 0.028f, 0.023f, boneMat);
                seg(h1, h2, 0.024f, 0.020f, boneMat);
                seg(h2, h3, 0.018f, 0.016f, boneMat);
                seg(h1, hornPoint(0.24f * side, 0.58f, 0.040f), 0.016f, 0.014f, boneMat);
                seg(h2, hornPoint(0.54f * side, 0.76f, -0.020f), 0.014f, 0.012f, boneMat);
                seg(h2, hornPoint(0.68f * side, 0.46f, 0.060f), 0.014f, 0.012f, boneMat);
            }
        }
