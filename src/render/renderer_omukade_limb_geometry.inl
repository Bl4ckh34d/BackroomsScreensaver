        for (int pairIndex = 0; pairIndex < limbPairs; ++pairIndex) {
            float pairT = static_cast<float>(pairIndex) / std::max(1.0f, static_cast<float>(limbPairs - 1));
            float frontness = 1.0f - pairT;
            int bodyIndex = std::clamp(static_cast<int>(std::round(pairT * static_cast<float>(bodyCount - 1) * 0.78f)),
                0, bodyCount - 1);
            XMFLOAT3 localUp = bodyUps[static_cast<size_t>(bodyIndex)];
            XMFLOAT3 tangent = bodyTangents[static_cast<size_t>(bodyIndex)];
            XMFLOAT3 sideAxis = bodySides[static_cast<size_t>(bodyIndex)];
            XMFLOAT3 p = Add3(bodyPoints[static_cast<size_t>(bodyIndex)],
                Scale3(localUp, std::sin(timeRuntime_.time * 1.7f + pairIndex) * 0.012f));
            for (int sideIndex = -1; sideIndex <= 1; sideIndex += 2) {
                int limbId = pairIndex * 2 + (sideIndex > 0 ? 1 : 0);
                float limbSeed = Rand01(limbId * 29 + 1, 823, sessionRuntime_.runtimeSeed);
                float limbPhase = limbSeed * kPi * 2.0f;
                float roamScramble = SmoothStep(0.0f, 1.0f, Clamp01(world.monsterRoamBurstTimer / 1.05f)) * (1.0f - monsterPresentation_.headChaseBlend * 0.65f);
                float chaseScramble = SmoothStep(0.18f, 1.0f, monsterPresentation_.headChaseBlend);
                float phase = timeRuntime_.time * (1.20f + limbSeed * 0.92f + monsterPresentation_.headChaseBlend * (1.45f + limbSeed * 1.35f) +
                    roamScramble * (2.8f + limbSeed * 2.6f) + chaseScramble * (2.2f + limbSeed * 1.8f)) +
                    static_cast<float>(pairIndex) * (0.27f + limbSeed * 0.31f) + static_cast<float>(sideIndex) * (1.10f + limbSeed) + limbPhase;
                float sideScale = static_cast<float>(sideIndex);
                XMFLOAT3 sideDir = Scale3(sideAxis, sideScale);
                float r = bodyRadii[static_cast<size_t>(bodyIndex)] * Lerp(1.08f, 0.92f, pairT);
                float lateralRoot = r * Lerp(1.22f, 1.02f, pairT) * (0.96f + limbSeed * 0.10f);
                float axialRoot = (limbSeed - 0.5f) * bodySpacing * 0.12f;
                XMFLOAT3 root = Add3(p, Add3(Scale3(sideDir, lateralRoot),
                    Add3(Scale3(tangent, axialRoot), Scale3(localUp, r * 0.06f + std::sin(phase) * (0.008f + limbSeed * 0.014f)))));
                float limb = Lerp(0.028f, 0.043f, pairT) * (0.94f + limbSeed * 0.18f);
                float upperLen = std::max(0.14f, maze.TileMinimum() * (Lerp(0.44f, 0.26f, pairT) + limbSeed * 0.035f) + limb * 0.70f);
                float lowerLen = std::max(0.14f, maze.TileMinimum() * (Lerp(0.50f, 0.30f, pairT) + Rand01(limbId * 31 + 9, 829, sessionRuntime_.runtimeSeed) * 0.040f) + limb * 0.66f);
                LimbReachTarget reachTarget = limbReachTarget(root, sideDir, tangent, localUp, limbId, upperLen, lowerLen);
                float swing = reachTarget.swing;
                XMFLOAT3 target = reachTarget.target;
                XMFLOAT3 contactNormal = Normalize3(reachTarget.normal, up);
                bool frontGrabArm = pairIndex < 2;
                XMFLOAT3 playerAim{playerPosition.x, playerPosition.y - 0.05f + static_cast<float>(pairIndex) * 0.13f, playerPosition.z};
                XMFLOAT3 rootToPlayer = Sub3(playerAim, root);
                float rootPlayerDist = Length3(rootToPlayer);
                float closeGrabRange = (upperLen + lowerLen) * (1.34f + frontness * 0.18f);
                bool playerReachable = IsPlayableSimulationMode(sessionRuntime_.mode) &&
                    !settingsRuntime_.live.debugInvincible &&
                    !MonsterIgnoresPlayer() &&
                    !world.deathActive &&
                    (maze.LineClear(CameraTile(), MonsterTile()) || rootPlayerDist < maze.TileAverage() * 1.08f);
                float closeGrabWeight = frontGrabArm && playerReachable
                    ? (1.0f - SmoothStep(closeGrabRange * 0.62f, closeGrabRange, rootPlayerDist))
                    : 0.0f;
                bool chaseReachArm = canTrackPlayer && monsterPresentation_.headChaseBlend > 0.34f && frontGrabArm;
                if (chaseReachArm || closeGrabWeight > 0.0f) {
                    float reachPulse = 0.76f + std::pow(std::max(0.0f, std::sin(timeRuntime_.time * (2.4f + limbSeed * 1.2f) + limbId * 1.9f)), 4.0f) * 0.32f;
                    float chaseReachWeight = SmoothStep(0.34f, 0.92f, monsterPresentation_.headChaseBlend) * Lerp(1.0f, 0.42f, pairT) * reachPulse;
                    float reachWeight = std::max(chaseReachWeight, closeGrabWeight * (1.12f + reachPulse * 0.16f));
                    XMFLOAT3 toPlayer = Sub3(playerAim, root);
                    float playerDist = Length3(toPlayer);
                    XMFLOAT3 playerDir = Normalize3(toPlayer, forward);
                    float reachLen = std::min(playerDist, (upperLen + lowerLen) * (1.16f + 0.08f * std::sin(timeRuntime_.time * 5.2f + limbId)));
                    XMFLOAT3 reachPoint = Add3(root, Scale3(playerDir, reachLen));
                    float clawJitter = 1.0f - closeGrabWeight * 0.62f;
                    reachPoint = Add3(reachPoint, Add3(Scale3(sideDir, std::sin(timeRuntime_.time * 5.2f + limbId * 1.7f) * 0.034f * clawJitter),
                        Scale3(localUp, std::sin(timeRuntime_.time * 6.8f + limbId * 0.9f) * 0.030f * clawJitter)));
                    target = Lerp3(target, reachPoint, Clamp01(reachWeight));
                    contactNormal = Scale3(playerDir, -1.0f);
                    swing = std::max(swing, 0.42f + Clamp01(reachWeight) * 0.54f);
                }
                XMFLOAT3 toTarget = Sub3(target, root);
                float reach = Length3(toTarget);
                XMFLOAT3 reachDir = Normalize3(toTarget, sideDir);
                float clampedReach = std::clamp(reach, 0.05f, upperLen + lowerLen - 0.025f);
                float along = (upperLen * upperLen - lowerLen * lowerLen + clampedReach * clampedReach) / std::max(0.001f, 2.0f * clampedReach);
                float bendHeight = std::sqrt(std::max(0.0f, upperLen * upperLen - along * along));
                XMFLOAT3 bendAxis = Normalize3(Add3(Scale3(up, 0.82f), Add3(Scale3(sideDir, 0.34f), Scale3(tangent, std::sin(phase) * 0.30f))), up);
                if (target.y < 0.08f) {
                    bendAxis = Normalize3(Add3(Scale3(sideDir, 0.72f), Add3(Scale3(up, 0.42f), Scale3(tangent, std::sin(phase) * 0.22f))), sideDir);
                } else if (target.y > settingsRuntime_.live.wallHeightMeters - 0.12f) {
                    bendAxis = Normalize3(Add3(Scale3(sideDir, 0.65f), Add3(Scale3(up, -0.48f), Scale3(tangent, std::sin(phase) * 0.22f))), sideDir);
                }
                XMFLOAT3 knee = Add3(root, Add3(Scale3(reachDir, along), Scale3(bendAxis, bendHeight * (0.58f + swing * 0.24f))));
                knee = Add3(knee, Add3(Scale3(tangent, std::sin(phase * 0.67f) * 0.045f), Scale3(localUp, swing * 0.045f)));
                float limbMaterial = limbMat + std::fmod(static_cast<float>(limbId) * 0.007f, 0.10f);
                organicSegment(root, knee, limb * 2.55f, limb * 1.16f, limbMaterial, 8);
                organicSegment(knee, target, limb * 1.28f, limb * 0.58f, limbMaterial, 7);
                AppendDynamicEllipsoid(solidVerts, root, sideAxis, localUp, tangent,
                    {limb * 2.95f, limb * 1.95f, limb * 2.85f}, 9, 5, limbMaterial + 0.025f);
                AppendDynamicEllipsoid(solidVerts, knee, sideAxis, localUp, tangent,
                    {limb * 1.58f, limb * 1.24f, limb * 1.58f}, 8, 5, limbMaterial + 0.030f);
                XMFLOAT3 palmRight{};
                XMFLOAT3 palmUp{};
                surfaceAxes(contactNormal, palmRight, palmUp);
                XMFLOAT3 palmCenter = Add3(target, Scale3(contactNormal, 0.022f + swing * 0.018f));
                float handScale = 0.86f + SmoothStep(0.72f, 1.0f, frontness) * 0.36f;
                float grabHandScale = handScale * (1.0f + closeGrabWeight * 0.10f);
                AppendDynamicEllipsoid(solidVerts, palmCenter, palmRight, palmUp, contactNormal,
                    {limb * 2.25f * grabHandScale, limb * 1.06f * grabHandScale, limb * 0.42f * grabHandScale}, 10, 5, limbMaterial + 0.045f);
                XMFLOAT3 padCenter = Add3(palmCenter, Scale3(contactNormal, 0.018f + swing * 0.010f));
                AppendDynamicEllipsoid(solidVerts, padCenter, palmRight, palmUp, contactNormal,
                    {limb * 1.52f * grabHandScale, limb * 0.62f * grabHandScale, limb * 0.050f * grabHandScale}, 8, 4, darkMat);
                if (closeGrabWeight > 0.72f && playerReachable) {
                    XMFLOAT3 chestProbe{playerPosition.x, playerPosition.y - 0.08f, playerPosition.z};
                    float grabRadius = std::max(maze.TileMinimum() * 0.15f, limb * (5.9f + frontness * 2.2f));
                    if (Length3(Sub3(palmCenter, chestProbe)) < grabRadius) {
                        BeginDeath();
                    }
                }
                if (monsterDetail >= 1 || closeGrabWeight > 0.10f) {
                    for (int finger = -1; finger <= 1; ++finger) {
                        float f = static_cast<float>(finger);
                        float spread = f * limb * 0.68f * grabHandScale;
                        float hookCurl = (1.0f - std::abs(f)) * limb * 0.18f;
                        XMFLOAT3 base = Add3(padCenter, Add3(Scale3(palmRight, spread), Scale3(palmUp, limb * 0.34f * handScale)));
                        XMFLOAT3 mid = Add3(base, Add3(Scale3(palmUp, limb * 0.60f * handScale), Scale3(contactNormal, limb * 0.16f * (0.65f + swing))));
                        XMFLOAT3 tip = Add3(mid, Add3(Scale3(palmUp, limb * 0.20f * handScale), Scale3(contactNormal, -limb * (0.36f + swing * 0.18f) - hookCurl)));
                        organicSegment(base, mid, limb * 0.22f * handScale, limb * 0.13f * handScale, darkMat, 5);
                        organicSegment(mid, tip, limb * 0.14f * handScale, limb * 0.055f * handScale, darkMat, 5);
                    }
                }
                XMFLOAT3 wristWeb = Add3(palmCenter, Scale3(contactNormal, -limb * 0.34f));
                organicSegment(knee, wristWeb, limb * 0.78f, limb * 0.32f, limbMaterial + 0.035f, 7);
            }
        }
