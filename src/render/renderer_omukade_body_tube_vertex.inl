        const int tubeSides = debugEffectMonster ? 14 : (monsterDetail >= 2 ? 16 : (monsterDetail == 1 ? 12 : 8));
        struct TubeVertex {
            XMFLOAT3 pos;
            XMFLOAT3 normal;
            XMFLOAT3 tangent;
            XMFLOAT2 uv;
        };
        auto tubeVertex = [&](int idx, int ring) {
            float baseU = static_cast<float>(ring) / static_cast<float>(tubeSides);
            float u = std::fmod(baseU + bodyUvShift[static_cast<size_t>(idx)], 1.0f);
            float angle = u * kPi * 2.0f;
            float radius = bodyRadii[static_cast<size_t>(idx)];
            float idxF = static_cast<float>(idx);
            float frontVolume = 1.0f - SmoothStep(0.0f, 0.22f, idxF / std::max(1.0f, static_cast<float>(bodyCount - 1)));
            float surfaceLump = 1.0f + std::sin(angle * 3.0f + idxF * 0.61f + timeRuntime_.time * 1.1f) * 0.075f +
                std::sin(angle * 5.0f + idxF * 0.37f) * 0.040f;
            float ovalYScale = 0.72f + frontVolume * 0.34f + std::sin(timeRuntime_.time * 3.2f + idxF * 0.63f) * 0.040f;
            float ovalXScale = 1.06f + frontVolume * 0.18f + std::sin(timeRuntime_.time * 3.7f + idxF * 0.77f) * 0.055f;
            float ovalY = radius * surfaceLump * ovalYScale;
            float ovalX = radius * surfaceLump * ovalXScale;
            XMFLOAT3 side = bodySides[static_cast<size_t>(idx)];
            XMFLOAT3 tubeUp = bodyUps[static_cast<size_t>(idx)];
            XMFLOAT3 tangent = bodyTangents[static_cast<size_t>(idx)];
            XMFLOAT3 normal = Normalize3(Add3(Scale3(side, std::cos(angle) / std::max(0.001f, ovalXScale)),
                Scale3(tubeUp, std::sin(angle) / std::max(0.001f, ovalYScale))), tubeUp);
            XMFLOAT3 pos = Add3(bodyPoints[static_cast<size_t>(idx)],
                Add3(Scale3(side, std::cos(angle) * ovalX),
                     Scale3(tubeUp, std::sin(angle) * ovalY)));
            float v = bodyUvV[static_cast<size_t>(idx)] + std::sin(idxF * 1.31f + baseU * kPi * 2.0f) * 0.035f;
            return TubeVertex{pos, normal, tangent, {u, v}};
        };
        auto pushTubeTri = [&](const TubeVertex& a, const TubeVertex& b, const TubeVertex& c, float material) {
            solidVerts.push_back({a.pos, a.normal, a.tangent, a.uv, material});
            solidVerts.push_back({b.pos, b.normal, b.tangent, b.uv, material});
            solidVerts.push_back({c.pos, c.normal, c.tangent, c.uv, material});
        };
