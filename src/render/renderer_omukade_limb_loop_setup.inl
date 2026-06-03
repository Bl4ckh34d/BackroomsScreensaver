        for (int pairIndex = 0; pairIndex < limbPairs; ++pairIndex) {
            float pairT = static_cast<float>(pairIndex) / std::max(1.0f, static_cast<float>(limbPairs - 1));
            float frontness = 1.0f - pairT;
            int bodyIndex = std::clamp(static_cast<int>(std::round(pairT * static_cast<float>(bodyCount - 1) * 0.78f)),
                0, bodyCount - 1);
            XMFLOAT3 localUp = bodyUps[static_cast<size_t>(bodyIndex)];
            XMFLOAT3 tangent = bodyTangents[static_cast<size_t>(bodyIndex)];
            XMFLOAT3 sideAxis = bodySides[static_cast<size_t>(bodyIndex)];
            XMFLOAT3 p = Add3(bodyPoints[static_cast<size_t>(bodyIndex)],
                Scale3(localUp, std::sin(timeRuntime_.time * 1.7f + pairIndex) * 0.012f));
            for (int sideIndex = -1; sideIndex <= 1; sideIndex += 2) {
                int limbId = pairIndex * 2 + (sideIndex > 0 ? 1 : 0);
                float limbSeed = Rand01(limbId * 29 + 1, 823, sessionRuntime_.runtimeSeed);
                float limbPhase = limbSeed * kPi * 2.0f;
                float roamScramble = SmoothStep(0.0f, 1.0f, Clamp01(world.monsterRoamBurstTimer / 1.05f)) * (1.0f - monsterPresentation_.headChaseBlend * 0.65f);
                float chaseScramble = SmoothStep(0.18f, 1.0f, monsterPresentation_.headChaseBlend);
                float phase = timeRuntime_.time * (1.20f + limbSeed * 0.92f + monsterPresentation_.headChaseBlend * (1.45f + limbSeed * 1.35f) +
                    roamScramble * (2.8f + limbSeed * 2.6f) + chaseScramble * (2.2f + limbSeed * 1.8f)) +
                    static_cast<float>(pairIndex) * (0.27f + limbSeed * 0.31f) + static_cast<float>(sideIndex) * (1.10f + limbSeed) + limbPhase;
                float sideScale = static_cast<float>(sideIndex);
                XMFLOAT3 sideDir = Scale3(sideAxis, sideScale);
                float r = bodyRadii[static_cast<size_t>(bodyIndex)] * Lerp(1.08f, 0.92f, pairT);
                float lateralRoot = r * Lerp(1.22f, 1.02f, pairT) * (0.96f + limbSeed * 0.10f);
                float axialRoot = (limbSeed - 0.5f) * bodySpacing * 0.12f;
                XMFLOAT3 root = Add3(p, Add3(Scale3(sideDir, lateralRoot),
                    Add3(Scale3(tangent, axialRoot), Scale3(localUp, r * 0.06f + std::sin(phase) * (0.008f + limbSeed * 0.014f)))));
                float limb = Lerp(0.028f, 0.043f, pairT) * (0.94f + limbSeed * 0.18f);
                float upperLen = std::max(0.14f, maze.TileMinimum() * (Lerp(0.44f, 0.26f, pairT) + limbSeed * 0.035f) + limb * 0.70f);
                float lowerLen = std::max(0.14f, maze.TileMinimum() * (Lerp(0.50f, 0.30f, pairT) + Rand01(limbId * 31 + 9, 829, sessionRuntime_.runtimeSeed) * 0.040f) + limb * 0.66f);
                LimbReachTarget reachTarget = limbReachTarget(root, sideDir, tangent, localUp, limbId, upperLen, lowerLen);
                float swing = reachTarget.swing;
                XMFLOAT3 target = reachTarget.target;
                XMFLOAT3 contactNormal = Normalize3(reachTarget.normal, up);
