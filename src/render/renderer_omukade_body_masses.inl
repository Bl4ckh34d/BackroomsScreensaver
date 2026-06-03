        int thoraxIndex = std::min(2, bodyCount - 1);
        int abdomenIndex = std::min(5, bodyCount - 1);
        XMFLOAT3 thorax = Add3(bodyPoints[static_cast<size_t>(thoraxIndex)],
            Scale3(bodyTangents[static_cast<size_t>(thoraxIndex)], 0.045f * modelXZ));
        XMFLOAT3 abdomen = Add3(bodyPoints[static_cast<size_t>(abdomenIndex)],
            Scale3(bodyTangents[static_cast<size_t>(abdomenIndex)], -0.045f * modelXZ));
        if (renderBodyMass) {
            auto detailSlices = [&](int high, int medium, int low) {
                return debugEffectMonster ? std::max(low, medium) : (monsterDetail >= 2 ? high : (monsterDetail == 1 ? medium : low));
            };
            AppendDynamicEllipsoid(solidVerts, thorax,
                bodySides[static_cast<size_t>(thoraxIndex)], bodyUps[static_cast<size_t>(thoraxIndex)], bodyTangents[static_cast<size_t>(thoraxIndex)],
                {0.50f * modelXZ, 0.32f * modelY, 0.54f * modelXZ},
                detailSlices(26, 20, 14), detailSlices(12, 9, 7), gutMat + 0.020f);
            AppendDynamicEllipsoid(solidVerts, abdomen,
                bodySides[static_cast<size_t>(abdomenIndex)], bodyUps[static_cast<size_t>(abdomenIndex)], bodyTangents[static_cast<size_t>(abdomenIndex)],
                {0.70f * modelXZ, 0.43f * modelY, 0.92f * modelXZ},
                detailSlices(30, 22, 16), detailSlices(14, 10, 8), gutMat + 0.055f);
            AppendDynamicEllipsoid(solidVerts, Add3(abdomen, Scale3(bodyTangents[static_cast<size_t>(abdomenIndex)], -0.34f * modelXZ)),
                bodySides[static_cast<size_t>(abdomenIndex)], bodyUps[static_cast<size_t>(abdomenIndex)], bodyTangents[static_cast<size_t>(abdomenIndex)],
                {0.43f * modelXZ, 0.30f * modelY, 0.38f * modelXZ},
                detailSlices(22, 16, 12), detailSlices(10, 8, 6), gutMat + 0.075f);
            for (int i = bodyCount - 1; i >= 0; --i) {
                int blobStride = monsterDetail >= 2 ? 3 : (monsterDetail == 1 ? 4 : 5);
                if (i != 0 && (i % blobStride) != 1) continue;
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
                    blobRadii, detailSlices(22, 14, 10), detailSlices(11, 7, 5), gutMat + std::fmod(fi * 0.011f, 0.10f));
            }
        }
