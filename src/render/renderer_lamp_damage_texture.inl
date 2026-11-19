    struct LampDamageTexel {
        uint32_t damage = 0;
        uint32_t addedDamage = 0;
        uint32_t damagedNeighbors[2]{};
    };

    std::vector<LampDamageTexel> BuildLampDamageTexels(const Maze& maze) const {
        constexpr int radius = 3;
        constexpr int diameter = radius * 2 + 1;
        const auto& damage = effectRuntime_.lampDamagePixels;
        const auto& baked = effectRuntime_.bakedLampDamageBaseline;
        const bool hasBaseline = baked.size() == damage.size();
        std::vector<LampDamageTexel> texels(damage.size());
        for (int y = 0; y < maze.h; ++y) {
            for (int x = 0; x < maze.w; ++x) {
                const size_t index = static_cast<size_t>(y * maze.w + x);
                LampDamageTexel& texel = texels[index];
                texel.damage = damage[index];
                const uint32_t oldDamage = hasBaseline ? baked[index] : 0;
                if (texel.damage <= oldDamage) continue;
                texel.addedDamage = texel.damage - oldDamage;
                // Each receiver stores the sparse subset of its 7x7 window it needs to scan.
                for (int ry = std::max(0, y - radius); ry <= std::min(maze.h - 1, y + radius); ++ry) {
                    for (int rx = std::max(0, x - radius); rx <= std::min(maze.w - 1, x + radius); ++rx) {
                        const int bit = (y - ry + radius) * diameter + x - rx + radius;
                        texels[static_cast<size_t>(ry * maze.w + rx)].damagedNeighbors[bit / 32] |= 1u << (bit % 32);
                    }
                }
            }
        }
        return texels;
    }

    bool CreateLampDamageTexture() {
        GameWorldRenderSnapshot world = gameWorld_.BuildRenderSnapshot();
        if (!d3dRuntime_.device || !world.maze || world.maze->w <= 0 || world.maze->h <= 0) return false;
        const Maze& maze = *world.maze;
        runtimeTextures_.lampDamageTexture.Reset();
        runtimeTextures_.lampDamageSrv.Reset();

        const size_t pixelCount = static_cast<size_t>(maze.w) * static_cast<size_t>(maze.h);
        if (effectRuntime_.lampDamagePixels.size() != pixelCount) {
            effectRuntime_.lampDamagePixels.assign(pixelCount, 0);
        }
        // The baked lightmap already accounts for these lamps. Only subsequent damage attenuates it.
        effectRuntime_.bakedLampDamageBaseline = effectRuntime_.lampDamagePixels;
        const auto texels = BuildLampDamageTexels(maze);

        D3D11_TEXTURE2D_DESC td{};
        td.Width = static_cast<UINT>(maze.w);
        td.Height = static_cast<UINT>(maze.h);
        td.MipLevels = 1;
        td.ArraySize = 1;
        td.Format = DXGI_FORMAT_R32G32B32A32_UINT;
        td.SampleDesc.Count = 1;
        td.Usage = D3D11_USAGE_DEFAULT;
        td.BindFlags = D3D11_BIND_SHADER_RESOURCE;

        D3D11_SUBRESOURCE_DATA init{};
        init.pSysMem = texels.data();
        init.SysMemPitch = static_cast<UINT>(maze.w * sizeof(LampDamageTexel));

        HRESULT hr = d3dRuntime_.device->CreateTexture2D(&td, &init, &runtimeTextures_.lampDamageTexture);
        if (FAILED(hr)) return false;

        D3D11_SHADER_RESOURCE_VIEW_DESC sd{};
        sd.Format = td.Format;
        sd.ViewDimension = D3D11_SRV_DIMENSION_TEXTURE2D;
        sd.Texture2D.MostDetailedMip = 0;
        sd.Texture2D.MipLevels = 1;
        hr = d3dRuntime_.device->CreateShaderResourceView(runtimeTextures_.lampDamageTexture.Get(), &sd, &runtimeTextures_.lampDamageSrv);
        if (FAILED(hr)) return false;
        effectRuntime_.lampDamageDirty = false;
        return true;
    }
