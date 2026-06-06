    bool CreateBackBuffer() {
        ComPtr<ID3D11Texture2D> backBuffer;
        HRESULT hr = d3dRuntime_.swapChain->GetBuffer(0, IID_PPV_ARGS(&backBuffer));
        if (FAILED(hr)) return false;
        hr = d3dRuntime_.device->CreateRenderTargetView(backBuffer.Get(), nullptr, &renderTargetRuntime_.rtv);
        if (FAILED(hr)) return false;

        int renderScalePercent = std::clamp(settingsRuntime_.live.renderScalePercent, 50, 100);
        renderTargetRuntime_.sceneWidth = static_cast<UINT>(std::max(1, static_cast<int>(std::lround(static_cast<double>(hostRuntime_.width) * renderScalePercent / 100.0))));
        renderTargetRuntime_.sceneHeight = static_cast<UINT>(std::max(1, static_cast<int>(std::lround(static_cast<double>(hostRuntime_.height) * renderScalePercent / 100.0))));
        UINT requestedSamples = static_cast<UINT>(std::clamp(AntiAliasingMsaaSamples(settingsRuntime_.live.antiAliasing), 1, 16));
        UINT sampleCount = 1;
        if (requestedSamples > 1) {
            constexpr UINT fallbacks[] = {16, 8, 4, 2};
            for (UINT candidate : fallbacks) {
                if (candidate > requestedSamples) continue;
                UINT quality = 0;
                if (SUCCEEDED(d3dRuntime_.device->CheckMultisampleQualityLevels(DXGI_FORMAT_R8G8B8A8_UNORM, candidate, &quality)) && quality > 0) {
                    sampleCount = candidate;
                    break;
                }
            }
        }
        renderTargetRuntime_.sceneSampleCount = sampleCount;

        D3D11_TEXTURE2D_DESC sceneDesc{};
        sceneDesc.Width = renderTargetRuntime_.sceneWidth;
        sceneDesc.Height = renderTargetRuntime_.sceneHeight;
        sceneDesc.MipLevels = 1;
        sceneDesc.ArraySize = 1;
        sceneDesc.Format = DXGI_FORMAT_R8G8B8A8_UNORM;
        sceneDesc.SampleDesc.Count = 1;
        sceneDesc.Usage = D3D11_USAGE_DEFAULT;
        sceneDesc.BindFlags = D3D11_BIND_RENDER_TARGET | D3D11_BIND_SHADER_RESOURCE;
        hr = d3dRuntime_.device->CreateTexture2D(&sceneDesc, nullptr, &renderTargetRuntime_.sceneColor);
        if (FAILED(hr)) return false;
        hr = d3dRuntime_.device->CreateRenderTargetView(renderTargetRuntime_.sceneColor.Get(), nullptr, &renderTargetRuntime_.sceneColorRtv);
        if (FAILED(hr)) return false;
        hr = d3dRuntime_.device->CreateShaderResourceView(renderTargetRuntime_.sceneColor.Get(), nullptr, &renderTargetRuntime_.sceneColorSrv);
        if (FAILED(hr)) return false;

        if (sampleCount > 1) {
            D3D11_TEXTURE2D_DESC msaaDesc = sceneDesc;
            msaaDesc.SampleDesc.Count = sampleCount;
            msaaDesc.BindFlags = D3D11_BIND_RENDER_TARGET;
            hr = d3dRuntime_.device->CreateTexture2D(&msaaDesc, nullptr, &renderTargetRuntime_.sceneColorMsaa);
            if (FAILED(hr)) return false;
            hr = d3dRuntime_.device->CreateRenderTargetView(renderTargetRuntime_.sceneColorMsaa.Get(), nullptr, &renderTargetRuntime_.sceneColorMsaaRtv);
            if (FAILED(hr)) return false;
        }

        D3D11_TEXTURE2D_DESC dd{};
        dd.Width = renderTargetRuntime_.sceneWidth;
        dd.Height = renderTargetRuntime_.sceneHeight;
        dd.MipLevels = 1;
        dd.ArraySize = 1;
        dd.Format = DXGI_FORMAT_D24_UNORM_S8_UINT;
        dd.SampleDesc.Count = sampleCount;
        dd.BindFlags = D3D11_BIND_DEPTH_STENCIL;
        hr = d3dRuntime_.device->CreateTexture2D(&dd, nullptr, &renderTargetRuntime_.depth);
        if (FAILED(hr)) return false;
        hr = d3dRuntime_.device->CreateDepthStencilView(renderTargetRuntime_.depth.Get(), nullptr, &renderTargetRuntime_.dsv);
        return SUCCEEDED(hr);
    }
