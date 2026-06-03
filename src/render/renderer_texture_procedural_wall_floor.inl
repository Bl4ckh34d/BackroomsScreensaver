                float u = static_cast<float>(x) / kTextureSize;
                float v = static_cast<float>(y) / kTextureSize;
                float n1 = FractalNoise(u * 8.0f, v * 8.0f, 13);
                float stains = FractalNoise(u * 2.0f + 31.0f, v * 3.0f, 33);
                float seam = std::min(std::abs(std::fmod(u * 4.0f, 1.0f) - 0.02f), std::abs(std::fmod(v * 2.0f, 1.0f) - 0.02f));
                float groove = SmoothStep(0.035f, 0.0f, seam);
                float grime = SmoothStep(0.4f, 1.0f, v) * 0.18f + SmoothStep(0.72f, 0.95f, stains) * 0.23f;
                float pattern = std::sin(u * 140.0f) * std::sin(v * 90.0f) * 0.015f;
                setPixel(0, x, y,
                    0.82f + pattern - grime * 0.82f + n1 * 0.055f,
                    0.68f + pattern - grime * 0.72f + n1 * 0.045f,
                    0.34f + pattern - grime * 0.42f + n1 * 0.018f,
                    1.0f,
                    0.54f - groove * 0.23f + n1 * 0.12f);

                float carpet = FractalNoise(u * 28.0f, v * 28.0f, 71);
                float tileGroove = std::max(SmoothStep(0.025f, 0.0f, std::abs(std::fmod(u * 4.0f, 1.0f) - 0.02f)),
                                            SmoothStep(0.025f, 0.0f, std::abs(std::fmod(v * 4.0f, 1.0f) - 0.02f)));
                float damp = SmoothStep(0.65f, 0.95f, FractalNoise(u * 3.0f + 11.0f, v * 3.0f + 19.0f, 92));
                setPixel(1, x, y,
                    0.60f + carpet * 0.11f - damp * 0.13f,
                    0.52f + carpet * 0.09f - damp * 0.10f,
                    0.30f + carpet * 0.055f - damp * 0.075f,
                    1.0f,
                    0.45f + carpet * 0.18f - tileGroove * 0.30f);
