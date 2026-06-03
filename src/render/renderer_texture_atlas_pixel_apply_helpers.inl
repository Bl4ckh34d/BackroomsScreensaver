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
