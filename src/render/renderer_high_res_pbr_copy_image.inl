        auto copyImage = [&](const ImageRGBA& img, std::vector<uint8_t>& dst) {
            if (!img.Valid()) return;
            for (int y = 0; y < size; ++y) {
                for (int x = 0; x < size; ++x) {
                    size_t src = static_cast<size_t>((y * img.width + x) * 4);
                    size_t out = static_cast<size_t>((y * size + x) * 4);
                    dst[out + 0] = img.pixels[src + 0];
                    dst[out + 1] = img.pixels[src + 1];
                    dst[out + 2] = img.pixels[src + 2];
                    dst[out + 3] = img.pixels[src + 3];
                }
            }
        };
