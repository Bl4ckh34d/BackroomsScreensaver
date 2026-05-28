    bool CreateBackBuffer() {
        ComPtr<ID3D11Texture2D> backBuffer;
        HRESULT hr = swapChain_->GetBuffer(0, IID_PPV_ARGS(&backBuffer));
        if (FAILED(hr)) return false;
        hr = device_->CreateRenderTargetView(backBuffer.Get(), nullptr, &rtv_);
        if (FAILED(hr)) return false;

        D3D11_TEXTURE2D_DESC sceneDesc{};
        sceneDesc.Width = static_cast<UINT>(width_);
        sceneDesc.Height = static_cast<UINT>(height_);
        sceneDesc.MipLevels = 1;
        sceneDesc.ArraySize = 1;
        sceneDesc.Format = DXGI_FORMAT_R8G8B8A8_UNORM;
        sceneDesc.SampleDesc.Count = 1;
        sceneDesc.Usage = D3D11_USAGE_DEFAULT;
        sceneDesc.BindFlags = D3D11_BIND_RENDER_TARGET | D3D11_BIND_SHADER_RESOURCE;
        hr = device_->CreateTexture2D(&sceneDesc, nullptr, &sceneColor_);
        if (FAILED(hr)) return false;
        hr = device_->CreateRenderTargetView(sceneColor_.Get(), nullptr, &sceneColorRtv_);
        if (FAILED(hr)) return false;
        hr = device_->CreateShaderResourceView(sceneColor_.Get(), nullptr, &sceneColorSrv_);
        if (FAILED(hr)) return false;

        D3D11_TEXTURE2D_DESC dd{};
        dd.Width = static_cast<UINT>(width_);
        dd.Height = static_cast<UINT>(height_);
        dd.MipLevels = 1;
        dd.ArraySize = 1;
        dd.Format = DXGI_FORMAT_D24_UNORM_S8_UINT;
        dd.SampleDesc.Count = 1;
        dd.BindFlags = D3D11_BIND_DEPTH_STENCIL;
        hr = device_->CreateTexture2D(&dd, nullptr, &depth_);
        if (FAILED(hr)) return false;
        hr = device_->CreateDepthStencilView(depth_.Get(), nullptr, &dsv_);
        return SUCCEEDED(hr);
    }
