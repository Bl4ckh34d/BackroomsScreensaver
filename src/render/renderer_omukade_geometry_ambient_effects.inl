        constexpr int monsterSmokePuffs = 0;
        for (int i = 0; i < monsterSmokePuffs; ++i) {
            float fi = static_cast<float>(i);
            float a = fi * 2.399963f + std::sin(timeRuntime_.time * (0.31f + fi * 0.017f)) * 0.34f;
            float layer = std::fmod(fi * 0.618034f, 1.0f);
            float column = std::fmod(fi * 0.381966f, 1.0f) - 0.5f;
            float lower = 1.0f - layer;
            float y = 0.05f + layer * 1.64f + std::sin(timeRuntime_.time * (0.74f + fi * 0.023f) + fi) * (0.060f + lower * 0.048f);
            float radius = 0.06f + 0.30f * (1.0f - std::abs(layer - 0.52f)) + lower * 0.28f;
            float x = std::cos(a) * radius + std::sin(timeRuntime_.time * 0.47f + fi) * 0.045f;
            float z = std::sin(a) * radius + kMonsterSmokeBackOffset * 0.34f + column * 0.20f +
                std::cos(timeRuntime_.time * 0.39f + fi * 1.7f) * 0.045f;
            float pulse = 0.88f + std::sin(timeRuntime_.time * (0.68f + fi * 0.031f) + fi * 3.1f) * 0.12f;
            float lowerScale = Lerp(1.04f, 2.55f, std::pow(lower, 1.30f));
            smokePuff(x, y, z,
                (0.166f + 0.104f * (1.0f - std::abs(layer - 0.50f))) * pulse * lowerScale,
                (0.156f + 0.122f * (1.0f - std::abs(layer - 0.45f))) * pulse * lowerScale,
                smokeMaterial(1.07f + fi * 0.043f));
        }

        const int capeStrips = 0;
        for (int i = 0; i < capeStrips; ++i) {
            float a0 = (static_cast<float>(i) / static_cast<float>(capeStrips)) * kPi * 2.0f;
            float a1 = (static_cast<float>(i + 1) / static_cast<float>(capeStrips)) * kPi * 2.0f;
            float wave0 = std::sin(timeRuntime_.time * 2.6f + a0 * 2.0f + monsterPosition.x * 0.11f) * 0.035f;
            float wave1 = std::sin(timeRuntime_.time * 2.6f + a1 * 2.0f + monsterPosition.z * 0.09f) * 0.035f;
            float topR0 = 0.33f + wave0 * 0.60f;
            float topR1 = 0.33f + wave1 * 0.60f;
            float midR0 = 0.50f + wave0 * 1.35f;
            float midR1 = 0.50f + wave1 * 1.35f;
            float botR0 = 0.40f + wave0 * 1.15f;
            float botR1 = 0.40f + wave1 * 1.15f;
            float torn0 = std::sin(static_cast<float>(i) * 2.91f + monsterPosition.x) * 0.055f;
            float torn1 = std::sin(static_cast<float>(i + 1) * 2.91f + monsterPosition.z) * 0.055f;

            XMFLOAT3 pTop0 = off(std::cos(a0) * topR0, 1.52f + breathe * 0.45f, std::sin(a0) * topR0 + kMonsterSmokeBackOffset);
            XMFLOAT3 pTop1 = off(std::cos(a1) * topR1, 1.52f + breathe * 0.45f, std::sin(a1) * topR1 + kMonsterSmokeBackOffset);
            XMFLOAT3 pMid0 = off(std::cos(a0) * midR0, 0.86f + wave0 * 0.55f + breathe, std::sin(a0) * midR0 + kMonsterSmokeBackOffset);
            XMFLOAT3 pMid1 = off(std::cos(a1) * midR1, 0.86f + wave1 * 0.55f + breathe, std::sin(a1) * midR1 + kMonsterSmokeBackOffset);
            XMFLOAT3 pBot0 = off(std::cos(a0) * botR0, 0.20f + torn0, std::sin(a0) * botR0 + kMonsterSmokeBackOffset);
            XMFLOAT3 pBot1 = off(std::cos(a1) * botR1, 0.20f + torn1, std::sin(a1) * botR1 + kMonsterSmokeBackOffset);
            float amid = (a0 + a1) * 0.5f;
            XMFLOAT3 normal = Normalize3(Add3(Scale3(right, std::cos(amid)), Scale3(forward, std::sin(amid))), forward);
            XMFLOAT3 tangent = Normalize3(Add3(Scale3(right, -std::sin(amid)), Scale3(forward, std::cos(amid))), right);
            float material = smokeMaterial(static_cast<float>(i) * 0.037f + monsterPosition.x * 0.021f + monsterPosition.z * 0.013f);
            AppendDynamicQuad(transparentVerts, pBot0, pBot1, pMid1, pMid0, normal, tangent, material);
            AppendDynamicQuad(transparentVerts, pMid0, pMid1, pTop1, pTop0, normal, tangent, material + 0.017f);
        }
        for (int i = 0; i < 0; ++i) {
            float a = static_cast<float>(i) / 7.0f * kPi * 2.0f + std::sin(timeRuntime_.time * 0.43f + i) * 0.22f;
            float wobble = std::sin(timeRuntime_.time * 1.1f + i * 2.3f) * 0.06f;
            XMFLOAT3 a0 = off(std::cos(a) * (0.10f + wobble), 1.42f + breathe, std::sin(a) * (0.10f + wobble) + kMonsterSmokeBackOffset);
            XMFLOAT3 a1 = off(std::cos(a + 0.38f) * (0.56f + wobble), 0.56f, std::sin(a + 0.38f) * (0.56f + wobble) + kMonsterSmokeBackOffset);
            XMFLOAT3 a2 = off(std::cos(a + 0.74f) * (0.86f + wobble), 0.03f, std::sin(a + 0.74f) * (0.86f + wobble) + kMonsterSmokeBackOffset);
            smokeBand(a0, a1, 0.09f, 0.20f, smokeMaterial(0.41f + i * 0.061f));
            smokeBand(a1, a2, 0.14f, 0.27f, smokeMaterial(0.63f + i * 0.053f));
        }
        // Keep the body as overlapping volumetric puffs; hard bands read as cards in preview.
