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

        auto applyNormal = [&](int material, const ImageRGBA& img, bool flipGreen = false) {
            if (!img.Valid()) return;
            for (int y = 0; y < kTextureSize; ++y) {
                int gy = material * kTextureSize + y;
                for (int x = 0; x < kTextureSize; ++x) {
                    size_t src = static_cast<size_t>((y * img.width + x) * 4);
                    size_t dst = static_cast<size_t>((gy * width + x) * 4);
                    externalNormal[dst + 0] = img.pixels[src + 0];
                    externalNormal[dst + 1] = flipGreen ? static_cast<uint8_t>(255 - img.pixels[src + 1]) : img.pixels[src + 1];
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

        auto resolvePbrPath = [&](const std::wstring& base, std::initializer_list<const wchar_t*> suffixes) {
            for (const wchar_t* suffix : suffixes) {
                std::filesystem::path path = ResolveAsset(settingsRuntime_.live, base + suffix);
                if (!path.empty()) return path;
            }
            return std::filesystem::path{};
        };

        auto loadPbrImage = [&](const std::wstring& base, std::initializer_list<const wchar_t*> suffixes, ImageRGBA& img) {
            std::filesystem::path path = resolvePbrPath(base, suffixes);
            return !path.empty() && LoadImageWic(path, kTextureSize, kTextureSize, img);
        };

        auto loadPbrMaterial = [&](int material, const wchar_t* stem) {
            ImageRGBA img;
            std::wstring base(stem);
            if (base.empty()) return;
            if (loadPbrImage(base, {
                L"_color_4k.jpg", L"_color_4k.png",
                L"_Color.jpg", L"_Color.png",
                L"_BaseColor.jpg", L"_BaseColor.png",
                L"_Albedo.jpg", L"_Albedo.png",
                L"_Diffuse.jpg", L"_Diffuse.png"
            }, img)) {
                applyAlbedo(material, img);
            }
            if (loadPbrImage(base, {
                L"_height_4k.png", L"_height_4k.jpg",
                L"_Displacement.jpg", L"_Displacement.png",
                L"_Height.jpg", L"_Height.png"
            }, img)) {
                applyHeight(material, img);
            }
            std::filesystem::path normalPath = resolvePbrPath(base, {
                L"_normal_directx_4k.png", L"_normal_directx_4k.jpg",
                L"_normal_dx_4k.png", L"_normal_dx_4k.jpg",
                L"_NormalDX.jpg", L"_NormalDX.png",
                L"_NormalDirectX.jpg", L"_NormalDirectX.png"
            });
            std::error_code ec;
            uintmax_t normalSize = std::filesystem::exists(normalPath, ec) ? std::filesystem::file_size(normalPath, ec) : 0;
            if (settingsRuntime_.live.useExternalNormals && normalSize > 0 &&
                (material == 15 || normalSize <= static_cast<uintmax_t>(settingsRuntime_.live.maxNormalMapMB) * 1024ull * 1024ull) &&
                LoadImageWic(normalPath, kTextureSize, kTextureSize, img)) {
                applyNormal(material, img);
            } else {
                normalPath = resolvePbrPath(base, {
                    L"_normal_opengl_4k.png", L"_normal_opengl_4k.jpg",
                    L"_normal_gl_4k.png", L"_normal_gl_4k.jpg",
                    L"_NormalGL.jpg", L"_NormalGL.png",
                    L"_NormalOpenGL.jpg", L"_NormalOpenGL.png"
                });
                ec.clear();
                normalSize = std::filesystem::exists(normalPath, ec) ? std::filesystem::file_size(normalPath, ec) : 0;
                if (settingsRuntime_.live.useExternalNormals && normalSize > 0 &&
                    (material == 15 || normalSize <= static_cast<uintmax_t>(settingsRuntime_.live.maxNormalMapMB) * 1024ull * 1024ull) &&
                    LoadImageWic(normalPath, kTextureSize, kTextureSize, img)) {
                    applyNormal(material, img, true);
                }
            }
            if (loadPbrImage(base, {
                L"_ao_4k.jpg", L"_ao_4k.png",
                L"_AO.jpg", L"_AO.png",
                L"_AmbientOcclusion.jpg", L"_AmbientOcclusion.png"
            }, img)) {
                applyScalarProp(material, img, 0);
            }
            if (loadPbrImage(base, {
                L"_roughness_4k.jpg", L"_roughness_4k.png",
                L"_Roughness.jpg", L"_Roughness.png"
            }, img)) {
                applyScalarProp(material, img, 1);
            }
            if (loadPbrImage(base, {
                L"_metallic_4k.jpg", L"_metallic_4k.png",
                L"_metalness_4k.jpg", L"_metalness_4k.png",
                L"_Metallic.jpg", L"_Metallic.png",
                L"_Metalness.jpg", L"_Metalness.png"
            }, img)) {
                applyScalarProp(material, img, 2);
            }
        };
