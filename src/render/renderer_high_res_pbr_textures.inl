// Renderer texture high res pbr helpers.

    bool CreateHighResPbrTextures(const std::wstring& base,
                                  ComPtr<ID3D11ShaderResourceView>& albedoSrv,
                                  ComPtr<ID3D11ShaderResourceView>& normalSrv,
                                  ComPtr<ID3D11ShaderResourceView>& propsSrv) {
        const int size = kHighResCeilingTextureSize;
        const size_t pixelCount = static_cast<size_t>(size) * size;
        std::vector<uint8_t> albedo(pixelCount * 4, 255);
        std::vector<uint8_t> normalHeight(pixelCount * 4, 255);
        std::vector<uint8_t> props(pixelCount * 4, 255);
        for (size_t i = 0; i < pixelCount; ++i) {
            normalHeight[i * 4 + 0] = 128;
            normalHeight[i * 4 + 1] = 128;
            normalHeight[i * 4 + 2] = 255;
            normalHeight[i * 4 + 3] = 128;
            props[i * 4 + 0] = 255;
            props[i * 4 + 1] = 178;
            props[i * 4 + 2] = 0;
            props[i * 4 + 3] = 255;
        }

        auto resolvePbrPath = [&](const std::wstring& base, std::initializer_list<const wchar_t*> suffixes) {
            for (const wchar_t* suffix : suffixes) {
                std::filesystem::path path = ResolveAsset(settingsRuntime_.live, base + suffix);
                if (!path.empty()) return path;
            }
            return std::filesystem::path{};
        };

        auto copyImage = [&](const ImageRGBA& img, std::vector<uint8_t>& dst) {
            if (!img.Valid()) return;
            for (int y = 0; y < size; ++y) {
                for (int x = 0; x < size; ++x) {
                    size_t src = static_cast<size_t>((y * img.width + x) * 4);
                    size_t out = static_cast<size_t>((y * size + x) * 4);
                    dst[out + 0] = img.pixels[src + 0];
                    dst[out + 1] = img.pixels[src + 1];
                    dst[out + 2] = img.pixels[src + 2];
                    dst[out + 3] = img.pixels[src + 3];
                }
            }
        };

        auto copyScalar = [&](const ImageRGBA& img, std::vector<uint8_t>& dst, int channel) {
            if (!img.Valid() || channel < 0 || channel > 3) return;
            for (int y = 0; y < size; ++y) {
                for (int x = 0; x < size; ++x) {
                    size_t src = static_cast<size_t>((y * img.width + x) * 4);
                    size_t out = static_cast<size_t>((y * size + x) * 4);
                    dst[out + channel] = img.pixels[src];
                }
            }
        };

        auto copyNormal = [&](const ImageRGBA& img, bool flipGreen) {
            if (!img.Valid()) return;
            for (int y = 0; y < size; ++y) {
                for (int x = 0; x < size; ++x) {
                    size_t src = static_cast<size_t>((y * img.width + x) * 4);
                    size_t out = static_cast<size_t>((y * size + x) * 4);
                    normalHeight[out + 0] = img.pixels[src + 0];
                    normalHeight[out + 1] = flipGreen ? static_cast<uint8_t>(255 - img.pixels[src + 1]) : img.pixels[src + 1];
                    normalHeight[out + 2] = img.pixels[src + 2];
                }
            }
        };

        ScopedCom com;
        if (com.Ok()) {
            ImageRGBA img;
            if (!base.empty()) {
                std::filesystem::path path = resolvePbrPath(base, {
                    L"_color_4k.jpg", L"_color_4k.png",
                    L"_Color.jpg", L"_Color.png",
                    L"_BaseColor.jpg", L"_BaseColor.png",
                    L"_Albedo.jpg", L"_Albedo.png",
                    L"_Diffuse.jpg", L"_Diffuse.png"
                });
                if (!path.empty() && LoadImageWic(path, size, size, img)) copyImage(img, albedo);

                path = resolvePbrPath(base, {
                    L"_height_4k.png", L"_height_4k.jpg",
                    L"_Displacement.jpg", L"_Displacement.png",
                    L"_Height.jpg", L"_Height.png"
                });
                if (!path.empty() && LoadImageWic(path, size, size, img)) copyScalar(img, normalHeight, 3);

                path = resolvePbrPath(base, {
                    L"_normal_directx_4k.png", L"_normal_directx_4k.jpg",
                    L"_normal_dx_4k.png", L"_normal_dx_4k.jpg",
                    L"_NormalDX.jpg", L"_NormalDX.png",
                    L"_NormalDirectX.jpg", L"_NormalDirectX.png"
                });
                if (!path.empty() && LoadImageWic(path, size, size, img)) {
                    copyNormal(img, false);
                } else {
                    path = resolvePbrPath(base, {
                        L"_normal_opengl_4k.png", L"_normal_opengl_4k.jpg",
                        L"_normal_gl_4k.png", L"_normal_gl_4k.jpg",
                        L"_NormalGL.jpg", L"_NormalGL.png",
                        L"_NormalOpenGL.jpg", L"_NormalOpenGL.png"
                    });
                    if (!path.empty() && LoadImageWic(path, size, size, img)) copyNormal(img, true);
                }

                path = resolvePbrPath(base, {
                    L"_ao_4k.jpg", L"_ao_4k.png",
                    L"_AO.jpg", L"_AO.png",
                    L"_AmbientOcclusion.jpg", L"_AmbientOcclusion.png"
                });
                if (!path.empty() && LoadImageWic(path, size, size, img)) copyScalar(img, props, 0);

                path = resolvePbrPath(base, {
                    L"_roughness_4k.jpg", L"_roughness_4k.png",
                    L"_Roughness.jpg", L"_Roughness.png"
                });
                if (!path.empty() && LoadImageWic(path, size, size, img)) copyScalar(img, props, 1);

                path = resolvePbrPath(base, {
                    L"_metallic_4k.jpg", L"_metallic_4k.png",
                    L"_metalness_4k.jpg", L"_metalness_4k.png",
                    L"_Metallic.jpg", L"_Metallic.png",
                    L"_Metalness.jpg", L"_Metalness.png"
                });
                if (!path.empty() && LoadImageWic(path, size, size, img)) copyScalar(img, props, 2);
            }
        }

        return CreateTexture2DSrvRGBA(size, albedo, albedoSrv) &&
            CreateTexture2DSrvRGBA(size, normalHeight, normalSrv) &&
            CreateTexture2DSrvRGBA(size, props, propsSrv);
    }

    bool CreateHighResCeilingTextures() {
        return CreateHighResPbrTextures(settingsRuntime_.live.ceilingStem, materialTextures_.ceilingAlbedoSrv, materialTextures_.ceilingNormalSrv, materialTextures_.ceilingPropsSrv);
    }

    bool CreateHighResDoorTextures() {
        return CreateHighResPbrTextures(
                L"assets\\PBRs\\downloads\\door001\\extracted\\Door001_4K-JPG",
                materialTextures_.doorAlbedoSrv, materialTextures_.doorNormalSrv, materialTextures_.doorPropsSrv) &&
            CreateHighResPbrTextures(
                L"assets\\PBRs\\downloads\\painted_wood_009c\\extracted\\PaintedWood009C_4K-JPG",
                materialTextures_.doorFrameAlbedoSrv, materialTextures_.doorFrameNormalSrv, materialTextures_.doorFramePropsSrv);
    }
