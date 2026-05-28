    bool CreateFlashlightPatternTexture() {
        if (!device_) return false;
        constexpr int kPatternSize = 512;
        ImageRGBA img;
        bool loaded = false;
        {
            ScopedCom com;
            if (com.Ok()) {
                loaded = LoadImageWic(ResolveConfiguredAssetPath(L"assets\\PBRs\\downloads\\t_flashlightpattern.png"),
                    kPatternSize, kPatternSize, img);
            }
        }
        if (!loaded) {
            img.width = kPatternSize;
            img.height = kPatternSize;
            img.pixels.assign(static_cast<size_t>(kPatternSize * kPatternSize * 4), 255);
        }

        D3D11_TEXTURE2D_DESC td{};
        td.Width = static_cast<UINT>(img.width);
        td.Height = static_cast<UINT>(img.height);
        td.MipLevels = 1;
        td.ArraySize = 1;
        td.Format = DXGI_FORMAT_R8G8B8A8_UNORM;
        td.SampleDesc.Count = 1;
        td.Usage = D3D11_USAGE_IMMUTABLE;
        td.BindFlags = D3D11_BIND_SHADER_RESOURCE;

        D3D11_SUBRESOURCE_DATA init{};
        init.pSysMem = img.pixels.data();
        init.SysMemPitch = static_cast<UINT>(img.width * 4);

        ComPtr<ID3D11Texture2D> tex;
        HRESULT hr = device_->CreateTexture2D(&td, &init, &tex);
        if (FAILED(hr)) return false;

        D3D11_SHADER_RESOURCE_VIEW_DESC sd{};
        sd.Format = td.Format;
        sd.ViewDimension = D3D11_SRV_DIMENSION_TEXTURE2D;
        sd.Texture2D.MostDetailedMip = 0;
        sd.Texture2D.MipLevels = 1;
        hr = device_->CreateShaderResourceView(tex.Get(), &sd, &flashlightPatternSrv_);
        return SUCCEEDED(hr);
    }

    bool CreateMazeMaskTexture() {
        if (!device_ || maze_.w <= 0 || maze_.h <= 0 || maze_.open.empty()) return false;
        mazeSrv_.Reset();

        std::vector<uint8_t> mask(static_cast<size_t>(maze_.w * maze_.h), 0);
        for (int y = 0; y < maze_.h; ++y) {
            for (int x = 0; x < maze_.w; ++x) {
                mask[static_cast<size_t>(y * maze_.w + x)] = maze_.IsOpen(x, y) ? 255 : 0;
            }
        }

        D3D11_TEXTURE2D_DESC td{};
        td.Width = static_cast<UINT>(maze_.w);
        td.Height = static_cast<UINT>(maze_.h);
        td.MipLevels = 1;
        td.ArraySize = 1;
        td.Format = DXGI_FORMAT_R8_UNORM;
        td.SampleDesc.Count = 1;
        td.Usage = D3D11_USAGE_IMMUTABLE;
        td.BindFlags = D3D11_BIND_SHADER_RESOURCE;

        D3D11_SUBRESOURCE_DATA init{};
        init.pSysMem = mask.data();
        init.SysMemPitch = static_cast<UINT>(maze_.w);

        ComPtr<ID3D11Texture2D> tex;
        HRESULT hr = device_->CreateTexture2D(&td, &init, &tex);
        if (FAILED(hr)) return false;

        D3D11_SHADER_RESOURCE_VIEW_DESC sd{};
        sd.Format = td.Format;
        sd.ViewDimension = D3D11_SRV_DIMENSION_TEXTURE2D;
        sd.Texture2D.MostDetailedMip = 0;
        sd.Texture2D.MipLevels = 1;
        hr = device_->CreateShaderResourceView(tex.Get(), &sd, &mazeSrv_);
        return SUCCEEDED(hr);
    }

    bool CreateLampDamageTexture() {
        if (!device_ || maze_.w <= 0 || maze_.h <= 0) return false;
        lampDamageTexture_.Reset();
        lampDamageSrv_.Reset();

        const size_t pixelCount = static_cast<size_t>(maze_.w) * static_cast<size_t>(maze_.h);
        if (lampDamagePixels_.size() != pixelCount) {
            lampDamagePixels_.assign(pixelCount, 0);
        }

        D3D11_TEXTURE2D_DESC td{};
        td.Width = static_cast<UINT>(maze_.w);
        td.Height = static_cast<UINT>(maze_.h);
        td.MipLevels = 1;
        td.ArraySize = 1;
        td.Format = DXGI_FORMAT_R8_UNORM;
        td.SampleDesc.Count = 1;
        td.Usage = D3D11_USAGE_DEFAULT;
        td.BindFlags = D3D11_BIND_SHADER_RESOURCE;

        D3D11_SUBRESOURCE_DATA init{};
        init.pSysMem = lampDamagePixels_.data();
        init.SysMemPitch = static_cast<UINT>(maze_.w);

        HRESULT hr = device_->CreateTexture2D(&td, &init, &lampDamageTexture_);
        if (FAILED(hr)) return false;

        D3D11_SHADER_RESOURCE_VIEW_DESC sd{};
        sd.Format = td.Format;
        sd.ViewDimension = D3D11_SRV_DIMENSION_TEXTURE2D;
        sd.Texture2D.MostDetailedMip = 0;
        sd.Texture2D.MipLevels = 1;
        hr = device_->CreateShaderResourceView(lampDamageTexture_.Get(), &sd, &lampDamageSrv_);
        if (FAILED(hr)) return false;
        lampDamageDirty_ = false;
        return true;
    }
