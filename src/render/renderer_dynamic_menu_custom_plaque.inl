        if (menuRuntime_.customViewActive || menuRuntime_.customViewTarget) {
            MenuPlaquePlacement panel = MenuCustomPanelPlacement();
            AppendDynamicBoxAxes(verts, Add3(panel.center, Scale3(panel.inward, -0.006f)),
                panel.right, up, panel.inward, {panel.halfW, panel.halfH, 0.018f}, 21.0f);

            auto boardPart = [&](float x, float y, float z, XMFLOAT3 half, float material) {
                AppendDynamicBoxAxes(verts, Add3(panel.center, OrientedOffset(panel.right, up, panel.inward, x, y, z)),
                    panel.right, up, panel.inward, half, material);
            };
            auto appendMiteredFrame = [&](float innerW, float innerH, float border, float frontZ, float backZ, float material) {
                float outerW = innerW + border;
                float outerH = innerH + border;
                auto p = [&](float x, float y, float z) {
                    return Add3(panel.center, OrientedOffset(panel.right, up, panel.inward, x, y, z));
                };
                auto q = [&](XMFLOAT3 a, XMFLOAT3 b, XMFLOAT3 c, XMFLOAT3 d, XMFLOAT3 normal, XMFLOAT3 tangent) {
                    AppendDynamicQuad(verts, a, b, c, d, normal, tangent, material);
                };

                XMFLOAT3 frontN = panel.inward;
                XMFLOAT3 backN = Scale3(panel.inward, -1.0f);
                XMFLOAT3 leftN = Scale3(panel.right, -1.0f);
                XMFLOAT3 rightN = panel.right;
                XMFLOAT3 downN = Scale3(up, -1.0f);

                q(p(-innerW, innerH, frontZ), p(innerW, innerH, frontZ), p(outerW, outerH, frontZ), p(-outerW, outerH, frontZ), frontN, panel.right);
                q(p(innerW, -innerH, frontZ), p(outerW, -outerH, frontZ), p(outerW, outerH, frontZ), p(innerW, innerH, frontZ), frontN, panel.right);
                q(p(-outerW, -outerH, frontZ), p(outerW, -outerH, frontZ), p(innerW, -innerH, frontZ), p(-innerW, -innerH, frontZ), frontN, panel.right);
                q(p(-outerW, -outerH, frontZ), p(-innerW, -innerH, frontZ), p(-innerW, innerH, frontZ), p(-outerW, outerH, frontZ), frontN, panel.right);

                q(p(-outerW, outerH, backZ), p(outerW, outerH, backZ), p(innerW, innerH, backZ), p(-innerW, innerH, backZ), backN, Scale3(panel.right, -1.0f));
                q(p(innerW, innerH, backZ), p(outerW, outerH, backZ), p(outerW, -outerH, backZ), p(innerW, -innerH, backZ), backN, Scale3(panel.right, -1.0f));
                q(p(-innerW, -innerH, backZ), p(innerW, -innerH, backZ), p(outerW, -outerH, backZ), p(-outerW, -outerH, backZ), backN, Scale3(panel.right, -1.0f));
                q(p(-outerW, outerH, backZ), p(-innerW, innerH, backZ), p(-innerW, -innerH, backZ), p(-outerW, -outerH, backZ), backN, Scale3(panel.right, -1.0f));

                q(p(-outerW, outerH, frontZ), p(outerW, outerH, frontZ), p(outerW, outerH, backZ), p(-outerW, outerH, backZ), up, panel.right);
                q(p(outerW, -outerH, frontZ), p(-outerW, -outerH, frontZ), p(-outerW, -outerH, backZ), p(outerW, -outerH, backZ), downN, Scale3(panel.right, -1.0f));
                q(p(outerW, outerH, frontZ), p(outerW, -outerH, frontZ), p(outerW, -outerH, backZ), p(outerW, outerH, backZ), rightN, Scale3(panel.inward, -1.0f));
                q(p(-outerW, -outerH, frontZ), p(-outerW, outerH, frontZ), p(-outerW, outerH, backZ), p(-outerW, -outerH, backZ), leftN, panel.inward);

                q(p(-innerW, innerH, frontZ), p(-innerW, innerH, backZ), p(innerW, innerH, backZ), p(innerW, innerH, frontZ), downN, panel.right);
                q(p(innerW, -innerH, frontZ), p(innerW, -innerH, backZ), p(-innerW, -innerH, backZ), p(-innerW, -innerH, frontZ), up, Scale3(panel.right, -1.0f));
                q(p(innerW, innerH, frontZ), p(innerW, innerH, backZ), p(innerW, -innerH, backZ), p(innerW, -innerH, frontZ), leftN, Scale3(panel.inward, -1.0f));
                q(p(-innerW, -innerH, frontZ), p(-innerW, -innerH, backZ), p(-innerW, innerH, backZ), p(-innerW, innerH, frontZ), rightN, panel.inward);
            };
            appendMiteredFrame(panel.halfW, panel.halfH, 0.064f, 0.046f, -0.008f, 10.0f);
            boardPart(-0.17f, -panel.halfH - 0.064f, 0.064f, {panel.halfW * 0.34f, 0.014f, 0.050f}, 10.0f);
            boardPart(-0.30f, -panel.halfH - 0.042f, 0.104f, {0.155f, 0.010f, 0.010f}, 23.0f);
            boardPart(0.04f, -panel.halfH - 0.040f, 0.102f, {0.070f, 0.018f, 0.030f}, 9.0f);

            XMFLOAT3 labelCenter = Add3(panel.center, Scale3(panel.inward, 0.058f));
            XMFLOAT3 hw = Scale3(panel.right, panel.halfW * 0.965f);
            XMFLOAT3 hh = Scale3(up, panel.halfH * 0.965f);
            AppendDynamicQuadUV(transparentVerts,
                Add3(labelCenter, Add3(Scale3(hw, -1.0f), Scale3(hh, -1.0f))),
                Add3(labelCenter, Add3(hw, Scale3(hh, -1.0f))),
                Add3(labelCenter, Add3(hw, hh)),
                Add3(labelCenter, Add3(Scale3(hw, -1.0f), hh)),
                panel.inward, panel.right, {0.0f, 1.0f}, {1.0f, 1.0f}, {1.0f, 0.0f}, {0.0f, 0.0f}, 18.92f);
        }
