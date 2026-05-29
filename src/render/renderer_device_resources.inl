    bool CreateStates() {
        D3D11_SAMPLER_DESC sd{};
        sd.Filter = D3D11_FILTER_ANISOTROPIC;
        sd.AddressU = D3D11_TEXTURE_ADDRESS_WRAP;
        sd.AddressV = D3D11_TEXTURE_ADDRESS_WRAP;
        sd.AddressW = D3D11_TEXTURE_ADDRESS_CLAMP;
        sd.MaxAnisotropy = 8;
        sd.MaxLOD = D3D11_FLOAT32_MAX;
        if (FAILED(device_->CreateSamplerState(&sd, &sampler_))) return false;

        D3D11_SAMPLER_DESC shadowSd{};
        shadowSd.Filter = D3D11_FILTER_COMPARISON_MIN_MAG_LINEAR_MIP_POINT;
        shadowSd.AddressU = D3D11_TEXTURE_ADDRESS_BORDER;
        shadowSd.AddressV = D3D11_TEXTURE_ADDRESS_BORDER;
        shadowSd.AddressW = D3D11_TEXTURE_ADDRESS_BORDER;
        shadowSd.BorderColor[0] = 1.0f;
        shadowSd.BorderColor[1] = 1.0f;
        shadowSd.BorderColor[2] = 1.0f;
        shadowSd.BorderColor[3] = 1.0f;
        shadowSd.ComparisonFunc = D3D11_COMPARISON_LESS_EQUAL;
        shadowSd.MaxLOD = D3D11_FLOAT32_MAX;
        if (FAILED(device_->CreateSamplerState(&shadowSd, &shadowSampler_))) return false;

        D3D11_SAMPLER_DESC postSd{};
        postSd.Filter = D3D11_FILTER_MIN_MAG_MIP_LINEAR;
        postSd.AddressU = D3D11_TEXTURE_ADDRESS_CLAMP;
        postSd.AddressV = D3D11_TEXTURE_ADDRESS_CLAMP;
        postSd.AddressW = D3D11_TEXTURE_ADDRESS_CLAMP;
        postSd.MaxLOD = D3D11_FLOAT32_MAX;
        if (FAILED(device_->CreateSamplerState(&postSd, &postSampler_))) return false;

        D3D11_RASTERIZER_DESC rd{};
        rd.FillMode = D3D11_FILL_SOLID;
        rd.CullMode = D3D11_CULL_NONE;
        rd.DepthClipEnable = TRUE;
        if (FAILED(device_->CreateRasterizerState(&rd, &rasterState_))) return false;

        D3D11_RASTERIZER_DESC shadowRd = rd;
        shadowRd.DepthBias = 32;
        shadowRd.SlopeScaledDepthBias = 1.6f;
        shadowRd.DepthBiasClamp = 0.0025f;
        if (FAILED(device_->CreateRasterizerState(&shadowRd, &shadowRasterState_))) return false;

        D3D11_DEPTH_STENCIL_DESC dsd{};
        dsd.DepthEnable = TRUE;
        dsd.DepthWriteMask = D3D11_DEPTH_WRITE_MASK_ALL;
        dsd.DepthFunc = D3D11_COMPARISON_LESS_EQUAL;
        if (FAILED(device_->CreateDepthStencilState(&dsd, &depthState_))) return false;
        D3D11_DEPTH_STENCIL_DESC lessDsd = dsd;
        lessDsd.DepthFunc = D3D11_COMPARISON_LESS;
        if (FAILED(device_->CreateDepthStencilState(&lessDsd, &depthLessState_))) return false;
        D3D11_DEPTH_STENCIL_DESC readOnlyDsd = dsd;
        readOnlyDsd.DepthWriteMask = D3D11_DEPTH_WRITE_MASK_ZERO;
        if (FAILED(device_->CreateDepthStencilState(&readOnlyDsd, &depthReadOnlyState_))) return false;
        D3D11_DEPTH_STENCIL_DESC liquidDsd = lessDsd;
        liquidDsd.DepthWriteMask = D3D11_DEPTH_WRITE_MASK_ALL;
        if (FAILED(device_->CreateDepthStencilState(&liquidDsd, &liquidDepthStencilState_))) return false;
        D3D11_DEPTH_STENCIL_DESC overlayDsd{};
        overlayDsd.DepthEnable = FALSE;
        overlayDsd.DepthWriteMask = D3D11_DEPTH_WRITE_MASK_ZERO;
        overlayDsd.DepthFunc = D3D11_COMPARISON_ALWAYS;
        if (FAILED(device_->CreateDepthStencilState(&overlayDsd, &depthDisabledState_))) return false;

        D3D11_BLEND_DESC bd{};
        bd.RenderTarget[0].BlendEnable = TRUE;
        bd.RenderTarget[0].SrcBlend = D3D11_BLEND_SRC_ALPHA;
        bd.RenderTarget[0].DestBlend = D3D11_BLEND_INV_SRC_ALPHA;
        bd.RenderTarget[0].BlendOp = D3D11_BLEND_OP_ADD;
        bd.RenderTarget[0].SrcBlendAlpha = D3D11_BLEND_ONE;
        bd.RenderTarget[0].DestBlendAlpha = D3D11_BLEND_ZERO;
        bd.RenderTarget[0].BlendOpAlpha = D3D11_BLEND_OP_ADD;
        bd.RenderTarget[0].RenderTargetWriteMask = D3D11_COLOR_WRITE_ENABLE_ALL;
        return SUCCEEDED(device_->CreateBlendState(&bd, &alphaBlend_));
    }

    bool CreateShadowResources() {
        shadowMapSize_ = static_cast<UINT>(std::clamp(settings_.flashlightShadowMapSize, 512, 4096));
        monsterEyeShadowMapSize_ = static_cast<UINT>(std::clamp(settings_.flashlightShadowMapSize / 2, 512, 2048));

        D3D11_TEXTURE2D_DESC td{};
        td.Width = shadowMapSize_;
        td.Height = shadowMapSize_;
        td.MipLevels = 1;
        td.ArraySize = 1;
        td.Format = DXGI_FORMAT_R32_TYPELESS;
        td.SampleDesc.Count = 1;
        td.Usage = D3D11_USAGE_DEFAULT;
        td.BindFlags = D3D11_BIND_DEPTH_STENCIL | D3D11_BIND_SHADER_RESOURCE;
        if (FAILED(device_->CreateTexture2D(&td, nullptr, &shadowDepth_))) return false;

        D3D11_DEPTH_STENCIL_VIEW_DESC dsvDesc{};
        dsvDesc.Format = DXGI_FORMAT_D32_FLOAT;
        dsvDesc.ViewDimension = D3D11_DSV_DIMENSION_TEXTURE2D;
        if (FAILED(device_->CreateDepthStencilView(shadowDepth_.Get(), &dsvDesc, &shadowDsv_))) return false;

        D3D11_SHADER_RESOURCE_VIEW_DESC srvDesc{};
        srvDesc.Format = DXGI_FORMAT_R32_FLOAT;
        srvDesc.ViewDimension = D3D11_SRV_DIMENSION_TEXTURE2D;
        srvDesc.Texture2D.MipLevels = 1;
        if (FAILED(device_->CreateShaderResourceView(shadowDepth_.Get(), &srvDesc, &shadowSrv_))) return false;

        td.Width = monsterEyeShadowMapSize_;
        td.Height = monsterEyeShadowMapSize_;
        for (size_t i = 0; i < monsterEyeShadowDepth_.size(); ++i) {
            monsterEyeShadowDepth_[i].Reset();
            monsterEyeShadowDsv_[i].Reset();
            monsterEyeShadowSrv_[i].Reset();
            if (FAILED(device_->CreateTexture2D(&td, nullptr, &monsterEyeShadowDepth_[i]))) return false;
            if (FAILED(device_->CreateDepthStencilView(monsterEyeShadowDepth_[i].Get(), &dsvDesc, &monsterEyeShadowDsv_[i]))) return false;
            if (FAILED(device_->CreateShaderResourceView(monsterEyeShadowDepth_[i].Get(), &srvDesc, &monsterEyeShadowSrv_[i]))) return false;
        }
        return true;
    }

    bool CreateConstantBuffer() {
        D3D11_BUFFER_DESC bd{};
        bd.ByteWidth = sizeof(SceneConstants);
        bd.Usage = D3D11_USAGE_DYNAMIC;
        bd.BindFlags = D3D11_BIND_CONSTANT_BUFFER;
        bd.CPUAccessFlags = D3D11_CPU_ACCESS_WRITE;
        return SUCCEEDED(device_->CreateBuffer(&bd, nullptr, &constantBuffer_));
    }
