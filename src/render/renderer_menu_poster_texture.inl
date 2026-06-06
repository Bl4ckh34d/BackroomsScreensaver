    void UpdateMenuPosterTexture() {
        if (runtimeTextures_.menuPosterSrv) return;
        constexpr int size = kCustomMenuTextureSize;
        std::vector<uint8_t> rgba(static_cast<size_t>(size) * size * 4, 0);

        auto blitPoster = [&](const wchar_t* path, int dstX, int dstY, int dstW, int dstH) {
            ImageRGBA img;
            if (!LoadImageWic(ResolveConfiguredAssetPath(path), dstW, dstH, img) || !img.Valid()) return;
            for (int y = 0; y < dstH; ++y) {
                for (int x = 0; x < dstW; ++x) {
                    size_t src = static_cast<size_t>((y * img.width + x) * 4);
                    size_t dst = static_cast<size_t>(((dstY + y) * size + (dstX + x)) * 4);
                    rgba[dst + 0] = img.pixels[src + 0];
                    rgba[dst + 1] = img.pixels[src + 1];
                    rgba[dst + 2] = img.pixels[src + 2];
                    rgba[dst + 3] = 255;
                }
            }
        };

        ScopedCom com;
        if (com.Ok()) {
            blitPoster(L"assets\\images\\menu\\new_game_poster.png", 0, 0, size / 2, size);
            blitPoster(L"assets\\images\\menu\\custom_level_poster.png", size / 2, 0, size / 2, size);
        }
        CreateTexture2DSrvRGBA(size, rgba, runtimeTextures_.menuPosterSrv);
    }
