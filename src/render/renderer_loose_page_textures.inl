// Renderer texture loose pages helpers.

    std::vector<std::filesystem::path> RandomLoosePageFiles() const {
        std::vector<std::filesystem::path> files;
        std::filesystem::path folder = ResolveConfiguredAssetPath(L"assets\\images\\randomPages");
        std::error_code ec;
        if (!std::filesystem::exists(folder, ec) || !std::filesystem::is_directory(folder, ec)) return files;
        for (const auto& entry : std::filesystem::directory_iterator(folder, ec)) {
            if (ec || !entry.is_regular_file(ec)) continue;
            std::wstring ext = entry.path().extension().wstring();
            std::transform(ext.begin(), ext.end(), ext.begin(), [](wchar_t c) { return static_cast<wchar_t>(std::towlower(c)); });
            if (ext == L".png" || ext == L".jpg" || ext == L".jpeg" || ext == L".bmp" || ext == L".tif" || ext == L".tiff") {
                files.push_back(entry.path());
            }
        }
        std::sort(files.begin(), files.end(), [](const std::filesystem::path& a, const std::filesystem::path& b) {
            return _wcsicmp(a.filename().c_str(), b.filename().c_str()) < 0;
        });
        return files;
    }

    bool CreateLoosePageTextureArray() {
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

        auto makeFallback = [&](std::vector<uint8_t>& pixels, int slot) {
            pixels.assign(static_cast<size_t>(pageW) * pageH * 4, 255);
            for (int y = 0; y < pageH; ++y) {
                float v = static_cast<float>(y) / static_cast<float>(pageH - 1);
                for (int x = 0; x < pageW; ++x) {
                    float u = static_cast<float>(x) / static_cast<float>(pageW - 1);
                    float grain = FractalNoise(u * 54.0f + slot * 0.73f, v * 72.0f + 11.0f, 481);
                    float edge = std::max(SmoothStep(0.032f, 0.0f, std::min(u, 1.0f - u)),
                        SmoothStep(0.032f, 0.0f, std::min(v, 1.0f - v)));
                    size_t dst = static_cast<size_t>((y * pageW + x) * 4);
                    pixels[dst + 0] = Byte(0.82f + grain * 0.075f - edge * 0.10f);
                    pixels[dst + 1] = Byte(0.80f + grain * 0.065f - edge * 0.09f);
                    pixels[dst + 2] = Byte(0.71f + grain * 0.050f - edge * 0.07f);
                    pixels[dst + 3] = 255;
                }
            }
        };

        auto blitFit = [&](const ImageRGBA& img, std::vector<uint8_t>& pixels) {
            if (!img.Valid()) return;
            float scale = std::min(static_cast<float>(pageW) / static_cast<float>(img.width),
                static_cast<float>(pageH) / static_cast<float>(img.height));
            int dstW = std::clamp(static_cast<int>(std::round(static_cast<float>(img.width) * scale)), 1, pageW);
            int dstH = std::clamp(static_cast<int>(std::round(static_cast<float>(img.height) * scale)), 1, pageH);
            int dstX0 = (pageW - dstW) / 2;
            int dstY0 = (pageH - dstH) / 2;
            for (int y = 0; y < dstH; ++y) {
                float sy = (static_cast<float>(y) + 0.5f) * static_cast<float>(img.height) / static_cast<float>(dstH) - 0.5f;
                int y0 = std::clamp(static_cast<int>(std::floor(sy)), 0, img.height - 1);
                int y1 = std::clamp(y0 + 1, 0, img.height - 1);
                float fy = sy - static_cast<float>(y0);
                for (int x = 0; x < dstW; ++x) {
                    float sx = (static_cast<float>(x) + 0.5f) * static_cast<float>(img.width) / static_cast<float>(dstW) - 0.5f;
                    int x0 = std::clamp(static_cast<int>(std::floor(sx)), 0, img.width - 1);
                    int x1 = std::clamp(x0 + 1, 0, img.width - 1);
                    float fx = sx - static_cast<float>(x0);
                    auto sample = [&](int px, int py, int c) {
                        return static_cast<float>(img.pixels[static_cast<size_t>((py * img.width + px) * 4 + c)]);
                    };
                    size_t dst = static_cast<size_t>(((dstY0 + y) * pageW + (dstX0 + x)) * 4);
                    float rgba[4]{};
                    for (int c = 0; c < 4; ++c) {
                        float a = Lerp(sample(x0, y0, c), sample(x1, y0, c), fx);
                        float b = Lerp(sample(x0, y1, c), sample(x1, y1, c), fx);
                        rgba[c] = Lerp(a, b, fy);
                    }
                    float alpha = Clamp01(rgba[3] / 255.0f);
                    for (int c = 0; c < 3; ++c) {
                        float bg = static_cast<float>(pixels[dst + c]);
                        pixels[dst + c] = static_cast<uint8_t>(std::clamp(static_cast<int>(std::round(Lerp(bg, rgba[c], alpha))), 0, 255));
                    }
                    pixels[dst + 3] = 255;
                }
            }
        };

        std::vector<std::filesystem::path> files = RandomLoosePageFiles();
        ScopedCom com;
        bool canLoadImages = com.Ok();
        std::vector<uint8_t> pixels;
        pixels.reserve(static_cast<size_t>(pageW) * pageH * 4);
        for (int slot = 0; slot < pageCount; ++slot) {
            makeFallback(pixels, slot);
            if (canLoadImages && slot < static_cast<int>(files.size())) {
                ImageRGBA img;
                if (LoadImageWic(files[static_cast<size_t>(slot)], 0, 0, img)) {
                    blitFit(img, pixels);
                }
            }
            UINT subresource = D3D11CalcSubresource(0, static_cast<UINT>(slot), mipLevels);
            d3dRuntime_.context->UpdateSubresource(tex.Get(), subresource, nullptr, pixels.data(), pageW * 4, 0);
        }

        D3D11_SHADER_RESOURCE_VIEW_DESC sd{};
        sd.Format = td.Format;
        sd.ViewDimension = D3D11_SRV_DIMENSION_TEXTURE2DARRAY;
        sd.Texture2DArray.MostDetailedMip = 0;
        sd.Texture2DArray.MipLevels = mipLevels;
        sd.Texture2DArray.FirstArraySlice = 0;
        sd.Texture2DArray.ArraySize = pageCount;
        if (FAILED(d3dRuntime_.device->CreateShaderResourceView(tex.Get(), &sd, &runtimeTextures_.loosePagesSrv))) return false;
        d3dRuntime_.context->GenerateMips(runtimeTextures_.loosePagesSrv.Get());
        return true;
    }
