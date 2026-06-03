        auto surfaceAxes = [&](XMFLOAT3 normal, XMFLOAT3& axisX, XMFLOAT3& axisY) {
            normal = Normalize3(normal, up);
            if (std::abs(normal.y) > 0.82f) {
                axisX = right;
                axisY = Normalize3(Cross3(normal, axisX), forward);
            } else {
                axisX = Normalize3(Cross3(up, normal), right);
                axisY = up;
            }
        };
        auto appendBloodHandprint = [&](const MonsterHandprint& hp) {
            XMFLOAT3 n = Normalize3(hp.normal, up);
            XMFLOAT3 axisX{};
            XMFLOAT3 axisY{};
            surfaceAxes(n, axisX, axisY);
            float w = hp.size * 0.92f;
            float h = hp.size * (1.18f + std::fmod(hp.seed * 23.0f, 0.36f));
            XMFLOAT3 center = Add3(hp.pos, Scale3(n, 0.0035f));
            XMFLOAT3 hx = Scale3(axisX, w);
            XMFLOAT3 hy = Scale3(axisY, h);
            float mat = handprintMat + std::fmod(hp.seed * 0.137f, 0.48f);
            AppendDynamicQuadUV(transparentVerts,
                Add3(center, Add3(Scale3(hx, -1.0f), Scale3(hy, -1.0f))),
                Add3(center, Add3(hx, Scale3(hy, -1.0f))),
                Add3(center, Add3(hx, hy)),
                Add3(center, Add3(Scale3(hx, -1.0f), hy)),
                n, axisX, {0, 1}, {1, 1}, {1, 0}, {0, 0}, mat);
        };
        auto recordHandprint = [&](XMFLOAT3, XMFLOAT3, int, int) {
        };
        monsterPresentation_.handprints.clear();

        auto smokeMaterial = [&](float seed) {
            return 11.08f + std::fmod(std::abs(seed), 0.34f);
        };
        auto smokeBand = [&](XMFLOAT3 a, XMFLOAT3 b, float halfA, float halfB, float material) {
            XMFLOAT3 axis = Sub3(b, a);
            float len = Length3(axis);
            if (len <= 0.001f) return;
            XMFLOAT3 axisN = Scale3(axis, 1.0f / len);
            XMFLOAT3 mid = Lerp3(a, b, 0.5f);
            XMFLOAT3 toCam = Normalize3(Sub3(playerPosition, mid), forward);
            XMFLOAT3 side = Normalize3(Cross3(axisN, toCam), right);
            XMFLOAT3 normal = Normalize3(Cross3(side, axisN), toCam);
            AppendDynamicQuad(transparentVerts,
                Add3(a, Scale3(side, -halfA * modelXZ)),
                Add3(a, Scale3(side, halfA * modelXZ)),
                Add3(b, Scale3(side, halfB * modelXZ)),
                Add3(b, Scale3(side, -halfB * modelXZ)),
                normal, side, material);
        };
        auto smokePuff = [&](float x, float y, float z, float halfW, float halfH, float material) {
            XMFLOAT3 center = off(x, y, z);
            XMFLOAT3 toCam = Normalize3(Sub3(playerPosition, center), forward);
            XMFLOAT3 side = Normalize3(Cross3(up, toCam), right);
            XMFLOAT3 puffUp = Normalize3(Cross3(toCam, side), up);
            XMFLOAT3 hw = Scale3(side, halfW * modelXZ);
            XMFLOAT3 hh = Scale3(puffUp, halfH * modelY);
            AppendDynamicQuad(transparentVerts,
                Add3(center, Add3(Scale3(hw, -1.0f), Scale3(hh, -1.0f))),
                Add3(center, Add3(hw, Scale3(hh, -1.0f))),
                Add3(center, Add3(hw, hh)),
                Add3(center, Add3(Scale3(hw, -1.0f), hh)),
                toCam, side, material);
        };
