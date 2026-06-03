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
