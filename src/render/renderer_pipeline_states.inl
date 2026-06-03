    bool CreateStates() {
        D3D11_SAMPLER_DESC sd{};
        sd.Filter = D3D11_FILTER_ANISOTROPIC;
        sd.AddressU = D3D11_TEXTURE_ADDRESS_WRAP;
        sd.AddressV = D3D11_TEXTURE_ADDRESS_WRAP;
        sd.AddressW = D3D11_TEXTURE_ADDRESS_CLAMP;
        sd.MaxAnisotropy = 8;
        sd.MaxLOD = D3D11_FLOAT32_MAX;
        if (FAILED(d3dRuntime_.device->CreateSamplerState(&sd, &pipelineStates_.sampler))) return false;

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
        if (FAILED(d3dRuntime_.device->CreateSamplerState(&shadowSd, &pipelineStates_.shadowSampler))) return false;

        D3D11_SAMPLER_DESC postSd{};
        postSd.Filter = D3D11_FILTER_MIN_MAG_MIP_LINEAR;
        postSd.AddressU = D3D11_TEXTURE_ADDRESS_CLAMP;
        postSd.AddressV = D3D11_TEXTURE_ADDRESS_CLAMP;
        postSd.AddressW = D3D11_TEXTURE_ADDRESS_CLAMP;
        postSd.MaxLOD = D3D11_FLOAT32_MAX;
        if (FAILED(d3dRuntime_.device->CreateSamplerState(&postSd, &pipelineStates_.postSampler))) return false;

        D3D11_RASTERIZER_DESC rd{};
        rd.FillMode = D3D11_FILL_SOLID;
        rd.CullMode = D3D11_CULL_NONE;
        rd.DepthClipEnable = TRUE;
        if (FAILED(d3dRuntime_.device->CreateRasterizerState(&rd, &pipelineStates_.rasterState))) return false;

        D3D11_RASTERIZER_DESC shadowRd = rd;
        shadowRd.DepthBias = 32;
        shadowRd.SlopeScaledDepthBias = 1.6f;
        shadowRd.DepthBiasClamp = 0.0025f;
        if (FAILED(d3dRuntime_.device->CreateRasterizerState(&shadowRd, &pipelineStates_.shadowRasterState))) return false;

        D3D11_DEPTH_STENCIL_DESC dsd{};
        dsd.DepthEnable = TRUE;
        dsd.DepthWriteMask = D3D11_DEPTH_WRITE_MASK_ALL;
        dsd.DepthFunc = D3D11_COMPARISON_LESS_EQUAL;
        if (FAILED(d3dRuntime_.device->CreateDepthStencilState(&dsd, &pipelineStates_.depthState))) return false;
        D3D11_DEPTH_STENCIL_DESC lessDsd = dsd;
        lessDsd.DepthFunc = D3D11_COMPARISON_LESS;
        if (FAILED(d3dRuntime_.device->CreateDepthStencilState(&lessDsd, &pipelineStates_.depthLessState))) return false;
        D3D11_DEPTH_STENCIL_DESC readOnlyDsd = dsd;
        readOnlyDsd.DepthWriteMask = D3D11_DEPTH_WRITE_MASK_ZERO;
        if (FAILED(d3dRuntime_.device->CreateDepthStencilState(&readOnlyDsd, &pipelineStates_.depthReadOnlyState))) return false;
        D3D11_DEPTH_STENCIL_DESC liquidDsd = lessDsd;
        liquidDsd.DepthWriteMask = D3D11_DEPTH_WRITE_MASK_ALL;
        if (FAILED(d3dRuntime_.device->CreateDepthStencilState(&liquidDsd, &pipelineStates_.liquidDepthStencilState))) return false;
        D3D11_DEPTH_STENCIL_DESC overlayDsd{};
        overlayDsd.DepthEnable = FALSE;
        overlayDsd.DepthWriteMask = D3D11_DEPTH_WRITE_MASK_ZERO;
        overlayDsd.DepthFunc = D3D11_COMPARISON_ALWAYS;
        if (FAILED(d3dRuntime_.device->CreateDepthStencilState(&overlayDsd, &pipelineStates_.depthDisabledState))) return false;

        D3D11_BLEND_DESC bd{};
        bd.RenderTarget[0].BlendEnable = TRUE;
        bd.RenderTarget[0].SrcBlend = D3D11_BLEND_SRC_ALPHA;
        bd.RenderTarget[0].DestBlend = D3D11_BLEND_INV_SRC_ALPHA;
        bd.RenderTarget[0].BlendOp = D3D11_BLEND_OP_ADD;
        bd.RenderTarget[0].SrcBlendAlpha = D3D11_BLEND_ONE;
        bd.RenderTarget[0].DestBlendAlpha = D3D11_BLEND_ZERO;
        bd.RenderTarget[0].BlendOpAlpha = D3D11_BLEND_OP_ADD;
        bd.RenderTarget[0].RenderTargetWriteMask = D3D11_COLOR_WRITE_ENABLE_ALL;
        if (FAILED(d3dRuntime_.device->CreateBlendState(&bd, &pipelineStates_.alphaBlend))) return false;
        CreateGpuProfileQueries();
        return true;
    }
