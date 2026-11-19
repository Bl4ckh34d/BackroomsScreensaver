    void CreateLiquidTarget(const D3D11_TEXTURE2D_DESC& sceneDesc) {
        RenderTargetRuntimeState::LiquidTarget liquid;
        liquid.width = (sceneDesc.Width + 1) / 2;
        liquid.height = (sceneDesc.Height + 1) / 2;

        auto* device = d3dRuntime_.device.Get();
        D3D11_TEXTURE2D_DESC desc = sceneDesc;
        desc.Width = liquid.width;
        desc.Height = liquid.height;
        desc.Format = DXGI_FORMAT_R16G16B16A16_FLOAT;
        ComPtr<ID3D11Texture2D> color;
        if (FAILED(device->CreateTexture2D(&desc, nullptr, &color)) ||
            FAILED(device->CreateRenderTargetView(color.Get(), nullptr, &liquid.color)) ||
            FAILED(device->CreateShaderResourceView(color.Get(), nullptr, &liquid.colorSrv))) return;

        desc.Format = DXGI_FORMAT_R32_TYPELESS;
        desc.BindFlags = D3D11_BIND_DEPTH_STENCIL | D3D11_BIND_SHADER_RESOURCE;
        D3D11_DEPTH_STENCIL_VIEW_DESC dsv{};
        dsv.Format = DXGI_FORMAT_D32_FLOAT;
        dsv.ViewDimension = D3D11_DSV_DIMENSION_TEXTURE2D;
        D3D11_SHADER_RESOURCE_VIEW_DESC srv{};
        srv.Format = DXGI_FORMAT_R32_FLOAT;
        srv.ViewDimension = D3D11_SRV_DIMENSION_TEXTURE2D;
        srv.Texture2D.MipLevels = 1;
        ComPtr<ID3D11Texture2D> depth;
        if (FAILED(device->CreateTexture2D(&desc, nullptr, &depth)) ||
            FAILED(device->CreateDepthStencilView(depth.Get(), &dsv, &liquid.depth)) ||
            FAILED(device->CreateShaderResourceView(depth.Get(), &srv, &liquid.depthSrv))) return;

        const XMFLOAT4 scale{
            static_cast<float>(liquid.width) / sceneDesc.Width,
            static_cast<float>(liquid.height) / sceneDesc.Height,
            0.0f, 0.0f
        };
        D3D11_BUFFER_DESC constants{};
        constants.ByteWidth = sizeof(scale);
        constants.Usage = D3D11_USAGE_IMMUTABLE;
        constants.BindFlags = D3D11_BIND_CONSTANT_BUFFER;
        const D3D11_SUBRESOURCE_DATA data{&scale, 0, 0};
        if (FAILED(device->CreateBuffer(&constants, &data, &liquid.resolveConstants))) return;
        renderTargetRuntime_.liquid = std::move(liquid);
    }
