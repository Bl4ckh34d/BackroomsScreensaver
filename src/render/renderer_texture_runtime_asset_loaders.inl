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
