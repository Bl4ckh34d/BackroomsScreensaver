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
