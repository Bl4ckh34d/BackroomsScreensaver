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
