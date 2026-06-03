            if (bitmap) DeleteObject(bitmap);
            if (dc) DeleteDC(dc);
            if (screen) ReleaseDC(nullptr, screen);

            for (int y = 0; y < kTextureSize; ++y) {
                int gy = material * kTextureSize + y;
                for (int x = 0; x < kTextureSize; ++x) {
                    size_t src = static_cast<size_t>((y * kTextureSize + x) * 4);
                    uint8_t b = dib[src + 0];
                    uint8_t g = dib[src + 1];
                    uint8_t r = dib[src + 2];
                    uint8_t a = std::max(r, std::max(g, b));
                    size_t dst = static_cast<size_t>((gy * width + x) * 4);
                    albedo[dst + 0] = r;
                    albedo[dst + 1] = g;
                    albedo[dst + 2] = b;
                    albedo[dst + 3] = a;
                }
            }
        };
