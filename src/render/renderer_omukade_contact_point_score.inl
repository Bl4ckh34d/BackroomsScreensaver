            int surfaceCycle = std::abs((limbIndex * 5 + cycle * 3 + static_cast<int>(crawl)) % 9);
            int preferred = allowCeilingLimbContacts && surfaceCycle == 0 ? 5 : (surfaceCycle <= 2 ? 4 : -1);
            float bestScore = 1.0e9f;
            XMFLOAT3 best = candidates[4];
            XMFLOAT3 bestNormal = normals[4];
            for (int i = 0; i < 6; ++i) {
                if (!usable[static_cast<size_t>(i)]) continue;
                XMFLOAT3 toRoot = Sub3(candidates[static_cast<size_t>(i)], root);
                float distToRoot = Length3(toRoot);
                float surfaceSideFit = std::max(0.0f, Dot3(normals[static_cast<size_t>(i)], sideDir));
                float score = distToRoot * distToRoot + std::abs(distToRoot - reach) * 0.055f;
                if (distToRoot > reach * 1.28f) {
                    float over = distToRoot - reach * 1.28f;
                    score += over * over * 12.0f;
                }
                if (preferred == i) score *= 0.82f;
                if (i < 4) score *= (0.92f - surfaceSideFit * 0.14f);
                if (i == 4 && !allowCeilingLimbContacts) score *= 0.72f;
                if (i == 5) score += maxReach * maxReach * 0.65f;
                if (score < bestScore) {
                    bestScore = score;
                    best = candidates[static_cast<size_t>(i)];
                    bestNormal = normals[static_cast<size_t>(i)];
                }
            }
            return SurfaceContact{best, bestNormal};
        };
