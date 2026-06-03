        bool allowBodySurfaceClimb = monsterPreview_.active || debugEffectMonster;
        bool allowCeilingLimbContacts = monsterPreview_.active || debugEffectMonster;

        struct SurfaceContact {
            XMFLOAT3 point;
            XMFLOAT3 normal;
        };
        auto contactPoint = [&](XMFLOAT3 root, XMFLOAT3 sideDir, XMFLOAT3 tangentDir, XMFLOAT3 upDir, int limbIndex, int cycle, float maxReach) {
            Tile t = maze.TileFromWorld(root.x, root.z);
            XMFLOAT3 c = maze.WorldCenter(t, 0.0f);
            float halfW = std::max(0.12f, maze.tileW * 0.5f - 0.018f);
            float halfD = std::max(0.12f, maze.tileD * 0.5f - 0.018f);
            float crawl = static_cast<float>((limbIndex * 37 + cycle * 17) & 1023);
            sideDir = Normalize3(sideDir, right);
            tangentDir = Normalize3(tangentDir, forward);
            upDir = Normalize3(upDir, up);
            maxReach = std::max(0.20f, maxReach);
            float reach = std::clamp(maze.TileMinimum() * (0.50f + Rand01(limbIndex * 11 + cycle * 3, 719, sessionRuntime_.runtimeSeed) * 0.18f),
                maxReach * 0.50f, maxReach * 0.82f);
            float sideBias = 0.60f + Rand01(limbIndex * 13 + cycle * 5, 727, sessionRuntime_.runtimeSeed) * 0.32f;
            float tangentBias = (Rand01(limbIndex * 17 + cycle * 7, 731, sessionRuntime_.runtimeSeed) - 0.5f) * 0.58f;
            float verticalBias = (Rand01(limbIndex * 19 + cycle * 11, 733, sessionRuntime_.runtimeSeed) - 0.5f) * 0.72f;
            if (cycle % 5 == 1) verticalBias += 0.42f;
            if (cycle % 7 == 2) verticalBias -= 0.42f;
            XMFLOAT3 radial = Normalize3(Add3(Scale3(sideDir, sideBias),
                Add3(Scale3(tangentDir, tangentBias), Scale3(upDir, verticalBias))), sideDir);
            XMFLOAT3 desired = Add3(root, Scale3(radial, reach));
            desired.y = std::clamp(desired.y, 0.035f, settingsRuntime_.live.wallHeightMeters - 0.045f);
            XMFLOAT3 anchorBase = Lerp3(root, desired, 0.92f);
            float jitterA = (Rand01(limbIndex + cycle * 3, 739, sessionRuntime_.runtimeSeed) - 0.5f) * maze.TileMinimum() * 0.042f;
            float jitterB = (Rand01(limbIndex + cycle * 5, 743, sessionRuntime_.runtimeSeed) - 0.5f) * maze.TileMinimum() * 0.042f;
            float jitterY = (Rand01(limbIndex + cycle * 7, 751, sessionRuntime_.runtimeSeed) - 0.5f) * maze.TileMinimum() * 0.032f;
            float surfaceY = std::clamp(anchorBase.y + jitterY, 0.040f, settingsRuntime_.live.wallHeightMeters - 0.045f);
            std::array<XMFLOAT3, 6> candidates = {{
                {c.x - halfW, surfaceY, std::clamp(anchorBase.z + jitterA, c.z - halfD, c.z + halfD)},
                {c.x + halfW, surfaceY, std::clamp(anchorBase.z - jitterA, c.z - halfD, c.z + halfD)},
                {std::clamp(anchorBase.x + jitterB, c.x - halfW, c.x + halfW), surfaceY, c.z - halfD},
                {std::clamp(anchorBase.x - jitterB, c.x - halfW, c.x + halfW), surfaceY, c.z + halfD},
                {std::clamp(anchorBase.x + jitterB * 0.5f, c.x - halfW, c.x + halfW), 0.022f, std::clamp(anchorBase.z + jitterA * 0.5f, c.z - halfD, c.z + halfD)},
                {std::clamp(anchorBase.x - jitterB * 0.5f, c.x - halfW, c.x + halfW), settingsRuntime_.live.wallHeightMeters - 0.035f, std::clamp(anchorBase.z - jitterA * 0.5f, c.z - halfD, c.z + halfD)}
            }};
            std::array<XMFLOAT3, 6> normals = {{
                {1.0f, 0.0f, 0.0f},
                {-1.0f, 0.0f, 0.0f},
                {0.0f, 0.0f, 1.0f},
                {0.0f, 0.0f, -1.0f},
                {0.0f, 1.0f, 0.0f},
                {0.0f, -1.0f, 0.0f}
            }};
            std::array<bool, 6> usable = {{
                !maze.IsOpen(t.x - 1, t.y),
                !maze.IsOpen(t.x + 1, t.y),
                !maze.IsOpen(t.x, t.y - 1),
                !maze.IsOpen(t.x, t.y + 1),
                true,
                allowCeilingLimbContacts
            }};
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
        auto organicSegment = [&](XMFLOAT3 a, XMFLOAT3 b, float radiusA, float radiusB, float material, int sides = 9) {
            XMFLOAT3 axis = Sub3(b, a);
            float len = Length3(axis);
            if (len <= 0.001f) return;
            XMFLOAT3 tangent = Scale3(axis, 1.0f / len);
            XMFLOAT3 ref = std::abs(Dot3(tangent, up)) > 0.86f ? right : up;
            XMFLOAT3 side0 = Normalize3(Cross3(ref, tangent), right);
            XMFLOAT3 side1 = Normalize3(Cross3(tangent, side0), up);
            sides = std::clamp(sides, 4, 18);
            if (!debugEffectMonster) {
                sides = std::max(4, sides - (monsterDetail >= 2 ? 1 : (monsterDetail == 1 ? 2 : 3)));
            }
            for (int s = 0; s < sides; ++s) {
                float a0 = static_cast<float>(s) / static_cast<float>(sides) * kPi * 2.0f;
                float a1 = static_cast<float>(s + 1) / static_cast<float>(sides) * kPi * 2.0f;
                auto p = [&](XMFLOAT3 base, float angle, float r, float wobbleSeed) {
                    float lump = 1.0f + std::sin(angle * 3.0f + wobbleSeed) * 0.075f +
                        std::sin(angle * 5.0f + wobbleSeed * 0.71f) * 0.035f;
                    return Add3(base, Add3(Scale3(side0, std::cos(angle) * r * lump),
                                           Scale3(side1, std::sin(angle) * r * lump)));
                };
                XMFLOAT3 p0 = p(a, a0, radiusA, timeRuntime_.time * 1.7f + a.x * 0.3f);
                XMFLOAT3 p1 = p(a, a1, radiusA, timeRuntime_.time * 1.7f + a.x * 0.3f);
                XMFLOAT3 p2 = p(b, a1, radiusB, timeRuntime_.time * 1.9f + b.z * 0.3f);
                XMFLOAT3 p3 = p(b, a0, radiusB, timeRuntime_.time * 1.9f + b.z * 0.3f);
                XMFLOAT3 normal = Normalize3(Add3(Scale3(side0, std::cos((a0 + a1) * 0.5f)),
                                                  Scale3(side1, std::sin((a0 + a1) * 0.5f))), side0);
                AppendDynamicQuadUV(solidVerts, p0, p1, p2, p3, normal, tangent,
                    {0.0f, 0.0f}, {1.0f, 0.0f}, {1.0f, 1.0f}, {0.0f, 1.0f}, material);
            }
        };
