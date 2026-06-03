        float smoothDt = std::clamp(timeRuntime_.time - monsterPresentation_.bodySmoothTime, 0.0f, 0.05f);
        bool resetBodySmoothing = monsterPresentation_.smoothedBodyPoints.size() != static_cast<size_t>(bodyCount) ||
            monsterPresentation_.smoothedBodyUps.size() != static_cast<size_t>(bodyCount) ||
            monsterPresentation_.bodySmoothTime < -100.0f ||
            smoothDt <= 0.0001f;
        if (resetBodySmoothing) {
            monsterPresentation_.smoothedBodyPoints.assign(bodyPoints.begin(), bodyPoints.begin() + bodyCount);
            monsterPresentation_.smoothedBodyUps.assign(bodyUps.begin(), bodyUps.begin() + bodyCount);
        } else {
            for (int i = 0; i < bodyCount; ++i) {
                float follow = Lerp(14.0f, 6.0f, static_cast<float>(i) / std::max(1.0f, static_cast<float>(bodyCount - 1)));
                float alpha = 1.0f - std::exp(-smoothDt * follow);
                XMFLOAT3 target = bodyPoints[static_cast<size_t>(i)];
                XMFLOAT3 prev = monsterPresentation_.smoothedBodyPoints[static_cast<size_t>(i)];
                if (Length3(Sub3(target, prev)) > maze.TileAverage() * 1.6f) {
                    monsterPresentation_.smoothedBodyPoints[static_cast<size_t>(i)] = target;
                } else {
                    monsterPresentation_.smoothedBodyPoints[static_cast<size_t>(i)] = Lerp3(prev, target, alpha);
                }
                XMFLOAT3 prevUp = monsterPresentation_.smoothedBodyUps[static_cast<size_t>(i)];
                XMFLOAT3 targetUp = bodyUps[static_cast<size_t>(i)];
                if (Dot3(prevUp, targetUp) < 0.0f) targetUp = Scale3(targetUp, -1.0f);
                monsterPresentation_.smoothedBodyUps[static_cast<size_t>(i)] = Normalize3(Lerp3(prevUp, targetUp, 1.0f - std::exp(-smoothDt * 4.8f)), targetUp);
            }
            std::copy(monsterPresentation_.smoothedBodyPoints.begin(), monsterPresentation_.smoothedBodyPoints.begin() + bodyCount, bodyPoints.begin());
            std::copy(monsterPresentation_.smoothedBodyUps.begin(), monsterPresentation_.smoothedBodyUps.begin() + bodyCount, bodyUps.begin());
            constrainBodyChain();
            std::copy(bodyPoints.begin(), bodyPoints.begin() + bodyCount, monsterPresentation_.smoothedBodyPoints.begin());
        }
        monsterPresentation_.bodySmoothTime = timeRuntime_.time;

        for (int i = 0; i < bodyCount; ++i) {
            XMFLOAT3 tangent = bodyChainTangent(i, bodyTangents[static_cast<size_t>(i)]);
            bodyTangents[static_cast<size_t>(i)] = tangent;
            XMFLOAT3 upProjected = Normalize3(Sub3(bodyUps[static_cast<size_t>(i)], Scale3(tangent, Dot3(bodyUps[static_cast<size_t>(i)], tangent))),
                i > 0 ? bodyUps[static_cast<size_t>(i - 1)] : up);
            if (i > 0 && Dot3(upProjected, bodyUps[static_cast<size_t>(i - 1)]) < 0.0f) {
                upProjected = Scale3(upProjected, -1.0f);
            }
            bodyUps[static_cast<size_t>(i)] = upProjected;
            bodySides[static_cast<size_t>(i)] = Normalize3(Cross3(bodyUps[static_cast<size_t>(i)], tangent), bodySides[static_cast<size_t>(i)]);
        }
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
        bool renderBodyMass = true;
        if (renderBodyMass) {
            for (int i = 0; i + 1 < bodyCount; ++i) {
                for (int r = 0; r < tubeSides; ++r) {
                    TubeVertex a = tubeVertex(i, r);
                    TubeVertex b = tubeVertex(i, r + 1);
                    TubeVertex c = tubeVertex(i + 1, r + 1);
                    TubeVertex d = tubeVertex(i + 1, r);
                    pushTubeTri(a, b, c, gutMat);
                    pushTubeTri(a, c, d, gutMat);
                }
            }
        }
        int thoraxIndex = std::min(2, bodyCount - 1);
        int abdomenIndex = std::min(5, bodyCount - 1);
        XMFLOAT3 thorax = Add3(bodyPoints[static_cast<size_t>(thoraxIndex)],
            Scale3(bodyTangents[static_cast<size_t>(thoraxIndex)], 0.045f * modelXZ));
        XMFLOAT3 abdomen = Add3(bodyPoints[static_cast<size_t>(abdomenIndex)],
            Scale3(bodyTangents[static_cast<size_t>(abdomenIndex)], -0.045f * modelXZ));
        if (renderBodyMass) {
            AppendDynamicEllipsoid(solidVerts, thorax,
                bodySides[static_cast<size_t>(thoraxIndex)], bodyUps[static_cast<size_t>(thoraxIndex)], bodyTangents[static_cast<size_t>(thoraxIndex)],
                {0.50f * modelXZ, 0.32f * modelY, 0.54f * modelXZ}, debugEffectMonster ? 18 : 26, debugEffectMonster ? 8 : 12, gutMat + 0.020f);
            AppendDynamicEllipsoid(solidVerts, abdomen,
                bodySides[static_cast<size_t>(abdomenIndex)], bodyUps[static_cast<size_t>(abdomenIndex)], bodyTangents[static_cast<size_t>(abdomenIndex)],
                {0.70f * modelXZ, 0.43f * modelY, 0.92f * modelXZ}, debugEffectMonster ? 20 : 30, debugEffectMonster ? 9 : 14, gutMat + 0.055f);
            AppendDynamicEllipsoid(solidVerts, Add3(abdomen, Scale3(bodyTangents[static_cast<size_t>(abdomenIndex)], -0.34f * modelXZ)),
                bodySides[static_cast<size_t>(abdomenIndex)], bodyUps[static_cast<size_t>(abdomenIndex)], bodyTangents[static_cast<size_t>(abdomenIndex)],
                {0.43f * modelXZ, 0.30f * modelY, 0.38f * modelXZ}, debugEffectMonster ? 16 : 22, debugEffectMonster ? 7 : 10, gutMat + 0.075f);
            for (int i = bodyCount - 1; i >= 0; --i) {
                if (i != 0 && (i % 3) != 1) continue;
                float fi = static_cast<float>(i);
                float centerWeight = i == 0 ? 1.0f : Clamp01(1.0f - Length3(Sub3(bodyPoints[static_cast<size_t>(i)], bodyPoints[0])) / std::max(0.1f, maze.TileMinimum() * 1.18f));
                XMFLOAT3 p = bodyPoints[static_cast<size_t>(i)];
                float breatheScale = 1.0f + std::sin(timeRuntime_.time * 2.15f + fi * 0.91f) * 0.034f;
                float secondaryScale = i == 0 ? 1.0f : Lerp(0.78f, 0.56f, Clamp01(fi / static_cast<float>(bodyCount - 1)));
                XMFLOAT3 blobRadii{
                    (0.27f + centerWeight * 0.14f) * modelXZ * breatheScale * secondaryScale,
                    (0.22f + centerWeight * 0.10f) * modelY * breatheScale * secondaryScale,
                    (0.34f + centerWeight * 0.18f) * modelXZ * breatheScale * secondaryScale
                };
                AppendDynamicEllipsoid(solidVerts, p,
                    bodySides[static_cast<size_t>(i)], bodyUps[static_cast<size_t>(i)], bodyTangents[static_cast<size_t>(i)],
                    blobRadii, debugEffectMonster ? 14 : 22, debugEffectMonster ? 7 : 11, gutMat + std::fmod(fi * 0.011f, 0.10f));
            }
        }
