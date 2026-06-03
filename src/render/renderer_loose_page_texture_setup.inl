        constexpr int pageW = kLoosePageTextureWidth;
        constexpr int pageH = kLoosePageTextureHeight;
        constexpr int pageCount = kRandomLoosePageAtlasSlots;
        UINT mipLevels = 1;
        for (int s = std::max(pageW, pageH); s > 1; s >>= 1) {
            ++mipLevels;
        }

        D3D11_TEXTURE2D_DESC td{};
        td.Width = pageW;
        td.Height = pageH;
        td.MipLevels = mipLevels;
        td.ArraySize = pageCount;
        td.Format = DXGI_FORMAT_R8G8B8A8_UNORM;
        td.SampleDesc.Count = 1;
        td.Usage = D3D11_USAGE_DEFAULT;
        td.BindFlags = D3D11_BIND_SHADER_RESOURCE | D3D11_BIND_RENDER_TARGET;
        td.MiscFlags = D3D11_RESOURCE_MISC_GENERATE_MIPS;
        ComPtr<ID3D11Texture2D> tex;
        if (FAILED(d3dRuntime_.device->CreateTexture2D(&td, nullptr, &tex))) return false;
