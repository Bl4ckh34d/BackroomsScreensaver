        auto loadRandomLoosePageAtlas = [&]() {
            constexpr int material = kRandomLoosePageMaterial;
            constexpr int cellW = kTextureSize / kRandomLoosePageAtlasColumns;
            constexpr int cellH = kTextureSize / kRandomLoosePageAtlasRows;
            for (int y = 0; y < kTextureSize; ++y) {
                int gy = material * kTextureSize + y;
                for (int x = 0; x < kTextureSize; ++x) {
                    float u = static_cast<float>(x) / static_cast<float>(kTextureSize - 1);
                    float v = static_cast<float>(y) / static_cast<float>(kTextureSize - 1);
                    float paperGrain = FractalNoise(u * 64.0f, v * 64.0f, 481);
                    size_t dst = static_cast<size_t>((gy * width + x) * 4);
                    albedo[dst + 0] = Byte(0.80f + paperGrain * 0.10f);
                    albedo[dst + 1] = Byte(0.78f + paperGrain * 0.08f);
                    albedo[dst + 2] = Byte(0.69f + paperGrain * 0.06f);
                    albedo[dst + 3] = 255;
                    props[dst + 0] = 238;
                    props[dst + 1] = 226;
                }
            }

            std::vector<std::filesystem::path> files = RandomLoosePageFiles();
            int count = std::min<int>(static_cast<int>(files.size()), kRandomLoosePageAtlasSlots);
            for (int slot = 0; slot < count; ++slot) {
                ImageRGBA img;
                if (!LoadImageWic(files[static_cast<size_t>(slot)], cellW, cellH, img) || !img.Valid()) continue;
                int cellX = slot % kRandomLoosePageAtlasColumns;
                int cellY = slot / kRandomLoosePageAtlasColumns;
                for (int y = 0; y < cellH; ++y) {
                    int dy = cellY * cellH + y;
                    int gy = material * kTextureSize + dy;
                    for (int x = 0; x < cellW; ++x) {
                        int dx = cellX * cellW + x;
                        size_t src = static_cast<size_t>((y * img.width + x) * 4);
                        size_t dst = static_cast<size_t>((gy * width + dx) * 4);
                        albedo[dst + 0] = img.pixels[src + 0];
                        albedo[dst + 1] = img.pixels[src + 1];
                        albedo[dst + 2] = img.pixels[src + 2];
                        albedo[dst + 3] = img.pixels[src + 3];
                    }
                }
            }
        };
