// D3D input layout creation.
// Included inside Renderer private section.

    bool CreateShaderInputLayouts(const ComPtr<ID3DBlob>& vs,
        const ComPtr<ID3DBlob>& instancedVs,
        const ComPtr<ID3DBlob>& overlayVs,
        const ComPtr<ID3DBlob>& texturedOverlayVs) {
        HRESULT hr = S_OK;
        D3D11_INPUT_ELEMENT_DESC desc[] = {
            {"POSITION", 0, DXGI_FORMAT_R32G32B32_FLOAT, 0, offsetof(Vertex, pos), D3D11_INPUT_PER_VERTEX_DATA, 0},
            {"NORMAL", 0, DXGI_FORMAT_R32G32B32_FLOAT, 0, offsetof(Vertex, normal), D3D11_INPUT_PER_VERTEX_DATA, 0},
            {"TANGENT", 0, DXGI_FORMAT_R32G32B32_FLOAT, 0, offsetof(Vertex, tangent), D3D11_INPUT_PER_VERTEX_DATA, 0},
            {"TEXCOORD", 0, DXGI_FORMAT_R32G32_FLOAT, 0, offsetof(Vertex, uv), D3D11_INPUT_PER_VERTEX_DATA, 0},
            {"TEXCOORD", 1, DXGI_FORMAT_R32_FLOAT, 0, offsetof(Vertex, material), D3D11_INPUT_PER_VERTEX_DATA, 0}
        };
        hr = d3dRuntime_.device->CreateInputLayout(desc, ARRAYSIZE(desc), vs->GetBufferPointer(), vs->GetBufferSize(), &inputLayouts_.inputLayout);
        if (FAILED(hr)) return false;
        D3D11_INPUT_ELEMENT_DESC instancedDesc[] = {
            {"POSITION", 0, DXGI_FORMAT_R32G32B32_FLOAT, 0, offsetof(Vertex, pos), D3D11_INPUT_PER_VERTEX_DATA, 0},
            {"NORMAL", 0, DXGI_FORMAT_R32G32B32_FLOAT, 0, offsetof(Vertex, normal), D3D11_INPUT_PER_VERTEX_DATA, 0},
            {"TANGENT", 0, DXGI_FORMAT_R32G32B32_FLOAT, 0, offsetof(Vertex, tangent), D3D11_INPUT_PER_VERTEX_DATA, 0},
            {"TEXCOORD", 0, DXGI_FORMAT_R32G32_FLOAT, 0, offsetof(Vertex, uv), D3D11_INPUT_PER_VERTEX_DATA, 0},
            {"TEXCOORD", 1, DXGI_FORMAT_R32_FLOAT, 0, offsetof(Vertex, material), D3D11_INPUT_PER_VERTEX_DATA, 0},
            {"TEXCOORD", 5, DXGI_FORMAT_R32G32B32A32_FLOAT, 1, offsetof(StaticInstanceData, axisXOriginX), D3D11_INPUT_PER_INSTANCE_DATA, 1},
            {"TEXCOORD", 6, DXGI_FORMAT_R32G32B32A32_FLOAT, 1, offsetof(StaticInstanceData, axisYOriginY), D3D11_INPUT_PER_INSTANCE_DATA, 1},
            {"TEXCOORD", 7, DXGI_FORMAT_R32G32B32A32_FLOAT, 1, offsetof(StaticInstanceData, axisZOriginZ), D3D11_INPUT_PER_INSTANCE_DATA, 1},
            {"TEXCOORD", 8, DXGI_FORMAT_R32G32B32A32_FLOAT, 1, offsetof(StaticInstanceData, materialOverrideVariant), D3D11_INPUT_PER_INSTANCE_DATA, 1}
        };
        hr = d3dRuntime_.device->CreateInputLayout(instancedDesc, ARRAYSIZE(instancedDesc),
            instancedVs->GetBufferPointer(), instancedVs->GetBufferSize(), &inputLayouts_.instancedInputLayout);
        if (FAILED(hr)) return false;
        D3D11_INPUT_ELEMENT_DESC overlayDesc[] = {
            {"POSITION", 0, DXGI_FORMAT_R32G32_FLOAT, 0, offsetof(OverlayVertex, pos), D3D11_INPUT_PER_VERTEX_DATA, 0},
            {"COLOR", 0, DXGI_FORMAT_R32G32B32A32_FLOAT, 0, offsetof(OverlayVertex, color), D3D11_INPUT_PER_VERTEX_DATA, 0}
        };
        hr = d3dRuntime_.device->CreateInputLayout(overlayDesc, ARRAYSIZE(overlayDesc), overlayVs->GetBufferPointer(), overlayVs->GetBufferSize(), &inputLayouts_.overlayInputLayout);
        if (FAILED(hr)) return false;
        D3D11_INPUT_ELEMENT_DESC texturedOverlayDesc[] = {
            {"POSITION", 0, DXGI_FORMAT_R32G32_FLOAT, 0, offsetof(TexturedOverlayVertex, pos), D3D11_INPUT_PER_VERTEX_DATA, 0},
            {"TEXCOORD", 0, DXGI_FORMAT_R32G32_FLOAT, 0, offsetof(TexturedOverlayVertex, uv), D3D11_INPUT_PER_VERTEX_DATA, 0},
            {"COLOR", 0, DXGI_FORMAT_R32G32B32A32_FLOAT, 0, offsetof(TexturedOverlayVertex, color), D3D11_INPUT_PER_VERTEX_DATA, 0}
        };
        hr = d3dRuntime_.device->CreateInputLayout(texturedOverlayDesc, ARRAYSIZE(texturedOverlayDesc),
            texturedOverlayVs->GetBufferPointer(), texturedOverlayVs->GetBufferSize(), &inputLayouts_.texturedOverlayInputLayout);
        return SUCCEEDED(hr);
    }
