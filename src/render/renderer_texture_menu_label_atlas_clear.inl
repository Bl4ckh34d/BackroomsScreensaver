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
