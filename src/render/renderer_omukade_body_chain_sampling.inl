        constexpr int maxBodyCount = 48;
        std::array<XMFLOAT3, maxBodyCount> bodyPoints{};
        std::array<XMFLOAT3, maxBodyCount> bodyCenterlinePoints{};
        std::array<float, maxBodyCount> bodyRadii{};
        std::array<XMFLOAT3, maxBodyCount> bodySides{};
        std::array<XMFLOAT3, maxBodyCount> bodyUps{};
        std::array<XMFLOAT3, maxBodyCount> bodyTangents{};
        std::array<float, maxBodyCount> bodyUvV{};
        std::array<float, maxBodyCount> bodyUvShift{};
        float bodySpacing = maze.TileMinimum() * (debugEffectMonster ? 0.21f : (monsterDetail >= 2 ? 0.19f : (monsterDetail == 1 ? 0.24f : 0.32f)));
        float visualBodySpacing = maze.TileMinimum() * (debugEffectMonster ? 0.19f : (monsterDetail >= 2 ? 0.17f : (monsterDetail == 1 ? 0.22f : 0.29f)));
        int bodyCount = debugEffectMonster
            ? 22
            : std::clamp(static_cast<int>(std::ceil(bodyLengthMeters / std::max(0.12f, bodySpacing))) + 1,
                monsterDetail >= 2 ? 18 : (monsterDetail == 1 ? 14 : 10),
                monsterDetail >= 2 ? 36 : (monsterDetail == 1 ? 26 : 18));
        float curiosityPose = MonsterCuriosityAmount();
        XMFLOAT3 monsterForward{std::sin(monsterYaw), 0.0f, std::cos(monsterYaw)};
        XMFLOAT3 monsterRight{std::cos(monsterYaw), 0.0f, -std::sin(monsterYaw)};
        XMFLOAT3 handSupportUp{0.0f, 0.0f, 0.0f};
        int handSupportCount = 0;
        for (const MonsterLimbAnchor& anchor : monsterPresentation_.limbAnchors) {
            XMFLOAT3 n = anchor.planted ? anchor.anchorNormal : anchor.swingToNormal;
            if (Length3(n) <= 0.001f) continue;
            n = Normalize3(n, up);
            handSupportUp = Add3(handSupportUp, n);
            ++handSupportCount;
        }
        bool hasHandSupportUp = handSupportCount >= 3 && Length3(handSupportUp) > 0.20f;
        if (hasHandSupportUp) {
            handSupportUp = Normalize3(handSupportUp, up);
            handSupportUp = Normalize3(Add3(handSupportUp, Scale3(up, 0.05f)), handSupportUp);
        }
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
        bodyUvV[0] = static_cast<float>(sessionRuntime_.runtimeSeed & 4095) * 0.0017f;
        for (int i = 1; i < bodyCount; ++i) {
            float segmentLen = Length3(Sub3(bodyPoints[static_cast<size_t>(i)], bodyPoints[static_cast<size_t>(i - 1)]));
            float seedStretch = 0.86f + Rand01(i * 53 + static_cast<int>(sessionRuntime_.runtimeSeed & 2047), 1723, sessionRuntime_.runtimeSeed) * 0.48f;
            bodyUvV[static_cast<size_t>(i)] = bodyUvV[static_cast<size_t>(i - 1)] + segmentLen * seedStretch * 0.62f;
        }
        auto bodyChainTangent = [&](int idx, XMFLOAT3 fallback) {
            if (bodyCount <= 1) return Normalize3(fallback, monsterForward);
            XMFLOAT3 tangent{};
            if (idx <= 0) {
                tangent = Sub3(bodyPoints[0], bodyPoints[1]);
            } else if (idx + 1 >= bodyCount) {
                tangent = Sub3(bodyPoints[static_cast<size_t>(idx - 1)], bodyPoints[static_cast<size_t>(idx)]);
            } else {
                tangent = Sub3(bodyPoints[static_cast<size_t>(idx - 1)], bodyPoints[static_cast<size_t>(idx + 1)]);
            }
            return Normalize3(tangent, fallback);
        };
        for (int i = 0; i < bodyCount; ++i) {
            XMFLOAT3 tangent = bodyChainTangent(i, i > 0 ? bodyTangents[static_cast<size_t>(i - 1)] : monsterForward);
            XMFLOAT3 side = Normalize3(Cross3(up, tangent), monsterRight);
            bodySides[static_cast<size_t>(i)] = side;
            bodyTangents[static_cast<size_t>(i)] = tangent;
        }
