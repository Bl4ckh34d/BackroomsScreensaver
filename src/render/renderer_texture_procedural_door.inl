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
