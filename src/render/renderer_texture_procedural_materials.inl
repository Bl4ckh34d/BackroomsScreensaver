        ReportStartupSubStep(L"Loading textures", L"Generating procedural material atlas.", 1);
        for (int y = 0; y < kTextureSize; ++y) {
            for (int x = 0; x < kTextureSize; ++x) {
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

                float verticalGrain = FractalNoise(u * 34.0f + 8.0f, v * 3.6f + 19.0f, 91);
                float fineGrain = FractalNoise(u * 96.0f + 4.0f, v * 18.0f + 2.0f, 132);
                float plank = std::abs(std::fmod(u * 7.0f + verticalGrain * 0.08f, 1.0f) - 0.5f);
                float plankLine = SmoothStep(0.035f, 0.0f, plank);
                float doorPanel = std::max(
                    SmoothStep(0.018f, 0.0f, std::abs(u - 0.115f)),
                    SmoothStep(0.018f, 0.0f, std::abs(u - 0.885f)));
                doorPanel = std::max(doorPanel, std::max(
                    SmoothStep(0.018f, 0.0f, std::abs(v - 0.150f)),
                    SmoothStep(0.018f, 0.0f, std::abs(v - 0.850f))));
                float doorGrime = SmoothStep(0.68f, 0.98f, FractalNoise(u * 5.0f + 17.0f, v * 8.0f + 4.0f, 19));
                float wornEdge = SmoothStep(0.055f, 0.0f, std::min(std::min(u, 1.0f - u), std::min(v, 1.0f - v)));
                setPixel(6, x, y,
                    0.315f + verticalGrain * 0.070f + fineGrain * 0.020f - doorPanel * 0.045f - plankLine * 0.035f - doorGrime * 0.060f + wornEdge * 0.025f,
                    0.242f + verticalGrain * 0.050f + fineGrain * 0.017f - doorPanel * 0.040f - plankLine * 0.026f - doorGrime * 0.048f + wornEdge * 0.018f,
                    0.154f + verticalGrain * 0.032f + fineGrain * 0.012f - doorPanel * 0.025f - plankLine * 0.016f - doorGrime * 0.026f,
                    1.0f,
                    0.47f + verticalGrain * 0.15f + doorPanel * 0.08f + plankLine * 0.10f);

                setPixel(7, x, y,
                    0.02f,
                    0.42f,
                    0.12f,
                    1.0f,
                    0.5f);

                float plasticGrain = FractalNoise(u * 14.0f, v * 13.0f, 91);
                setPixel(8, x, y,
                    0.66f + plasticGrain * 0.065f,
                    0.61f + plasticGrain * 0.055f,
                    0.47f + plasticGrain * 0.040f,
                    1.0f,
                    0.50f + plasticGrain * 0.05f);

                float paperEdge = std::max(SmoothStep(0.035f, 0.0f, std::min(u, 1.0f - u)),
                                           SmoothStep(0.035f, 0.0f, std::min(v, 1.0f - v)));
                float paperStain = SmoothStep(0.62f, 0.95f, FractalNoise(u * 4.0f + 4.0f, v * 6.0f + 15.0f, 21));
                setPixel(9, x, y,
                    0.82f - paperEdge * 0.12f - paperStain * 0.22f,
                    0.80f - paperEdge * 0.11f - paperStain * 0.18f,
                    0.70f - paperEdge * 0.08f - paperStain * 0.13f,
                    1.0f,
                    0.50f);

                float scratch = std::max(SmoothStep(0.018f, 0.0f, std::abs(std::fmod(u * 19.0f + v * 3.0f, 1.0f) - 0.04f)),
                                         SmoothStep(0.014f, 0.0f, std::abs(std::fmod(v * 23.0f - u * 2.0f, 1.0f) - 0.06f)));
                float rust = SmoothStep(0.72f, 0.96f, FractalNoise(u * 5.0f + 19.0f, v * 6.0f - 7.0f, 118));
                setPixel(10, x, y,
                    0.055f + n1 * 0.025f + scratch * 0.040f + rust * 0.055f,
                    0.058f + n1 * 0.023f + scratch * 0.034f + rust * 0.025f,
                    0.062f + n1 * 0.020f + scratch * 0.030f - rust * 0.008f,
                    1.0f,
                    0.46f + n1 * 0.07f + scratch * 0.10f - rust * 0.08f);

                float wet = SmoothStep(0.35f, 0.0f, std::abs(u - 0.5f)) * SmoothStep(0.50f, 0.02f, std::abs(v - 0.5f));
                wet = std::max(wet, SmoothStep(0.78f, 1.0f, FractalNoise(u * 6.0f, v * 6.0f, 137)));
                setPixel(11, x, y,
                    0.020f + wet * 0.030f,
                    0.026f + wet * 0.036f,
                    0.024f + wet * 0.032f,
                    1.0f,
                    0.60f + wet * 0.15f);

                float eyeDx = (u - 0.5f) / 0.34f;
                float eyeDy = (v - 0.5f) / 0.26f;
                float eyeGlow = std::exp(-(eyeDx * eyeDx + eyeDy * eyeDy) * 3.3f);
                float hot = std::exp(-(eyeDx * eyeDx + eyeDy * eyeDy) * 28.0f);
                float ragged = 0.78f + FractalNoise(u * 11.0f, v * 9.0f, 222) * 0.22f;
                float eyeAlpha = Clamp01((eyeGlow * 0.98f + hot * 1.18f) * ragged);
                setPixel(12, x, y,
                    1.0f,
                    0.060f + hot * 0.26f,
                    0.020f,
                    eyeAlpha,
                    0.58f);
            }
        }
        profile.Mark(L"ProceduralMaterials");
