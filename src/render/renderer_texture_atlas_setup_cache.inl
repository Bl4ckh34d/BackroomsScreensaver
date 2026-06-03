        StartupProfile profile(L"CreateTextures");
        const int width = kTextureSize;
        const int height = kTextureSize * kMaterialCount;
        std::vector<uint8_t> albedo(static_cast<size_t>(width * height * 4), 255);
        std::vector<uint8_t> normal(static_cast<size_t>(width * height * 4), 255);
        std::vector<uint8_t> props(static_cast<size_t>(width * height * 4), 255);
        profile.Mark(L"AllocateBaseArrays");

        auto makeSrv = [&](const std::vector<uint8_t>& pixels, ComPtr<ID3D11ShaderResourceView>& srv) {
            UINT mipLevels = 1;
            for (int s = kTextureSize; s > 1; s >>= 1) {
                ++mipLevels;
            }

            D3D11_TEXTURE2D_DESC td{};
            td.Width = width;
            td.Height = kTextureSize;
            td.MipLevels = mipLevels;
            td.ArraySize = kMaterialCount;
            td.Format = DXGI_FORMAT_R8G8B8A8_UNORM;
            td.SampleDesc.Count = 1;
            td.Usage = D3D11_USAGE_DEFAULT;
            td.BindFlags = D3D11_BIND_SHADER_RESOURCE | D3D11_BIND_RENDER_TARGET;
            td.MiscFlags = D3D11_RESOURCE_MISC_GENERATE_MIPS;
            ComPtr<ID3D11Texture2D> tex;
            HRESULT hr = d3dRuntime_.device->CreateTexture2D(&td, nullptr, &tex);
            if (FAILED(hr)) return false;
            for (UINT slice = 0; slice < kMaterialCount; ++slice) {
                const uint8_t* src = pixels.data() + static_cast<size_t>(slice) * kTextureSize * width * 4;
                UINT subresource = D3D11CalcSubresource(0, slice, mipLevels);
                d3dRuntime_.context->UpdateSubresource(tex.Get(), subresource, nullptr, src, width * 4, 0);
            }

            D3D11_SHADER_RESOURCE_VIEW_DESC sd{};
            sd.Format = td.Format;
            sd.ViewDimension = D3D11_SRV_DIMENSION_TEXTURE2DARRAY;
            sd.Texture2DArray.MostDetailedMip = 0;
            sd.Texture2DArray.MipLevels = mipLevels;
            sd.Texture2DArray.FirstArraySlice = 0;
            sd.Texture2DArray.ArraySize = kMaterialCount;
            hr = d3dRuntime_.device->CreateShaderResourceView(tex.Get(), &sd, &srv);
            if (FAILED(hr)) return false;
            d3dRuntime_.context->GenerateMips(srv.Get());
            return true;
        };

        uint64_t textureHash = TextureCacheHash();
        if (LoadTextureCache(textureHash, albedo, normal, props)) {
            profile.Mark(L"LoadTextureCache");
            ReportStartupSubStep(L"Loading textures", L"Loaded cached material atlas. Creating GPU texture views.", 2);
            if (!makeSrv(albedo, materialTextures_.albedoSrv)) return false;
            profile.Mark(L"CreateAlbedoSRV");
            if (!makeSrv(normal, materialTextures_.normalSrv)) return false;
            profile.Mark(L"CreateNormalSRV");
            if (!makeSrv(props, materialTextures_.materialPropsSrv)) return false;
            profile.Mark(L"CreateMaterialPropsSRV");
            if (!CreateHighResCeilingTextures()) return false;
            profile.Mark(L"CreateHighResCeilingSRVs");
            if (!CreateHighResDoorTextures()) return false;
            profile.Mark(L"CreateHighResDoorSRVs");
            if (!CreateLoosePageTextureArray()) return false;
            profile.Mark(L"CreateLoosePagesSRV");
            return true;
        }
        profile.Mark(L"TextureCacheMiss");
        ReportStartupSubStep(L"Loading textures", L"Texture cache miss. Generating material atlas.", 1);
