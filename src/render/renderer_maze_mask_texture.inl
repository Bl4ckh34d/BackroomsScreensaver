    bool CreateMazeMaskTexture() {
        GameWorldRenderSnapshot world = gameWorld_.BuildRenderSnapshot();
        if (!d3dRuntime_.device || !world.maze || world.maze->w <= 0 || world.maze->h <= 0 || world.maze->open.empty()) return false;
        const Maze& maze = *world.maze;
        runtimeTextures_.mazeSrv.Reset();

        std::vector<uint8_t> mask(static_cast<size_t>(maze.w * maze.h), 0);
        for (int y = 0; y < maze.h; ++y) {
            for (int x = 0; x < maze.w; ++x) {
                uint8_t value = 0;
                if (maze.IsOpen(x, y)) {
                    value = 255;
                } else {
                    MazeWallFeature feature = maze.WallFeature(x, y);
                    if (feature == MazeWallFeature::Window) {
                        value = 255;
                    } else if (feature == MazeWallFeature::Tunnel) {
                        value = 128;
                    }
                }
                mask[static_cast<size_t>(y * maze.w + x)] = value;
            }
        }

        D3D11_TEXTURE2D_DESC td{};
        td.Width = static_cast<UINT>(maze.w);
        td.Height = static_cast<UINT>(maze.h);
        td.MipLevels = 1;
        td.ArraySize = 1;
        td.Format = DXGI_FORMAT_R8_UNORM;
        td.SampleDesc.Count = 1;
        td.Usage = D3D11_USAGE_IMMUTABLE;
        td.BindFlags = D3D11_BIND_SHADER_RESOURCE;

        D3D11_SUBRESOURCE_DATA init{};
        init.pSysMem = mask.data();
        init.SysMemPitch = static_cast<UINT>(maze.w);

        ComPtr<ID3D11Texture2D> tex;
        HRESULT hr = d3dRuntime_.device->CreateTexture2D(&td, &init, &tex);
        if (FAILED(hr)) return false;

        D3D11_SHADER_RESOURCE_VIEW_DESC sd{};
        sd.Format = td.Format;
        sd.ViewDimension = D3D11_SRV_DIMENSION_TEXTURE2D;
        sd.Texture2D.MostDetailedMip = 0;
        sd.Texture2D.MipLevels = 1;
        hr = d3dRuntime_.device->CreateShaderResourceView(tex.Get(), &sd, &runtimeTextures_.mazeSrv);
        return SUCCEEDED(hr);
    }
