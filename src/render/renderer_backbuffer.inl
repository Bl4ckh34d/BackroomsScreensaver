    bool CreateBackBuffer() {
        ComPtr<ID3D11Texture2D> backBuffer;
        HRESULT hr = d3dRuntime_.swapChain->GetBuffer(0, IID_PPV_ARGS(&backBuffer));
        if (FAILED(hr)) return false;
        hr = d3dRuntime_.device->CreateRenderTargetView(backBuffer.Get(), nullptr, &renderTargetRuntime_.rtv);
        if (FAILED(hr)) return false;

        D3D11_TEXTURE2D_DESC sceneDesc{};
        sceneDesc.Width = static_cast<UINT>(hostRuntime_.width);
        sceneDesc.Height = static_cast<UINT>(hostRuntime_.height);
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

        D3D11_TEXTURE2D_DESC dd{};
        dd.Width = static_cast<UINT>(hostRuntime_.width);
        dd.Height = static_cast<UINT>(hostRuntime_.height);
        dd.MipLevels = 1;
        dd.ArraySize = 1;
        dd.Format = DXGI_FORMAT_D24_UNORM_S8_UINT;
        dd.SampleDesc.Count = 1;
        dd.BindFlags = D3D11_BIND_DEPTH_STENCIL;
        hr = d3dRuntime_.device->CreateTexture2D(&dd, nullptr, &renderTargetRuntime_.depth);
        if (FAILED(hr)) return false;
        hr = d3dRuntime_.device->CreateDepthStencilView(renderTargetRuntime_.depth.Get(), nullptr, &renderTargetRuntime_.dsv);
        return SUCCEEDED(hr);
    }
