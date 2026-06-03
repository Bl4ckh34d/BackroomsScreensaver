// Render preflight, target clear, viewport, camera, projection, and light-space setup.

        presentRuntime_.lastCompleted = false;
        if (!presentRuntime_.enabled) {
            presentRuntime_.lastCompleted = true;
            return;
        }
        if (!renderTargetRuntime_.rtv || !renderTargetRuntime_.dsv) {
            presentRuntime_.lastCompleted = true;
            return;
        }
        StartupProfile renderProfile(L"Render");
        BeginGpuProfileFrame();
        bool postAvailable = renderTargetRuntime_.sceneColorRtv && renderTargetRuntime_.sceneColorSrv && shaders_.postVertexShader && shaders_.postPixelShader && pipelineStates_.postSampler;
        ID3D11RenderTargetView* sceneTarget = postAvailable ? renderTargetRuntime_.sceneColorRtv.Get() : renderTargetRuntime_.rtv.Get();

        float clear[4] = {0.004f, 0.004f, 0.004f, 1.0f};
        if (monsterPreview_.active) {
            if (monsterPreview_.view == MonsterPreviewView::Orbit) {
                clear[0] = clear[1] = clear[2] = 0.93f;
            } else {
                clear[0] = clear[1] = clear[2] = 0.0f;
            }
        }
        d3dRuntime_.context->ClearRenderTargetView(renderTargetRuntime_.rtv.Get(), clear);
        if (postAvailable) {
            d3dRuntime_.context->ClearRenderTargetView(renderTargetRuntime_.sceneColorRtv.Get(), clear);
        }
        d3dRuntime_.context->ClearDepthStencilView(renderTargetRuntime_.dsv.Get(), D3D11_CLEAR_DEPTH | D3D11_CLEAR_STENCIL, 1.0f, 0);
        renderProfile.Mark(L"ClearTargets");
        MarkGpuProfile(GpuProfileMarker::ClearTargets);

        D3D11_VIEWPORT vp{};
        vp.Width = static_cast<float>(hostRuntime_.width);
        vp.Height = static_cast<float>(hostRuntime_.height);
        vp.MinDepth = 0.0f;
        vp.MaxDepth = 1.0f;
        d3dRuntime_.context->RSSetViewports(1, &vp);

        GameWorldRenderSnapshot world = gameWorld_.BuildRenderSnapshot();
        float tileAverage = world.maze ? world.maze->TileAverage() : kTile;
        float deathProgress = world.deathActive ? Clamp01(world.deathTimer / 4.25f) : 0.0f;
        float deathFadeProgress = world.deathActive ? Clamp01((world.deathTimer - 0.10f) / 3.55f) : 0.0f;
        float deathFocus = world.deathActive ? SmoothStep(0.0f, 0.24f, world.deathTimer) : 0.0f;
        XMFLOAT3 f = Forward();
        float stepPhase = world.playerStepPhase;
        float runCameraMotion = 1.0f + world.playerRunIntensity * 0.42f + world.playerRunEffort * 0.55f;
        float stepWave = std::sin(stepPhase * 2.0f);
        float sideSway = stepWave * settingsRuntime_.live.sideSwayAmount * 0.26f * runCameraMotion * (1.0f - deathFocus);
        XMVECTOR right = XMVectorSet(std::cos(world.playerBodyYaw), 0.0f, -std::sin(world.playerBodyYaw), 0.0f);
        XMVECTOR eye = XMLoadFloat3(&world.playerPosition) + right * sideSway;
        XMVECTOR worldUp = XMVectorSet(0, 1, 0, 0);
        float bobPitch = stepWave * 0.0045f * runCameraMotion * (1.0f - deathFocus);
        XMVECTOR viewDir = XMVector3Normalize(XMVectorSet(f.x, world.playerPitch + bobPitch, f.z, 0.0f));
        XMVECTOR viewRight = XMVector3Normalize(XMVector3Cross(worldUp, viewDir));
        if (monsterPreview_.active && monsterPreview_.view == MonsterPreviewView::Top) {
            XMVECTOR topTarget = XMVectorSet(world.monsterPosition.x, 1.20f, world.monsterPosition.z, 0.0f);
            viewDir = XMVector3Normalize(topTarget - eye);
            worldUp = XMVectorSet(0.0f, 0.0f, -1.0f, 0.0f);
            viewRight = XMVector3Normalize(XMVector3Cross(worldUp, viewDir));
        }
        if (world.deathActive) {
            float jitterStrength = (1.0f - SmoothStep(0.74f, 1.0f, deathProgress)) * (0.010f + deathProgress * 0.025f);
            float jitterX = (std::sin(timeRuntime_.time * 31.0f) + std::sin(timeRuntime_.time * 57.0f) * 0.45f) * jitterStrength;
            float jitterY = (std::sin(timeRuntime_.time * 43.0f) + std::sin(timeRuntime_.time * 71.0f) * 0.35f) * jitterStrength;
            viewDir = XMVector3Normalize(viewDir + viewRight * jitterX + worldUp * jitterY);
            viewRight = XMVector3Normalize(XMVector3Cross(worldUp, viewDir));
        }
        float tunnelRoll = world.playerTunnelLeanSide * SmoothStep(0.0f, 1.0f, world.playerTunnelLeanAmount) * 0.115f;
        float roll = world.deathActive
            ? (std::sin(timeRuntime_.time * 18.0f) * 0.075f + std::sin(timeRuntime_.time * 37.0f) * 0.045f) * SmoothStep(0.06f, 0.42f, deathProgress)
            : viewRuntime_.stumbleYawOffset * 0.12f * SmoothStep(0.0f, 0.35f, viewRuntime_.stumbleTimer) + tunnelRoll;
        XMVECTOR up = XMVector3Normalize(worldUp * std::cos(roll) + viewRight * std::sin(roll));
        XMVECTOR at = eye + viewDir;
        XMMATRIX view = XMMatrixLookAtLH(eye, at, up);
        float aspect = static_cast<float>(std::max<LONG>(1, hostRuntime_.width)) / static_cast<float>(std::max<LONG>(1, hostRuntime_.height));
        float fovDegrees = monsterPreview_.active ? 48.0f : 70.0f - viewRuntime_.dangerLevel * 3.5f;
        if (sessionRuntime_.mode == RendererRuntimeMode::MainMenu) {
            fovDegrees = 84.0f;
        }
        if (gEffectDebugViewer || gBloodDebugEveryWall || settingsRuntime_.live.bloodStudyView) {
            fovDegrees = 86.0f;
        }
        if (world.deathActive) {
            fovDegrees = Lerp(70.0f, 58.0f, SmoothStep(0.05f, 0.66f, deathProgress));
        }
        float exitStepStart = ExitAlignSeconds() + std::max(0.05f, settingsRuntime_.live.exitDoorOpenSeconds * 0.68f);
        float exitStepEnd = exitStepStart + settingsRuntime_.live.exitStepSeconds;
        float exitFade = world.exitTransitionActive ? SmoothStep(exitStepEnd - settingsRuntime_.live.exitFadeSeconds * 0.64f, exitStepEnd + settingsRuntime_.live.exitFadeSeconds * 0.36f, world.exitTransitionTimer) : 0.0f;
        float fadeIn = (viewRuntime_.fadeInTimer > 0.0f && settingsRuntime_.live.fadeInSeconds > 0.001f)
            ? 1.0f - SmoothStep(0.0f, settingsRuntime_.live.fadeInSeconds, settingsRuntime_.live.fadeInSeconds - viewRuntime_.fadeInTimer)
            : 0.0f;
        float transitionFade = std::max(std::max(exitFade, fadeIn), menuRuntime_.startTransitionFade);
        float viewFarMeters = monsterPreview_.active ? 1000.0f : std::max(72.0f, settingsRuntime_.live.fogEndMeters + tileAverage * 12.0f);
        XMMATRIX proj = XMMatrixPerspectiveFovLH(fovDegrees * kPi / 180.0f, aspect, 0.045f, viewFarMeters);

        XMFLOAT3 flashlightForward = FlashlightForward();
        XMVECTOR lightDir = XMVector3Normalize(XMVectorSet(flashlightForward.x, flashlightForward.y, flashlightForward.z, 0.0f));
        XMVECTOR lightPos = eye + viewRight * 0.16f - up * 0.18f + viewDir * 0.08f;
        float shadowDistance = settingsRuntime_.live.flashlightShadowDistanceMeters;
        float coneHalfRadians = std::clamp(settingsRuntime_.live.flashlightConeDegrees, 20.0f, 140.0f) * 0.5f * kPi / 180.0f;
        float coneOuter = std::cos(coneHalfRadians);
        float coneInner = std::cos(std::max(3.0f * kPi / 180.0f, coneHalfRadians * 0.36f));
        float shadowFov = std::clamp(settingsRuntime_.live.flashlightConeDegrees + 8.0f, 36.0f, 150.0f) * kPi / 180.0f;
        XMMATRIX lightView = XMMatrixLookAtLH(lightPos, lightPos + lightDir, up);
        XMMATRIX lightProj = XMMatrixPerspectiveFovLH(shadowFov, 1.0f, 0.06f, shadowDistance);
        XMMATRIX lightViewProj = lightView * lightProj;
        XMFLOAT3 lightPosFloat{};
        XMFLOAT3 lightDirFloat{};
        XMStoreFloat3(&lightPosFloat, lightPos);
        XMStoreFloat3(&lightDirFloat, lightDir);

