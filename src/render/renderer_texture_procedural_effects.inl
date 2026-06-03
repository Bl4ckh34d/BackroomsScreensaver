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
