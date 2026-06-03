    bool CreateConstantBuffer() {
        D3D11_BUFFER_DESC bd{};
        bd.ByteWidth = sizeof(SceneConstants);
        bd.Usage = D3D11_USAGE_DYNAMIC;
        bd.BindFlags = D3D11_BIND_CONSTANT_BUFFER;
        bd.CPUAccessFlags = D3D11_CPU_ACCESS_WRITE;
        return SUCCEEDED(d3dRuntime_.device->CreateBuffer(&bd, nullptr, &renderBuffers_.constantBuffer));
    }
