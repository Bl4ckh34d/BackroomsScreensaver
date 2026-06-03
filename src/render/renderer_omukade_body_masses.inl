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
