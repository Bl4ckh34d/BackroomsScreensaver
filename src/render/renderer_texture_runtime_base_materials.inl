        auto fillRuntimeMaterial = [&](int material, float r, float g, float b, float roughness) {
            if (material < 0 || material >= kMaterialCount) return;
            for (int y = 0; y < kTextureSize; ++y) {
                int gy = material * kTextureSize + y;
                for (int x = 0; x < kTextureSize; ++x) {
                    setPixel(material, x, y, r, g, b, 1.0f, 0.50f);
                    size_t i = static_cast<size_t>((gy * width + x) * 4);
                    props[i + 1] = Byte(roughness);
                }
            }
        };
        fillRuntimeMaterial(16, 0.62f, 0.58f, 0.50f, 0.62f);
        fillRuntimeMaterial(17, 0.13f, 0.145f, 0.15f, 0.72f);
        fillRuntimeMaterial(18, 0.012f, 0.012f, 0.012f, 0.58f);
        fillRuntimeMaterial(19, 0.70f, 0.68f, 0.62f, 0.50f);
        fillRuntimeMaterial(20, 0.29f, 0.50f, 0.64f, 0.82f);
        fillRuntimeMaterial(21, 0.78f, 0.78f, 0.74f, 0.42f);
        fillRuntimeMaterial(22, 0.38f, 0.36f, 0.34f, 0.62f);
        fillRuntimeMaterial(23, 0.035f, 0.034f, 0.031f, 0.70f);
        fillRuntimeMaterial(24, 0.78f, 0.74f, 0.62f, 0.58f);

