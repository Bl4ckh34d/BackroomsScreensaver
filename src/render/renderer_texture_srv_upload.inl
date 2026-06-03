// Renderer texture runtime upload helpers.

    bool CreateTexture2DSrvRGBA(int size, const std::vector<uint8_t>& pixels, ComPtr<ID3D11ShaderResourceView>& srv) {
        if (size <= 0 || pixels.size() < static_cast<size_t>(size) * size * 4) return false;
        UINT mipLevels = 1;
        for (int s = size; s > 1; s >>= 1) {
            ++mipLevels;
        }

        D3D11_TEXTURE2D_DESC td{};
        td.Width = static_cast<UINT>(size);
        td.Height = static_cast<UINT>(size);
        td.MipLevels = mipLevels;
        td.ArraySize = 1;
        td.Format = DXGI_FORMAT_R8G8B8A8_UNORM;
        td.SampleDesc.Count = 1;
        td.Usage = D3D11_USAGE_DEFAULT;
        td.BindFlags = D3D11_BIND_SHADER_RESOURCE | D3D11_BIND_RENDER_TARGET;
        td.MiscFlags = D3D11_RESOURCE_MISC_GENERATE_MIPS;
        ComPtr<ID3D11Texture2D> tex;
        HRESULT hr = d3dRuntime_.device->CreateTexture2D(&td, nullptr, &tex);
        if (FAILED(hr)) return false;

        d3dRuntime_.context->UpdateSubresource(tex.Get(), 0, nullptr, pixels.data(), static_cast<UINT>(size * 4), 0);

        D3D11_SHADER_RESOURCE_VIEW_DESC sd{};
        sd.Format = td.Format;
        sd.ViewDimension = D3D11_SRV_DIMENSION_TEXTURE2D;
        sd.Texture2D.MostDetailedMip = 0;
        sd.Texture2D.MipLevels = mipLevels;
        hr = d3dRuntime_.device->CreateShaderResourceView(tex.Get(), &sd, &srv);
        if (FAILED(hr)) return false;
        d3dRuntime_.context->GenerateMips(srv.Get());
        return true;
    }
