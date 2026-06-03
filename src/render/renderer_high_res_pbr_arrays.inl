        const int size = kHighResCeilingTextureSize;
        const size_t pixelCount = static_cast<size_t>(size) * size;
        std::vector<uint8_t> albedo(pixelCount * 4, 255);
        std::vector<uint8_t> normalHeight(pixelCount * 4, 255);
        std::vector<uint8_t> props(pixelCount * 4, 255);
        for (size_t i = 0; i < pixelCount; ++i) {
            normalHeight[i * 4 + 0] = 128;
            normalHeight[i * 4 + 1] = 128;
            normalHeight[i * 4 + 2] = 255;
            normalHeight[i * 4 + 3] = 128;
            props[i * 4 + 0] = 255;
            props[i * 4 + 1] = 178;
            props[i * 4 + 2] = 0;
            props[i * 4 + 3] = 255;
        }
