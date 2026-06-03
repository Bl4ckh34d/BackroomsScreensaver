        auto copyScalar = [&](const ImageRGBA& img, std::vector<uint8_t>& dst, int channel) {
            if (!img.Valid() || channel < 0 || channel > 3) return;
            for (int y = 0; y < size; ++y) {
                for (int x = 0; x < size; ++x) {
                    size_t src = static_cast<size_t>((y * img.width + x) * 4);
                    size_t out = static_cast<size_t>((y * size + x) * 4);
                    dst[out + channel] = img.pixels[src];
                }
            }
        };
