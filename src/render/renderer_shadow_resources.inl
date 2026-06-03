    bool CreateShadowResources() {
        shadowResources_.shadowMapSize = static_cast<UINT>(std::clamp(settingsRuntime_.live.flashlightShadowMapSize, 512, 4096));
        shadowResources_.fixtureShadowMapSize = static_cast<UINT>(std::clamp(settingsRuntime_.live.flashlightShadowMapSize / 4, 512, 1024));

        D3D11_TEXTURE2D_DESC td{};
        td.Width = shadowResources_.shadowMapSize;
        td.Height = shadowResources_.shadowMapSize;
        td.MipLevels = 1;
        td.ArraySize = 1;
        td.Format = DXGI_FORMAT_R32_TYPELESS;
        td.SampleDesc.Count = 1;
        td.Usage = D3D11_USAGE_DEFAULT;
        td.BindFlags = D3D11_BIND_DEPTH_STENCIL | D3D11_BIND_SHADER_RESOURCE;
        if (FAILED(d3dRuntime_.device->CreateTexture2D(&td, nullptr, &shadowResources_.shadowDepth))) return false;

        D3D11_DEPTH_STENCIL_VIEW_DESC dsvDesc{};
        dsvDesc.Format = DXGI_FORMAT_D32_FLOAT;
        dsvDesc.ViewDimension = D3D11_DSV_DIMENSION_TEXTURE2D;
        if (FAILED(d3dRuntime_.device->CreateDepthStencilView(shadowResources_.shadowDepth.Get(), &dsvDesc, &shadowResources_.shadowDsv))) return false;

        D3D11_SHADER_RESOURCE_VIEW_DESC srvDesc{};
        srvDesc.Format = DXGI_FORMAT_R32_FLOAT;
        srvDesc.ViewDimension = D3D11_SRV_DIMENSION_TEXTURE2D;
        srvDesc.Texture2D.MipLevels = 1;
        if (FAILED(d3dRuntime_.device->CreateShaderResourceView(shadowResources_.shadowDepth.Get(), &srvDesc, &shadowResources_.shadowSrv))) return false;

        td.Width = shadowResources_.fixtureShadowMapSize;
        td.Height = shadowResources_.fixtureShadowMapSize;
        shadowResources_.fixtureShadowDepth.Reset();
        shadowResources_.fixtureShadowDsv.Reset();
        shadowResources_.fixtureShadowSrv.Reset();
        if (FAILED(d3dRuntime_.device->CreateTexture2D(&td, nullptr, &shadowResources_.fixtureShadowDepth)) ||
            FAILED(d3dRuntime_.device->CreateDepthStencilView(shadowResources_.fixtureShadowDepth.Get(), &dsvDesc, &shadowResources_.fixtureShadowDsv)) ||
            FAILED(d3dRuntime_.device->CreateShaderResourceView(shadowResources_.fixtureShadowDepth.Get(), &srvDesc, &shadowResources_.fixtureShadowSrv))) {
            shadowResources_.fixtureShadowDepth.Reset();
            shadowResources_.fixtureShadowDsv.Reset();
            shadowResources_.fixtureShadowSrv.Reset();
            StartupProfileLine(L"Fixture shadow resources unavailable; continuing without fixture-cast shadows.");
        }

        return true;
    }
