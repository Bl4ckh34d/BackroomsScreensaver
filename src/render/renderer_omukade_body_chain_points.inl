        XMFLOAT3 blobCenter = Add3(monsterPosition, Scale3(monsterForward, -maze.TileMinimum() * 0.18f));
        for (int i = 0; i < bodyCount; ++i) {
            float fi = static_cast<float>(i);
            float t = fi / static_cast<float>(std::max(1, bodyCount - 1));
            XMFLOAT3 p = sampleTrail(fi * visualBodySpacing);
            float lateral = std::sin(fi * 1.83f + timeRuntime_.time * 0.22f + static_cast<float>(sessionRuntime_.runtimeSeed & 255) * 0.011f) *
                maze.TileMinimum() * Lerp(0.006f, 0.032f, SmoothStep(0.08f, 0.72f, t));
            p = Add3(p, Scale3(monsterRight, lateral));
            bodyUps[static_cast<size_t>(i)] = {0.0f, 1.0f, 0.0f};
            p.y = 0.0f;
            bodyPoints[static_cast<size_t>(i)] = p;
            bodyCenterlinePoints[static_cast<size_t>(i)] = p;
            float torsoBulge = std::exp(-std::pow((t - 0.34f) / 0.28f, 2.0f));
            float maskShoulderBulge = std::exp(-std::pow(t / 0.18f, 2.0f));
            float taperRadius = Lerp(0.39f, 0.21f, SmoothStep(0.0f, 1.0f, t));
            float peristalsis = 1.0f + std::sin(timeRuntime_.time * 3.40f - fi * 0.91f) * 0.045f;
            bodyRadii[static_cast<size_t>(i)] = (taperRadius + torsoBulge * 0.235f + maskShoulderBulge * 0.165f) * modelXZ * peristalsis;
            bodyUvShift[static_cast<size_t>(i)] = std::fmod(Rand01(i * 41 + static_cast<int>(sessionRuntime_.runtimeSeed & 1023), 1709, sessionRuntime_.runtimeSeed) * 0.47f +
                std::sin(fi * 1.73f + static_cast<float>(sessionRuntime_.runtimeSeed & 255) * 0.013f) * 0.08f, 1.0f);
        }
