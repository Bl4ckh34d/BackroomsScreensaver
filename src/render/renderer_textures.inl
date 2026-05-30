// Renderer material atlas generation, external PBR loading, and texture-cache upload.
// Included inside Renderer private section before flashlight-pattern texture creation.

    bool CreateTextures() {
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
            HRESULT hr = device_->CreateTexture2D(&td, nullptr, &tex);
            if (FAILED(hr)) return false;
            for (UINT slice = 0; slice < kMaterialCount; ++slice) {
                const uint8_t* src = pixels.data() + static_cast<size_t>(slice) * kTextureSize * width * 4;
                UINT subresource = D3D11CalcSubresource(0, slice, mipLevels);
                context_->UpdateSubresource(tex.Get(), subresource, nullptr, src, width * 4, 0);
            }

            D3D11_SHADER_RESOURCE_VIEW_DESC sd{};
            sd.Format = td.Format;
            sd.ViewDimension = D3D11_SRV_DIMENSION_TEXTURE2DARRAY;
            sd.Texture2DArray.MostDetailedMip = 0;
            sd.Texture2DArray.MipLevels = mipLevels;
            sd.Texture2DArray.FirstArraySlice = 0;
            sd.Texture2DArray.ArraySize = kMaterialCount;
            hr = device_->CreateShaderResourceView(tex.Get(), &sd, &srv);
            if (FAILED(hr)) return false;
            context_->GenerateMips(srv.Get());
            return true;
        };

        uint64_t textureHash = TextureCacheHash();
        if (LoadTextureCache(textureHash, albedo, normal, props)) {
            profile.Mark(L"LoadTextureCache");
            ReportStartupSubStep(L"Loading textures", L"Loaded cached material atlas. Creating GPU texture views.", 2);
            if (!makeSrv(albedo, albedoSrv_)) return false;
            profile.Mark(L"CreateAlbedoSRV");
            if (!makeSrv(normal, normalSrv_)) return false;
            profile.Mark(L"CreateNormalSRV");
            if (!makeSrv(props, materialPropsSrv_)) return false;
            profile.Mark(L"CreateMaterialPropsSRV");
            return true;
        }
        profile.Mark(L"TextureCacheMiss");
        ReportStartupSubStep(L"Loading textures", L"Texture cache miss. Generating material atlas.", 1);

        std::vector<float> heights(static_cast<size_t>(width * height), 0.5f);
        std::vector<uint8_t> externalNormal(static_cast<size_t>(width * height * 4), 0);
        std::vector<uint8_t> hasExternalNormal(static_cast<size_t>(width * height), 0);
        for (int m = 0; m < kMaterialCount; ++m) {
            for (int y = 0; y < kTextureSize; ++y) {
                int gy = m * kTextureSize + y;
                for (int x = 0; x < kTextureSize; ++x) {
                    size_t i = static_cast<size_t>((gy * width + x) * 4);
                    props[i + 0] = 255; // AO
                    props[i + 1] = 178; // roughness
                    props[i + 2] = 0;
                    props[i + 3] = 255;
                }
            }
        }
        profile.Mark(L"AllocateWorkArrays");

        auto setPixel = [&](int material, int x, int y, float r, float g, float b, float a, float h) {
            int gy = material * kTextureSize + y;
            size_t i = static_cast<size_t>((gy * width + x) * 4);
            albedo[i + 0] = Byte(r);
            albedo[i + 1] = Byte(g);
            albedo[i + 2] = Byte(b);
            albedo[i + 3] = Byte(a);
            heights[static_cast<size_t>(gy * width + x)] = Clamp01(h);
        };

        auto applyAlbedo = [&](int material, const ImageRGBA& img) {
            if (!img.Valid()) return;
            for (int y = 0; y < kTextureSize; ++y) {
                int gy = material * kTextureSize + y;
                for (int x = 0; x < kTextureSize; ++x) {
                    size_t src = static_cast<size_t>((y * img.width + x) * 4);
                    size_t dst = static_cast<size_t>((gy * width + x) * 4);
                    albedo[dst + 0] = img.pixels[src + 0];
                    albedo[dst + 1] = img.pixels[src + 1];
                    albedo[dst + 2] = img.pixels[src + 2];
                    albedo[dst + 3] = img.pixels[src + 3];
                }
            }
        };

        auto applyHeight = [&](int material, const ImageRGBA& img) {
            if (!img.Valid()) return;
            for (int y = 0; y < kTextureSize; ++y) {
                int gy = material * kTextureSize + y;
                for (int x = 0; x < kTextureSize; ++x) {
                    size_t src = static_cast<size_t>((y * img.width + x) * 4);
                    float h = img.pixels[src] / 255.0f;
                    heights[static_cast<size_t>(gy * width + x)] = h;
                }
            }
        };

        auto applyNormal = [&](int material, const ImageRGBA& img) {
            if (!img.Valid()) return;
            for (int y = 0; y < kTextureSize; ++y) {
                int gy = material * kTextureSize + y;
                for (int x = 0; x < kTextureSize; ++x) {
                    size_t src = static_cast<size_t>((y * img.width + x) * 4);
                    size_t dst = static_cast<size_t>((gy * width + x) * 4);
                    externalNormal[dst + 0] = img.pixels[src + 0];
                    externalNormal[dst + 1] = img.pixels[src + 1];
                    externalNormal[dst + 2] = img.pixels[src + 2];
                    externalNormal[dst + 3] = 255;
                    hasExternalNormal[static_cast<size_t>(gy * width + x)] = 1;
                }
            }
        };

        auto applyScalarProp = [&](int material, const ImageRGBA& img, int channel) {
            if (!img.Valid() || channel < 0 || channel > 3) return;
            for (int y = 0; y < kTextureSize; ++y) {
                int gy = material * kTextureSize + y;
                for (int x = 0; x < kTextureSize; ++x) {
                    size_t src = static_cast<size_t>((y * img.width + x) * 4);
                    size_t dst = static_cast<size_t>((gy * width + x) * 4);
                    props[dst + channel] = img.pixels[src];
                }
            }
        };

        auto loadPbrMaterial = [&](int material, const wchar_t* stem) {
            ImageRGBA img;
            std::wstring base(stem);
            if (base.empty()) return;
            if (LoadImageWic(ResolveAsset(settings_, base + L"_color_4k.jpg"), kTextureSize, kTextureSize, img)) {
                applyAlbedo(material, img);
            }
            if (LoadImageWic(ResolveAsset(settings_, base + L"_height_4k.png"), kTextureSize, kTextureSize, img)) {
                applyHeight(material, img);
            }
            std::filesystem::path normalPath = ResolveAsset(settings_, base + L"_normal_directx_4k.png");
            std::error_code ec;
            uintmax_t normalSize = std::filesystem::exists(normalPath, ec) ? std::filesystem::file_size(normalPath, ec) : 0;
            if (settings_.useExternalNormals && normalSize > 0 &&
                (material == 15 || normalSize <= static_cast<uintmax_t>(settings_.maxNormalMapMB) * 1024ull * 1024ull) &&
                LoadImageWic(normalPath, kTextureSize, kTextureSize, img)) {
                applyNormal(material, img);
            }
            if (LoadImageWic(ResolveAsset(settings_, base + L"_ao_4k.jpg"), kTextureSize, kTextureSize, img)) {
                applyScalarProp(material, img, 0);
            }
            if (LoadImageWic(ResolveAsset(settings_, base + L"_roughness_4k.jpg"), kTextureSize, kTextureSize, img)) {
                applyScalarProp(material, img, 1);
            }
        };

        ReportStartupSubStep(L"Loading textures", L"Generating procedural material atlas.", 1);
        for (int y = 0; y < kTextureSize; ++y) {
            for (int x = 0; x < kTextureSize; ++x) {
                float u = static_cast<float>(x) / kTextureSize;
                float v = static_cast<float>(y) / kTextureSize;
                float n1 = FractalNoise(u * 8.0f, v * 8.0f, 13);
                float stains = FractalNoise(u * 2.0f + 31.0f, v * 3.0f, 33);
                float seam = std::min(std::abs(std::fmod(u * 4.0f, 1.0f) - 0.02f), std::abs(std::fmod(v * 2.0f, 1.0f) - 0.02f));
                float groove = SmoothStep(0.035f, 0.0f, seam);
                float grime = SmoothStep(0.4f, 1.0f, v) * 0.18f + SmoothStep(0.72f, 0.95f, stains) * 0.23f;
                float pattern = std::sin(u * 140.0f) * std::sin(v * 90.0f) * 0.015f;
                setPixel(0, x, y,
                    0.82f + pattern - grime * 0.82f + n1 * 0.055f,
                    0.68f + pattern - grime * 0.72f + n1 * 0.045f,
                    0.34f + pattern - grime * 0.42f + n1 * 0.018f,
                    1.0f,
                    0.54f - groove * 0.23f + n1 * 0.12f);

                float carpet = FractalNoise(u * 28.0f, v * 28.0f, 71);
                float tileGroove = std::max(SmoothStep(0.025f, 0.0f, std::abs(std::fmod(u * 4.0f, 1.0f) - 0.02f)),
                                            SmoothStep(0.025f, 0.0f, std::abs(std::fmod(v * 4.0f, 1.0f) - 0.02f)));
                float damp = SmoothStep(0.65f, 0.95f, FractalNoise(u * 3.0f + 11.0f, v * 3.0f + 19.0f, 92));
                setPixel(1, x, y,
                    0.60f + carpet * 0.11f - damp * 0.13f,
                    0.52f + carpet * 0.09f - damp * 0.10f,
                    0.30f + carpet * 0.055f - damp * 0.075f,
                    1.0f,
                    0.45f + carpet * 0.18f - tileGroove * 0.30f);

                float panelX = std::abs(std::fmod(u * 2.0f, 1.0f) - 0.01f);
                float panelY = std::abs(std::fmod(v * 2.0f, 1.0f) - 0.01f);
                float grid = std::max(SmoothStep(0.03f, 0.0f, panelX), SmoothStep(0.03f, 0.0f, panelY));
                float strip = SmoothStep(0.035f, 0.0f, std::abs(std::fmod(u * 2.0f, 1.0f) - 0.5f)) *
                              SmoothStep(0.35f, 0.05f, std::abs(std::fmod(v * 2.0f, 1.0f) - 0.5f));
                float speck = FractalNoise(u * 60.0f, v * 60.0f, 44);
                setPixel(2, x, y,
                    0.76f + strip * 0.20f - grid * 0.13f + speck * 0.030f,
                    0.64f + strip * 0.18f - grid * 0.12f + speck * 0.026f,
                    0.34f + strip * 0.10f - grid * 0.08f + speck * 0.018f,
                    1.0f,
                    0.48f - grid * 0.28f + speck * 0.08f);

                float edge = std::max(SmoothStep(0.055f, 0.0f, std::min(u, 1.0f - u)),
                                      SmoothStep(0.055f, 0.0f, std::min(v, 1.0f - v)));
                float lens = SmoothStep(0.42f, 0.0f, std::abs(v - 0.5f)) * SmoothStep(0.46f, 0.0f, std::abs(u - 0.5f));
                setPixel(3, x, y,
                    0.72f + lens * 0.24f - edge * 0.18f,
                    0.76f + lens * 0.23f - edge * 0.16f,
                    0.70f + lens * 0.20f - edge * 0.12f,
                    1.0f,
                    0.5f);
                setPixel(5, x, y,
                    0.018f + lens * 0.012f,
                    0.018f + lens * 0.012f,
                    0.016f + lens * 0.010f,
                    1.0f,
                    0.5f);

                auto lineMask = [&](float value, float center, float width) {
                    return SmoothStep(width, 0.0f, std::abs(value - center));
                };
                float torso = SmoothStep(1.0f, 0.64f, ((u - 0.50f) / 0.135f) * ((u - 0.50f) / 0.135f) + ((v - 0.57f) / 0.35f) * ((v - 0.57f) / 0.35f));
                float waist = SmoothStep(1.0f, 0.70f, ((u - 0.50f) / 0.085f) * ((u - 0.50f) / 0.085f) + ((v - 0.77f) / 0.25f) * ((v - 0.77f) / 0.25f));
                float head = SmoothStep(1.0f, 0.63f, ((u - 0.5f) / 0.135f) * ((u - 0.5f) / 0.135f) + ((v - 0.245f) / 0.145f) * ((v - 0.245f) / 0.145f));
                float neck = lineMask(u, 0.50f, 0.055f) * SmoothStep(0.31f, 0.48f, v) * (1.0f - SmoothStep(0.52f, 0.60f, v));
                float armL = lineMask(u, 0.34f + (v - 0.34f) * 0.18f, 0.035f) * SmoothStep(0.30f, 0.56f, v) * (1.0f - SmoothStep(0.83f, 0.93f, v));
                float armR = lineMask(u, 0.66f - (v - 0.34f) * 0.18f, 0.035f) * SmoothStep(0.30f, 0.56f, v) * (1.0f - SmoothStep(0.83f, 0.93f, v));
                float clawL = lineMask(u, 0.29f, 0.030f) * SmoothStep(0.76f, 0.88f, v) * (1.0f - SmoothStep(0.90f, 0.98f, v));
                float clawR = lineMask(u, 0.71f, 0.030f) * SmoothStep(0.76f, 0.88f, v) * (1.0f - SmoothStep(0.90f, 0.98f, v));
                float legL = lineMask(u, 0.44f - (v - 0.74f) * 0.09f, 0.040f) * SmoothStep(0.68f, 0.82f, v);
                float legR = lineMask(u, 0.56f + (v - 0.74f) * 0.09f, 0.040f) * SmoothStep(0.68f, 0.82f, v);
                float antlerL = lineMask(u, 0.43f - (0.22f - v) * 0.95f, 0.026f) * SmoothStep(0.06f, 0.16f, v) * (1.0f - SmoothStep(0.22f, 0.29f, v));
                float antlerR = lineMask(u, 0.57f + (0.22f - v) * 0.95f, 0.026f) * SmoothStep(0.06f, 0.16f, v) * (1.0f - SmoothStep(0.22f, 0.29f, v));
                float tineL = lineMask(u, 0.31f, 0.020f) * SmoothStep(0.07f, 0.14f, v) * (1.0f - SmoothStep(0.18f, 0.24f, v));
                float tineR = lineMask(u, 0.69f, 0.020f) * SmoothStep(0.07f, 0.14f, v) * (1.0f - SmoothStep(0.18f, 0.24f, v));
                float rib = std::max(std::max(lineMask(v, 0.50f, 0.012f), lineMask(v, 0.56f, 0.012f)), lineMask(v, 0.62f, 0.012f)) * lineMask(u, 0.50f, 0.13f);
                float skinNoise = FractalNoise(u * 18.0f, v * 24.0f, 213);
                float vein = SmoothStep(0.030f, 0.0f, std::abs(std::fmod(u * 9.0f + v * 2.7f, 1.0f) - 0.06f)) *
                    SmoothStep(0.46f, 0.92f, FractalNoise(u * 5.0f + 9.0f, v * 8.0f, 214));
                float scar = SmoothStep(0.018f, 0.0f, std::abs(std::fmod(u * 13.0f - v * 4.0f, 1.0f) - 0.04f));
                setPixel(4, x, y,
                    0.018f + skinNoise * 0.016f + vein * 0.018f + scar * 0.012f,
                    0.017f + skinNoise * 0.014f + vein * 0.014f + scar * 0.010f,
                    0.016f + skinNoise * 0.012f + vein * 0.010f + scar * 0.009f,
                    1.0f,
                    0.43f + skinNoise * 0.18f + scar * 0.12f);

                float doorPanel = std::max(
                    SmoothStep(0.025f, 0.0f, std::abs(u - 0.12f)),
                    SmoothStep(0.025f, 0.0f, std::abs(u - 0.88f)));
                doorPanel = std::max(doorPanel, std::max(
                    SmoothStep(0.025f, 0.0f, std::abs(v - 0.16f)),
                    SmoothStep(0.025f, 0.0f, std::abs(v - 0.84f))));
                float doorGrime = SmoothStep(0.62f, 0.96f, FractalNoise(u * 5.0f + 17.0f, v * 8.0f + 4.0f, 19));
                setPixel(6, x, y,
                    0.23f - doorPanel * 0.04f - doorGrime * 0.05f,
                    0.18f - doorPanel * 0.035f - doorGrime * 0.04f,
                    0.12f - doorPanel * 0.02f,
                    1.0f,
                    0.50f - doorPanel * 0.08f);

                setPixel(7, x, y,
                    0.02f,
                    0.42f,
                    0.12f,
                    1.0f,
                    0.5f);

                float plasticGrain = FractalNoise(u * 14.0f, v * 13.0f, 91);
                setPixel(8, x, y,
                    0.66f + plasticGrain * 0.065f,
                    0.61f + plasticGrain * 0.055f,
                    0.47f + plasticGrain * 0.040f,
                    1.0f,
                    0.50f + plasticGrain * 0.05f);

                float paperEdge = std::max(SmoothStep(0.035f, 0.0f, std::min(u, 1.0f - u)),
                                           SmoothStep(0.035f, 0.0f, std::min(v, 1.0f - v)));
                float paperStain = SmoothStep(0.62f, 0.95f, FractalNoise(u * 4.0f + 4.0f, v * 6.0f + 15.0f, 21));
                setPixel(9, x, y,
                    0.82f - paperEdge * 0.12f - paperStain * 0.22f,
                    0.80f - paperEdge * 0.11f - paperStain * 0.18f,
                    0.70f - paperEdge * 0.08f - paperStain * 0.13f,
                    1.0f,
                    0.50f);

                float scratch = std::max(SmoothStep(0.018f, 0.0f, std::abs(std::fmod(u * 19.0f + v * 3.0f, 1.0f) - 0.04f)),
                                         SmoothStep(0.014f, 0.0f, std::abs(std::fmod(v * 23.0f - u * 2.0f, 1.0f) - 0.06f)));
                float rust = SmoothStep(0.72f, 0.96f, FractalNoise(u * 5.0f + 19.0f, v * 6.0f - 7.0f, 118));
                setPixel(10, x, y,
                    0.055f + n1 * 0.025f + scratch * 0.040f + rust * 0.055f,
                    0.058f + n1 * 0.023f + scratch * 0.034f + rust * 0.025f,
                    0.062f + n1 * 0.020f + scratch * 0.030f - rust * 0.008f,
                    1.0f,
                    0.46f + n1 * 0.07f + scratch * 0.10f - rust * 0.08f);

                float wet = SmoothStep(0.35f, 0.0f, std::abs(u - 0.5f)) * SmoothStep(0.50f, 0.02f, std::abs(v - 0.5f));
                wet = std::max(wet, SmoothStep(0.78f, 1.0f, FractalNoise(u * 6.0f, v * 6.0f, 137)));
                setPixel(11, x, y,
                    0.020f + wet * 0.030f,
                    0.026f + wet * 0.036f,
                    0.024f + wet * 0.032f,
                    1.0f,
                    0.60f + wet * 0.15f);

                float eyeDx = (u - 0.5f) / 0.34f;
                float eyeDy = (v - 0.5f) / 0.26f;
                float eyeGlow = std::exp(-(eyeDx * eyeDx + eyeDy * eyeDy) * 3.3f);
                float hot = std::exp(-(eyeDx * eyeDx + eyeDy * eyeDy) * 28.0f);
                float ragged = 0.78f + FractalNoise(u * 11.0f, v * 9.0f, 222) * 0.22f;
                float eyeAlpha = Clamp01((eyeGlow * 0.98f + hot * 1.18f) * ragged);
                setPixel(12, x, y,
                    1.0f,
                    0.060f + hot * 0.26f,
                    0.020f,
                    eyeAlpha,
                    0.58f);
            }
        }
        profile.Mark(L"ProceduralMaterials");

        auto fillRuntimeMaterial = [&](int material, float r, float g, float b, float roughness) {
            if (material < 0 || material >= kMaterialCount) return;
            for (int y = 0; y < kTextureSize; ++y) {
                int gy = material * kTextureSize + y;
                for (int x = 0; x < kTextureSize; ++x) {
                    setPixel(material, x, y, r, g, b, 1.0f, 0.50f);
                    size_t i = static_cast<size_t>((gy * width + x) * 4);
                    props[i + 1] = Byte(roughness);
                }
            }
        };
        fillRuntimeMaterial(16, 0.62f, 0.58f, 0.50f, 0.62f);
        fillRuntimeMaterial(17, 0.13f, 0.145f, 0.15f, 0.72f);
        fillRuntimeMaterial(18, 0.012f, 0.012f, 0.012f, 0.58f);
        fillRuntimeMaterial(19, 0.70f, 0.68f, 0.62f, 0.50f);
        fillRuntimeMaterial(20, 0.29f, 0.50f, 0.64f, 0.82f);
        fillRuntimeMaterial(21, 0.78f, 0.78f, 0.74f, 0.42f);
        fillRuntimeMaterial(22, 0.38f, 0.36f, 0.34f, 0.62f);
        fillRuntimeMaterial(23, 0.035f, 0.034f, 0.031f, 0.70f);
        fillRuntimeMaterial(24, 0.78f, 0.74f, 0.62f, 0.58f);

        auto fillMenuLabelAtlas = [&](int material) {
            for (int y = 0; y < kTextureSize; ++y) {
                int gy = material * kTextureSize + y;
                for (int x = 0; x < kTextureSize; ++x) {
                    size_t i = static_cast<size_t>((gy * width + x) * 4);
                    albedo[i + 0] = 0;
                    albedo[i + 1] = 0;
                    albedo[i + 2] = 0;
                    albedo[i + 3] = 0;
                    heights[static_cast<size_t>(gy * width + x)] = 0.50f;
                    props[i + 0] = 255;
                    props[i + 1] = 40;
                    props[i + 2] = 0;
                    props[i + 3] = 255;
                }
            }

            std::vector<uint8_t> dib(static_cast<size_t>(kTextureSize) * kTextureSize * 4, 0);
            BITMAPINFO bmi{};
            bmi.bmiHeader.biSize = sizeof(BITMAPINFOHEADER);
            bmi.bmiHeader.biWidth = kTextureSize;
            bmi.bmiHeader.biHeight = -kTextureSize;
            bmi.bmiHeader.biPlanes = 1;
            bmi.bmiHeader.biBitCount = 32;
            bmi.bmiHeader.biCompression = BI_RGB;
            void* bits = nullptr;
            HDC screen = GetDC(nullptr);
            HDC dc = CreateCompatibleDC(screen);
            HBITMAP bitmap = CreateDIBSection(dc, &bmi, DIB_RGB_COLORS, &bits, nullptr, 0);
            if (bitmap && bits) {
                HGDIOBJ oldBitmap = SelectObject(dc, bitmap);
                std::memset(bits, 0, dib.size());
                SetBkMode(dc, TRANSPARENT);
                HFONT font = CreateFontW(-38, 0, 0, 0, FW_SEMIBOLD, FALSE, FALSE, FALSE,
                    DEFAULT_CHARSET, OUT_DEFAULT_PRECIS, CLIP_DEFAULT_PRECIS, CLEARTYPE_QUALITY,
                    DEFAULT_PITCH | FF_SWISS, L"Segoe UI");
                HGDIOBJ oldFont = SelectObject(dc, font);
                const wchar_t* labels[] = {L"Single Player", L"Resume", L"Settings", L"Debug"};
                constexpr int kMenuLabelRows = 4;
                for (int row = 0; row < kMenuLabelRows; ++row) {
                    int top = row * kTextureSize / kMenuLabelRows;
                    int bottom = (row + 1) * kTextureSize / kMenuLabelRows;
                    RECT textRect{28, top + 18, kTextureSize - 28, bottom - 16};
                    SetTextColor(dc, RGB(58, 45, 18));
                    for (int oy = -3; oy <= 3; ++oy) {
                        for (int ox = -3; ox <= 3; ++ox) {
                            if (ox * ox + oy * oy > 10) continue;
                            RECT glow = textRect;
                            OffsetRect(&glow, ox, oy);
                            DrawTextW(dc, labels[row], -1, &glow, DT_CENTER | DT_VCENTER | DT_SINGLELINE | DT_END_ELLIPSIS);
                        }
                    }
                    SetTextColor(dc, RGB(238, 212, 132));
                    DrawTextW(dc, labels[row], -1, &textRect, DT_CENTER | DT_VCENTER | DT_SINGLELINE | DT_END_ELLIPSIS);
                }
                SelectObject(dc, oldFont);
                SelectObject(dc, oldBitmap);
                DeleteObject(font);
                std::memcpy(dib.data(), bits, dib.size());
            }
            if (bitmap) DeleteObject(bitmap);
            if (dc) DeleteDC(dc);
            if (screen) ReleaseDC(nullptr, screen);

            for (int y = 0; y < kTextureSize; ++y) {
                int gy = material * kTextureSize + y;
                for (int x = 0; x < kTextureSize; ++x) {
                    size_t src = static_cast<size_t>((y * kTextureSize + x) * 4);
                    uint8_t b = dib[src + 0];
                    uint8_t g = dib[src + 1];
                    uint8_t r = dib[src + 2];
                    uint8_t a = std::max(r, std::max(g, b));
                    size_t dst = static_cast<size_t>((gy * width + x) * 4);
                    albedo[dst + 0] = r;
                    albedo[dst + 1] = g;
                    albedo[dst + 2] = b;
                    albedo[dst + 3] = a;
                }
            }
        };
        fillMenuLabelAtlas(18);

        auto loadRuntimeAlbedo = [&](int material, const wchar_t* relativePath) {
            if (material < 0 || material >= kMaterialCount) return;
            ImageRGBA img;
            if (LoadImageWic(ResolveConfiguredAssetPath(relativePath), kTextureSize, kTextureSize, img)) {
                applyAlbedo(material, img);
            }
        };
        auto loadRuntimeNormal = [&](int material, const wchar_t* relativePath) {
            if (material < 0 || material >= kMaterialCount) return;
            ImageRGBA img;
            if (LoadImageWic(ResolveConfiguredAssetPath(relativePath), kTextureSize, kTextureSize, img)) {
                applyNormal(material, img);
            }
        };
        auto loadRuntimeRoughnessFromGreen = [&](int material, const wchar_t* relativePath) {
            if (material < 0 || material >= kMaterialCount) return;
            ImageRGBA img;
            if (!LoadImageWic(ResolveConfiguredAssetPath(relativePath), kTextureSize, kTextureSize, img) || !img.Valid()) return;
            for (int y = 0; y < kTextureSize; ++y) {
                int gy = material * kTextureSize + y;
                for (int x = 0; x < kTextureSize; ++x) {
                    size_t src = static_cast<size_t>((y * img.width + x) * 4);
                    size_t dst = static_cast<size_t>((gy * width + x) * 4);
                    props[dst + 1] = img.pixels[src + 1];
                    props[dst + 2] = img.pixels[src + 2];
                }
            }
        };
        auto darkenWhiteChairPlastic = [&](int material) {
            if (material < 0 || material >= kMaterialCount) return;
            for (int y = 0; y < kTextureSize; ++y) {
                int gy = material * kTextureSize + y;
                for (int x = 0; x < kTextureSize; ++x) {
                    size_t i = static_cast<size_t>((gy * width + x) * 4);
                    int r = albedo[i + 0];
                    int g = albedo[i + 1];
                    int b = albedo[i + 2];
                    int hi = std::max(r, std::max(g, b));
                    int lo = std::min(r, std::min(g, b));
                    int lum = (r * 54 + g * 183 + b * 19) >> 8;
                    if (lum > 168 && hi - lo < 72) {
                        uint8_t shade = static_cast<uint8_t>(std::clamp(18 + (lum - 168) / 8, 18, 34));
                        albedo[i + 0] = shade;
                        albedo[i + 1] = static_cast<uint8_t>(std::min<int>(255, shade + 2));
                        albedo[i + 2] = shade;
                        props[i + 0] = 230;
                        props[i + 1] = 218;
                    }
                }
            }
        };
        auto toneTaskChairFabric = [&](int material) {
            if (material < 0 || material >= kMaterialCount) return;
            for (int y = 0; y < kTextureSize; ++y) {
                int gy = material * kTextureSize + y;
                for (int x = 0; x < kTextureSize; ++x) {
                    size_t i = static_cast<size_t>((gy * width + x) * 4);
                    if (albedo[i + 3] < 8) continue;
                    int r = albedo[i + 0];
                    int g = albedo[i + 1];
                    int b = albedo[i + 2];
                    int lum = (r * 54 + g * 183 + b * 19) >> 8;
                    int hi = std::max(r, std::max(g, b));
                    int lo = std::min(r, std::min(g, b));
                    if (lum < 22 || hi - lo > 110) continue;
                    float grain = static_cast<float>(lum - 122) * 0.10f;
                    int shade = std::clamp(static_cast<int>(48.0f + static_cast<float>(lum - 95) * 0.22f + grain), 34, 78);
                    albedo[i + 0] = static_cast<uint8_t>(std::clamp(static_cast<int>(shade * 0.82f), 0, 255));
                    albedo[i + 1] = static_cast<uint8_t>(std::clamp(static_cast<int>(shade * 0.91f), 0, 255));
                    albedo[i + 2] = static_cast<uint8_t>(std::clamp(static_cast<int>(shade * 0.86f), 0, 255));
                    props[i + 0] = 236;
                    props[i + 1] = 226;
                }
            }
        };

        {
            ReportStartupSubStep(L"Loading textures", L"Loading external PBR textures.", 2);
            ScopedCom com;
            if (com.Ok()) {
                loadPbrMaterial(0, settings_.wallStem.c_str());
                loadPbrMaterial(1, settings_.floorStem.c_str());
                loadPbrMaterial(2, settings_.ceilingStem.c_str());
                loadPbrMaterial(15, settings_.fleshStem.c_str());
                loadRuntimeAlbedo(7, L"assets\\models\\runtime\\textures\\emergency_exit_sign_diffuse.jpeg");
                loadRuntimeAlbedo(16, L"assets\\models\\runtime\\textures\\office_chair_modern_diffuse.jpg");
                loadRuntimeAlbedo(19, L"assets\\models\\runtime\\textures\\office_chair_classic_2209.jpg");
                loadRuntimeAlbedo(20, L"assets\\models\\runtime\\textures\\office_chair_classic_textiles.png");
                loadRuntimeAlbedo(22, L"assets\\models\\runtime\\textures\\office_chair_task_diffuse.png");
                loadRuntimeAlbedo(26, L"assets\\models\\monster_face_mask\\horror_mask_baseColor.png");
                loadRuntimeNormal(26, L"assets\\models\\monster_face_mask\\horror_mask_normal.png");
                loadRuntimeRoughnessFromGreen(26, L"assets\\models\\monster_face_mask\\horror_mask_metallicRoughness.png");
                for (int page = 0; page < 8; ++page) {
                    std::wstring path = L"assets\\images\\8pages\\page" + std::to_wstring(page + 1) + L".jpg";
                    loadRuntimeAlbedo(27 + page, path.c_str());
                }
                darkenWhiteChairPlastic(16);
                darkenWhiteChairPlastic(20);
                toneTaskChairFabric(22);
            }
        }
        profile.Mark(L"LoadExternalPBRs");

        ReportStartupSubStep(L"Loading textures", L"Building normal and material property maps.", 3);
        for (int m = 0; m < kMaterialCount; ++m) {
            for (int y = 0; y < kTextureSize; ++y) {
                for (int x = 0; x < kTextureSize; ++x) {
                    auto hAt = [&](int sx, int sy) {
                        sx = (sx + kTextureSize) % kTextureSize;
                        sy = (sy + kTextureSize) % kTextureSize;
                        return heights[static_cast<size_t>((m * kTextureSize + sy) * width + sx)];
                    };
                    float hl = hAt(x - 1, y);
                    float hr = hAt(x + 1, y);
                    float hu = hAt(x, y - 1);
                    float hd = hAt(x, y + 1);
                    XMVECTOR n = XMVector3Normalize(XMVectorSet((hl - hr) * 3.1f, (hu - hd) * 3.1f, 1.0f, 0.0f));
                    XMFLOAT3 nf;
                    XMStoreFloat3(&nf, n);
                    int gy = m * kTextureSize + y;
                    size_t i = static_cast<size_t>((gy * width + x) * 4);
                    if (hasExternalNormal[static_cast<size_t>(gy * width + x)]) {
                        normal[i + 0] = externalNormal[i + 0];
                        normal[i + 1] = externalNormal[i + 1];
                        normal[i + 2] = externalNormal[i + 2];
                    } else {
                        normal[i + 0] = Byte(nf.x * 0.5f + 0.5f);
                        normal[i + 1] = Byte(nf.y * 0.5f + 0.5f);
                        normal[i + 2] = Byte(nf.z * 0.5f + 0.5f);
                    }
                    normal[i + 3] = Byte(hAt(x, y));
                }
            }
        }
        profile.Mark(L"BuildNormalHeightArray");
        SaveTextureCache(textureHash, albedo, normal, props);
        profile.Mark(L"SaveTextureCache");

        ReportStartupSubStep(L"Loading textures", L"Creating GPU texture views.", 3);
        if (!makeSrv(albedo, albedoSrv_)) return false;
        profile.Mark(L"CreateAlbedoSRV");
        if (!makeSrv(normal, normalSrv_)) return false;
        profile.Mark(L"CreateNormalSRV");
        if (!makeSrv(props, materialPropsSrv_)) return false;
        profile.Mark(L"CreateMaterialPropsSRV");
        return true;
    }
