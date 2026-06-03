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
