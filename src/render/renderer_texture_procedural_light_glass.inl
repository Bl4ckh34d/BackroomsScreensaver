                float edge = std::max(SmoothStep(0.055f, 0.0f, std::min(u, 1.0f - u)),
                                      SmoothStep(0.055f, 0.0f, std::min(v, 1.0f - v)));
                float lens = SmoothStep(0.42f, 0.0f, std::abs(v - 0.5f)) * SmoothStep(0.46f, 0.0f, std::abs(u - 0.5f));
                setPixel(3, x, y,
                    0.72f + lens * 0.24f - edge * 0.18f,
                    0.76f + lens * 0.23f - edge * 0.16f,
                    0.70f + lens * 0.20f - edge * 0.12f,
                    1.0f,
                    0.5f);
                setPixel(5, x, y,
                    0.018f + lens * 0.012f,
                    0.018f + lens * 0.012f,
                    0.016f + lens * 0.010f,
                    1.0f,
                    0.5f);
