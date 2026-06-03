        auto copyNormal = [&](const ImageRGBA& img, bool flipGreen) {
            if (!img.Valid()) return;
            for (int y = 0; y < size; ++y) {
                for (int x = 0; x < size; ++x) {
                    size_t src = static_cast<size_t>((y * img.width + x) * 4);
                    size_t out = static_cast<size_t>((y * size + x) * 4);
                    normalHeight[out + 0] = img.pixels[src + 0];
                    normalHeight[out + 1] = flipGreen ? static_cast<uint8_t>(255 - img.pixels[src + 1]) : img.pixels[src + 1];
                    normalHeight[out + 2] = img.pixels[src + 2];
                }
            }
        };
