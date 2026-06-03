        auto makeFallback = [&](std::vector<uint8_t>& pixels, int slot) {
            pixels.assign(static_cast<size_t>(pageW) * pageH * 4, 255);
            for (int y = 0; y < pageH; ++y) {
                float v = static_cast<float>(y) / static_cast<float>(pageH - 1);
                for (int x = 0; x < pageW; ++x) {
                    float u = static_cast<float>(x) / static_cast<float>(pageW - 1);
                    float grain = FractalNoise(u * 54.0f + slot * 0.73f, v * 72.0f + 11.0f, 481);
                    float edge = std::max(SmoothStep(0.032f, 0.0f, std::min(u, 1.0f - u)),
                        SmoothStep(0.032f, 0.0f, std::min(v, 1.0f - v)));
                    size_t dst = static_cast<size_t>((y * pageW + x) * 4);
                    pixels[dst + 0] = Byte(0.82f + grain * 0.075f - edge * 0.10f);
                    pixels[dst + 1] = Byte(0.80f + grain * 0.065f - edge * 0.09f);
                    pixels[dst + 2] = Byte(0.71f + grain * 0.050f - edge * 0.07f);
                    pixels[dst + 3] = 255;
                }
            }
        };
