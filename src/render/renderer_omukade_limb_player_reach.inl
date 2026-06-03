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
