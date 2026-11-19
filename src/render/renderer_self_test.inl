    bool VerifyLiquidTexture() {
        if (!renderTargetRuntime_.liquid.resolveConstants || !shaders_.liquidResolvePixelShader) return false;
        EnterDebugViewer(DebugSliceEffect::FloorWater, 3);
        ApplyDebugSliceCamera();
        debugRuntime_.bloodDebugStartTicks = GetTickCount64() -
            static_cast<ULONGLONG>(DebugSliceLoopSeconds() * 720.0f);
        timeRuntime_.time = 45.0f;
        Render();

        ComPtr<ID3D11Resource> liquidColor;
        renderTargetRuntime_.liquid.colorSrv->GetResource(&liquidColor);
        D3D11_TEXTURE2D_DESC desc{};
        desc.Width = renderTargetRuntime_.liquid.width;
        desc.Height = renderTargetRuntime_.liquid.height;
        desc.ArraySize = 1;
        desc.MipLevels = 1;
        desc.Format = DXGI_FORMAT_R16G16B16A16_FLOAT;
        desc.SampleDesc.Count = 1;
        desc.Usage = D3D11_USAGE_STAGING;
        desc.CPUAccessFlags = D3D11_CPU_ACCESS_READ;
        ComPtr<ID3D11Texture2D> readback;
        if (FAILED(d3dRuntime_.device->CreateTexture2D(&desc, nullptr, &readback))) return false;
        d3dRuntime_.context->CopyResource(readback.Get(), liquidColor.Get());
        D3D11_MAPPED_SUBRESOURCE mapped{};
        if (FAILED(d3dRuntime_.context->Map(readback.Get(), 0, D3D11_MAP_READ, 0, &mapped))) return false;
        unsigned coveredPixels = 0;
        for (UINT y = 0; y < desc.Height; ++y) {
            const auto* row = reinterpret_cast<const uint16_t*>(
                static_cast<const uint8_t*>(mapped.pData) + static_cast<size_t>(y) * mapped.RowPitch);
            for (UINT x = 0; x < desc.Width; ++x) {
                if (row[x * 4 + 3] > 0) ++coveredPixels;
            }
        }
        d3dRuntime_.context->Unmap(readback.Get(), 0);
        gEffectDebugViewer = false;
        gBloodDebugEveryWall = false;
        StartupProfileLine(L"SelfTest liquid texture covered pixels: " + std::to_wstring(coveredPixels));
        return coveredPixels > 0;
    }

    bool VerifyPreparedMazeAndRespawn() {
        EnsureFullSceneAssets();
        sessionRuntime_.ConfigureFromSpec(GameSessionSpec::PlayableRun());
        sessionRuntime_.SeedRuntime(325127u);
        gameWorld_.progressionEnabled = true;
        gameWorld_.BeginLayerRun(false, sessionRuntime_.rng);
        BeginPlayableLevel(4, false);

        // Enter the normal score continuation without completing a full playthrough.
        gameWorld_.CompleteCurrentPlayableLevel();
        PrepareNextPlayableMaze();
        if (!nextMazePreparation_ || !nextMazePreparation_->scene.valid() ||
            nextMazePreparation_->scene.wait_for(std::chrono::seconds(10)) != std::future_status::ready) return false;
        auto prepared = nextMazePreparation_->scene.get();
        if (!prepared || !prepared->renderBuffers_.indexBuffer) return false;
        auto expectedMesh = prepared->renderBuffers_.indexBuffer;
        const auto expectedLayout = prepared->gameWorld_.maze.open;
        const auto expectedWetTiles = prepared->effectRuntime_.wetFootstepTiles;
        const auto expectedScares = prepared->scareRuntime_.bloodScarePoints.size();
        std::promise<std::unique_ptr<Renderer>> ready;
        nextMazePreparation_->scene = ready.get_future();
        ready.set_value(std::move(prepared));
        ContinueAfterScoreScreen();
        if (gameWorld_.CurrentPlayableLevel().levelInLayer != 5 ||
            !gameWorld_.PlayableLevelRunning() || renderBuffers_.indexBuffer.Get() != expectedMesh.Get() ||
            gameWorld_.maze.open != expectedLayout || effectRuntime_.wetFootstepTiles != expectedWetTiles ||
            scareRuntime_.bloodScarePoints.size() != expectedScares) return false;

        // Dirty an effect, then advance the regular death sequence past respawn.
        std::fill(effectRuntime_.lampDamagePixels.begin(), effectRuntime_.lampDamagePixels.end(), 255);
        scareRuntime_.bloodScarePoints.clear();
        if (!gameWorld_.BeginDeath()) return false;
        UpdateDeath(4.5f);
        return !gameWorld_.deathActive && renderBuffers_.indexBuffer.Get() == expectedMesh.Get() &&
            gameWorld_.maze.open == expectedLayout && effectRuntime_.wetFootstepTiles == expectedWetTiles &&
            effectRuntime_.lampDamagePixels == effectRuntime_.bakedLampDamageBaseline &&
            scareRuntime_.bloodScarePoints.size() == expectedScares;
    }
