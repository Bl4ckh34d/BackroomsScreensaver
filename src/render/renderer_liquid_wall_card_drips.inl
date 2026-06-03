        float bottomY = frame.center.y - height * 0.5f;
        if (bottomY > wallBloodFloorMargin + 0.045f) {
            int dripStrips = 1 + static_cast<int>(LampHash(seed * 41.0f + frame.center.x, frame.center.z - seed * 13.0f) * 3.0f);
            for (int strip = 0; strip < dripStrips; ++strip) {
                float r0 = LampHash(seed * 53.0f + static_cast<float>(strip) * 7.1f, frame.center.x + frame.center.z);
                float r1 = LampHash(seed * 67.0f + static_cast<float>(strip) * 11.3f, frame.center.z - frame.center.x);
                float stripW = std::min(width * (0.10f + r0 * 0.18f), 0.34f);
                float offset = (r1 - 0.5f) * std::max(0.0f, width - stripW) * 0.62f;
                float bridgeTop = std::min(bottomY + 0.12f + r0 * 0.10f, liquidBuild.wallH - wallBloodCeilingMargin);
                float bridgeBottom = wallBloodFloorMargin;
                float bridgeH = bridgeTop - bridgeBottom;
                if (bridgeH <= 0.035f) continue;
                XMFLOAT3 up{0.0f, 1.0f, 0.0f};
                XMFLOAT3 bridgeCenter = Add3(frame.center, Add3(Scale3(frame.right, offset),
                    {0.0f, (bridgeTop + bridgeBottom) * 0.5f - frame.center.y, 0.0f}));
                XMFLOAT3 ba = Add3(bridgeCenter, Add3(Scale3(frame.right, -stripW * 0.5f), Scale3(up, -bridgeH * 0.5f)));
                XMFLOAT3 bb = Add3(bridgeCenter, Add3(Scale3(frame.right,  stripW * 0.5f), Scale3(up, -bridgeH * 0.5f)));
                XMFLOAT3 bc = Add3(bridgeCenter, Add3(Scale3(frame.right,  stripW * 0.5f), Scale3(up,  bridgeH * 0.5f)));
                XMFLOAT3 bd = Add3(bridgeCenter, Add3(Scale3(frame.right, -stripW * 0.5f), Scale3(up,  bridgeH * 0.5f)));
                AddQuadUV(liquidBuild.vertices, liquidBuild.liquidIndices, ba, bb, bc, bd,
                    frame.normal, frame.right, {0, 1}, {1, 1}, {1, 0}, {0, 0},
                    LiquidSurfaceMaterial(waterLiquid,
                        0.37f + std::fmod(seed + r0 * 0.61f + static_cast<float>(strip) * 0.17f, 0.51f)));
            }
        }
