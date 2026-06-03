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
