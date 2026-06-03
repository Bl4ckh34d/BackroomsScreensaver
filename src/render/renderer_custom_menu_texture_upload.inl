        if (bitmap) DeleteObject(bitmap);
        if (dc) DeleteDC(dc);
        if (screen) ReleaseDC(nullptr, screen);

        std::vector<uint8_t> rgba(static_cast<size_t>(size) * size * 4, 0);
        for (size_t i = 0; i < static_cast<size_t>(size) * size; ++i) {
            uint8_t b = dib[i * 4 + 0];
            uint8_t g = dib[i * 4 + 1];
            uint8_t r = dib[i * 4 + 2];
            if (r > 170 && g > 130 && b < 150) {
                rgba[i * 4 + 0] = 238;
                rgba[i * 4 + 1] = 195;
                rgba[i * 4 + 2] = 36;
                rgba[i * 4 + 3] = 150;
                continue;
            }
            int ink = 255 - std::min<int>(r, std::min<int>(g, b));
            uint8_t a = ink < 7 ? 0 : static_cast<uint8_t>(std::clamp(ink * 2, 0, 255));
            uint8_t shade = static_cast<uint8_t>(std::clamp(6 + (255 - ink) / 46, 6, 28));
            rgba[i * 4 + 0] = a > 0 ? shade : 0;
            rgba[i * 4 + 1] = a > 0 ? shade : 0;
            rgba[i * 4 + 2] = a > 0 ? static_cast<uint8_t>(std::max<int>(4, shade - 2)) : 0;
            rgba[i * 4 + 3] = a;
        }
        CreateTexture2DSrvRGBA(size, rgba, runtimeTextures_.customMenuSrv);
