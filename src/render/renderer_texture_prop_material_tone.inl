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
