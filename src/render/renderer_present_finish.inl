// Postprocess, overlays, resource cleanup, and swap-chain present.

        ID3D11ShaderResourceView* nullSrvs[] = {nullptr, nullptr, nullptr, nullptr, nullptr, nullptr, nullptr, nullptr, nullptr, nullptr, nullptr, nullptr, nullptr, nullptr};
        d3dRuntime_.context->PSSetShaderResources(0, 14, nullSrvs);
        d3dRuntime_.context->DSSetShaderResources(0, 14, nullSrvs);
        d3dRuntime_.context->HSSetShader(nullptr, nullptr, 0);
        d3dRuntime_.context->DSSetShader(nullptr, nullptr, 0);
        if (postAvailable) {
            StartupProfileLine(L"Render before DrawPostProcess");
            DrawPostProcess();
            renderProfile.Mark(L"PostProcess");
        }
        MarkGpuProfile(GpuProfileMarker::PostProcess);

        if (!monsterPreview_.active) {
            StartupProfileLine(L"Render before DrawMapOverlay");
            DrawMapOverlay();
            renderProfile.Mark(L"MapOverlay");
            StartupProfileLine(L"Render before DrawDreadMeterOverlay");
            DrawDreadMeterOverlay();
            renderProfile.Mark(L"DreadOverlay");
            StartupProfileLine(L"Render before DrawGameHudOverlay");
            DrawGameHudOverlay();
            renderProfile.Mark(L"GameHudOverlay");
        }
        MarkGpuProfile(GpuProfileMarker::Overlays);

        d3dRuntime_.context->PSSetShaderResources(0, 14, nullSrvs);
        d3dRuntime_.context->DSSetShaderResources(0, 14, nullSrvs);
        d3dRuntime_.context->HSSetShader(nullptr, nullptr, 0);
        d3dRuntime_.context->DSSetShader(nullptr, nullptr, 0);
        EndGpuProfileFrame();
        StartupProfileLine(L"Render before Present");
        HRESULT presentHr = d3dRuntime_.swapChain->Present(presentRuntime_.syncInterval, presentRuntime_.flags);
        presentRuntime_.lastCompleted = presentHr != DXGI_ERROR_WAS_STILL_DRAWING;
        renderProfile.Mark(L"Present");
