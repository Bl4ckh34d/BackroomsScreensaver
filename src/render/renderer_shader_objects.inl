// D3D shader object creation.
// Included inside Renderer private section.

    bool CreateShaderObjects(const ComPtr<ID3DBlob>& vs,
        const ComPtr<ID3DBlob>& instancedVs,
        const ComPtr<ID3DBlob>& hs,
        const ComPtr<ID3DBlob>& ds,
        const ComPtr<ID3DBlob>& ps,
        const ComPtr<ID3DBlob>& liquidPs,
        const ComPtr<ID3DBlob>& shadowPs,
        const ComPtr<ID3DBlob>& overlayVs,
        const ComPtr<ID3DBlob>& overlayPs,
        const ComPtr<ID3DBlob>& texturedOverlayVs,
        const ComPtr<ID3DBlob>& texturedOverlayPs,
        const ComPtr<ID3DBlob>& postVs,
        const ComPtr<ID3DBlob>& postPs) {
        HRESULT hr = d3dRuntime_.device->CreateVertexShader(vs->GetBufferPointer(), vs->GetBufferSize(), nullptr, &shaders_.vertexShader);
        if (FAILED(hr)) return false;
        hr = d3dRuntime_.device->CreateVertexShader(instancedVs->GetBufferPointer(), instancedVs->GetBufferSize(), nullptr, &shaders_.instancedVertexShader);
        if (FAILED(hr)) return false;
        if (d3dRuntime_.featureLevel >= D3D_FEATURE_LEVEL_11_0) {
            hr = d3dRuntime_.device->CreateHullShader(hs->GetBufferPointer(), hs->GetBufferSize(), nullptr, &shaders_.hullShader);
            if (FAILED(hr)) return false;
            hr = d3dRuntime_.device->CreateDomainShader(ds->GetBufferPointer(), ds->GetBufferSize(), nullptr, &shaders_.domainShader);
            if (FAILED(hr)) return false;
        }
        hr = d3dRuntime_.device->CreatePixelShader(ps->GetBufferPointer(), ps->GetBufferSize(), nullptr, &shaders_.pixelShader);
        if (FAILED(hr)) return false;
        hr = d3dRuntime_.device->CreatePixelShader(liquidPs->GetBufferPointer(), liquidPs->GetBufferSize(), nullptr, &shaders_.liquidPixelShader);
        if (FAILED(hr)) return false;
        hr = d3dRuntime_.device->CreatePixelShader(shadowPs->GetBufferPointer(), shadowPs->GetBufferSize(), nullptr, &shaders_.shadowPixelShader);
        if (FAILED(hr)) return false;
        hr = d3dRuntime_.device->CreateVertexShader(overlayVs->GetBufferPointer(), overlayVs->GetBufferSize(), nullptr, &shaders_.overlayVertexShader);
        if (FAILED(hr)) return false;
        hr = d3dRuntime_.device->CreatePixelShader(overlayPs->GetBufferPointer(), overlayPs->GetBufferSize(), nullptr, &shaders_.overlayPixelShader);
        if (FAILED(hr)) return false;
        hr = d3dRuntime_.device->CreateVertexShader(texturedOverlayVs->GetBufferPointer(), texturedOverlayVs->GetBufferSize(), nullptr, &shaders_.texturedOverlayVertexShader);
        if (FAILED(hr)) return false;
        hr = d3dRuntime_.device->CreatePixelShader(texturedOverlayPs->GetBufferPointer(), texturedOverlayPs->GetBufferSize(), nullptr, &shaders_.texturedOverlayPixelShader);
        if (FAILED(hr)) return false;
        hr = d3dRuntime_.device->CreateVertexShader(postVs->GetBufferPointer(), postVs->GetBufferSize(), nullptr, &shaders_.postVertexShader);
        if (FAILED(hr)) return false;
        hr = d3dRuntime_.device->CreatePixelShader(postPs->GetBufferPointer(), postPs->GetBufferSize(), nullptr, &shaders_.postPixelShader);
        if (FAILED(hr)) return false;
        return true;
    }
