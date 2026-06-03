// Scene constants for camera, fixture shadows, lighting, fog, AO, postprocess, maze, texture dirt, and transitions.

        SceneConstants cb{};
        XMStoreFloat4x4(&cb.viewProj, view * proj);
        XMStoreFloat4x4(&cb.lightViewProj, lightViewProj);
        XMStoreFloat4x4(&cb.fixtureLightViewProj, XMMatrixIdentity());
        XMFLOAT3 eyePos{};
        XMStoreFloat3(&eyePos, eye);
        XMFLOAT3 viewDirFloat{};
        XMStoreFloat3(&viewDirFloat, viewDir);

        bool fixtureShadowActive = false;
        XMFLOAT3 fixtureShadowPos{0.0f, 0.0f, 0.0f};
        XMFLOAT3 fixtureShadowDir{0.0f, -1.0f, 0.0f};
        const Maze& lightingMaze = *world.maze;
        float fixtureShadowRange = std::max(settingsRuntime_.live.wallHeightMeters + 0.75f, tileAverage * 1.70f);
        XMMATRIX fixtureLightViewProj = XMMatrixIdentity();
        if (!monsterPreview_.active && IsPlayableSimulationMode(sessionRuntime_.mode) && shadowResources_.fixtureShadowDsv && shadowResources_.fixtureShadowSrv &&
            settingsRuntime_.live.lampIntensity > 0.001f && settingsRuntime_.live.lampOnRatio > 0.001f && !effectRuntime_.runtimeLamps.empty()) {
            Tile cameraTile = CameraTile();
            float maxCandidateDistance = std::clamp(tileAverage * 3.2f, 5.5f, 10.5f);
            float maxCandidateDistanceSq = maxCandidateDistance * maxCandidateDistance;
            float bestScore = std::numeric_limits<float>::infinity();
            const RuntimeLampState* bestLamp = nullptr;
            for (const RuntimeLampState& lamp : effectRuntime_.runtimeLamps) {
                if (lamp.broken || !lightingMaze.InBounds(lamp.tile.x, lamp.tile.y) || !lightingMaze.LineClear(cameraTile, lamp.tile)) continue;
                float dx = lamp.pos.x - eyePos.x;
                float dy = lamp.pos.y - eyePos.y;
                float dz = lamp.pos.z - eyePos.z;
                float distSq = dx * dx + dy * dy + dz * dz;
                if (distSq > maxCandidateDistanceSq) continue;
                float dist = std::sqrt(std::max(0.0001f, distSq));
                float viewFacing = (dx * viewDirFloat.x + dy * viewDirFloat.y + dz * viewDirFloat.z) / dist;
                if (dist > tileAverage * 1.25f && viewFacing < -0.18f) continue;
                float screenBias = 1.0f - std::clamp((viewFacing + 0.18f) / 1.18f, 0.0f, 1.0f);
                float score = distSq * (0.62f + screenBias * 1.25f);
                if (score < bestScore) {
                    bestScore = score;
                    bestLamp = &lamp;
                }
            }
            if (bestLamp) {
                fixtureShadowActive = true;
                fixtureShadowPos = bestLamp->pos;
                fixtureShadowPos.y -= 0.015f;
                XMVECTOR fixturePos = XMLoadFloat3(&fixtureShadowPos);
                XMVECTOR fixtureDir = XMLoadFloat3(&fixtureShadowDir);
                XMVECTOR fixtureUp = XMVectorSet(0.0f, 0.0f, 1.0f, 0.0f);
                XMMATRIX fixtureView = XMMatrixLookAtLH(fixturePos, fixturePos + fixtureDir, fixtureUp);
                XMMATRIX fixtureProj = XMMatrixPerspectiveFovLH(116.0f * kPi / 180.0f, 1.0f, 0.035f, fixtureShadowRange);
                fixtureLightViewProj = fixtureView * fixtureProj;
                XMStoreFloat4x4(&cb.fixtureLightViewProj, fixtureLightViewProj);
            }
        }
