                float panelX = std::abs(std::fmod(u * 2.0f, 1.0f) - 0.01f);
                float panelY = std::abs(std::fmod(v * 2.0f, 1.0f) - 0.01f);
                float grid = std::max(SmoothStep(0.03f, 0.0f, panelX), SmoothStep(0.03f, 0.0f, panelY));
                float strip = SmoothStep(0.035f, 0.0f, std::abs(std::fmod(u * 2.0f, 1.0f) - 0.5f)) *
                              SmoothStep(0.35f, 0.05f, std::abs(std::fmod(v * 2.0f, 1.0f) - 0.5f));
                float speck = FractalNoise(u * 60.0f, v * 60.0f, 44);
                setPixel(2, x, y,
                    0.76f + strip * 0.20f - grid * 0.13f + speck * 0.030f,
                    0.64f + strip * 0.18f - grid * 0.12f + speck * 0.026f,
                    0.34f + strip * 0.10f - grid * 0.08f + speck * 0.018f,
                    1.0f,
                    0.48f - grid * 0.28f + speck * 0.08f);
