        auto blitFit = [&](const ImageRGBA& img, std::vector<uint8_t>& pixels) {
            if (!img.Valid()) return;
            float scale = std::min(static_cast<float>(pageW) / static_cast<float>(img.width),
                static_cast<float>(pageH) / static_cast<float>(img.height));
            int dstW = std::clamp(static_cast<int>(std::round(static_cast<float>(img.width) * scale)), 1, pageW);
            int dstH = std::clamp(static_cast<int>(std::round(static_cast<float>(img.height) * scale)), 1, pageH);
            int dstX0 = (pageW - dstW) / 2;
            int dstY0 = (pageH - dstH) / 2;
            for (int y = 0; y < dstH; ++y) {
                float sy = (static_cast<float>(y) + 0.5f) * static_cast<float>(img.height) / static_cast<float>(dstH) - 0.5f;
                int y0 = std::clamp(static_cast<int>(std::floor(sy)), 0, img.height - 1);
                int y1 = std::clamp(y0 + 1, 0, img.height - 1);
                float fy = sy - static_cast<float>(y0);
                for (int x = 0; x < dstW; ++x) {
                    float sx = (static_cast<float>(x) + 0.5f) * static_cast<float>(img.width) / static_cast<float>(dstW) - 0.5f;
                    int x0 = std::clamp(static_cast<int>(std::floor(sx)), 0, img.width - 1);
                    int x1 = std::clamp(x0 + 1, 0, img.width - 1);
                    float fx = sx - static_cast<float>(x0);
                    auto sample = [&](int px, int py, int c) {
                        return static_cast<float>(img.pixels[static_cast<size_t>((py * img.width + px) * 4 + c)]);
                    };
                    size_t dst = static_cast<size_t>(((dstY0 + y) * pageW + (dstX0 + x)) * 4);
                    float rgba[4]{};
                    for (int c = 0; c < 4; ++c) {
                        float a = Lerp(sample(x0, y0, c), sample(x1, y0, c), fx);
                        float b = Lerp(sample(x0, y1, c), sample(x1, y1, c), fx);
                        rgba[c] = Lerp(a, b, fy);
                    }
                    float alpha = Clamp01(rgba[3] / 255.0f);
                    for (int c = 0; c < 3; ++c) {
                        float bg = static_cast<float>(pixels[dst + c]);
                        pixels[dst + c] = static_cast<uint8_t>(std::clamp(static_cast<int>(std::round(Lerp(bg, rgba[c], alpha))), 0, 255));
                    }
                    pixels[dst + 3] = 255;
                }
            }
        };
