                auto lineMask = [&](float value, float center, float width) {
                    return SmoothStep(width, 0.0f, std::abs(value - center));
                };
                float torso = SmoothStep(1.0f, 0.64f, ((u - 0.50f) / 0.135f) * ((u - 0.50f) / 0.135f) + ((v - 0.57f) / 0.35f) * ((v - 0.57f) / 0.35f));
                float waist = SmoothStep(1.0f, 0.70f, ((u - 0.50f) / 0.085f) * ((u - 0.50f) / 0.085f) + ((v - 0.77f) / 0.25f) * ((v - 0.77f) / 0.25f));
                float head = SmoothStep(1.0f, 0.63f, ((u - 0.5f) / 0.135f) * ((u - 0.5f) / 0.135f) + ((v - 0.245f) / 0.145f) * ((v - 0.245f) / 0.145f));
                float neck = lineMask(u, 0.50f, 0.055f) * SmoothStep(0.31f, 0.48f, v) * (1.0f - SmoothStep(0.52f, 0.60f, v));
                float armL = lineMask(u, 0.34f + (v - 0.34f) * 0.18f, 0.035f) * SmoothStep(0.30f, 0.56f, v) * (1.0f - SmoothStep(0.83f, 0.93f, v));
                float armR = lineMask(u, 0.66f - (v - 0.34f) * 0.18f, 0.035f) * SmoothStep(0.30f, 0.56f, v) * (1.0f - SmoothStep(0.83f, 0.93f, v));
                float clawL = lineMask(u, 0.29f, 0.030f) * SmoothStep(0.76f, 0.88f, v) * (1.0f - SmoothStep(0.90f, 0.98f, v));
                float clawR = lineMask(u, 0.71f, 0.030f) * SmoothStep(0.76f, 0.88f, v) * (1.0f - SmoothStep(0.90f, 0.98f, v));
                float legL = lineMask(u, 0.44f - (v - 0.74f) * 0.09f, 0.040f) * SmoothStep(0.68f, 0.82f, v);
                float legR = lineMask(u, 0.56f + (v - 0.74f) * 0.09f, 0.040f) * SmoothStep(0.68f, 0.82f, v);
                float antlerL = lineMask(u, 0.43f - (0.22f - v) * 0.95f, 0.026f) * SmoothStep(0.06f, 0.16f, v) * (1.0f - SmoothStep(0.22f, 0.29f, v));
                float antlerR = lineMask(u, 0.57f + (0.22f - v) * 0.95f, 0.026f) * SmoothStep(0.06f, 0.16f, v) * (1.0f - SmoothStep(0.22f, 0.29f, v));
                float tineL = lineMask(u, 0.31f, 0.020f) * SmoothStep(0.07f, 0.14f, v) * (1.0f - SmoothStep(0.18f, 0.24f, v));
                float tineR = lineMask(u, 0.69f, 0.020f) * SmoothStep(0.07f, 0.14f, v) * (1.0f - SmoothStep(0.18f, 0.24f, v));
                float rib = std::max(std::max(lineMask(v, 0.50f, 0.012f), lineMask(v, 0.56f, 0.012f)), lineMask(v, 0.62f, 0.012f)) * lineMask(u, 0.50f, 0.13f);
                float skinNoise = FractalNoise(u * 18.0f, v * 24.0f, 213);
                float vein = SmoothStep(0.030f, 0.0f, std::abs(std::fmod(u * 9.0f + v * 2.7f, 1.0f) - 0.06f)) *
                    SmoothStep(0.46f, 0.92f, FractalNoise(u * 5.0f + 9.0f, v * 8.0f, 214));
                float scar = SmoothStep(0.018f, 0.0f, std::abs(std::fmod(u * 13.0f - v * 4.0f, 1.0f) - 0.04f));
                setPixel(4, x, y,
                    0.018f + skinNoise * 0.016f + vein * 0.018f + scar * 0.012f,
                    0.017f + skinNoise * 0.014f + vein * 0.014f + scar * 0.010f,
                    0.016f + skinNoise * 0.012f + vein * 0.010f + scar * 0.009f,
                    1.0f,
                    0.43f + skinNoise * 0.18f + scar * 0.12f);
