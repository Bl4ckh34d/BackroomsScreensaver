        bodyUvV[0] = static_cast<float>(sessionRuntime_.runtimeSeed & 4095) * 0.0017f;
        for (int i = 1; i < bodyCount; ++i) {
            float segmentLen = Length3(Sub3(bodyPoints[static_cast<size_t>(i)], bodyPoints[static_cast<size_t>(i - 1)]));
            float seedStretch = 0.86f + Rand01(i * 53 + static_cast<int>(sessionRuntime_.runtimeSeed & 2047), 1723, sessionRuntime_.runtimeSeed) * 0.48f;
            bodyUvV[static_cast<size_t>(i)] = bodyUvV[static_cast<size_t>(i - 1)] + segmentLen * seedStretch * 0.62f;
        }
