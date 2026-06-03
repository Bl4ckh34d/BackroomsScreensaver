// Dynamic exit-door and main-menu plaque geometry helpers. 
// Included inside Renderer's private section from renderer_dynamic_geometry.inl.

    void AppendDynamicDoor(std::vector<Vertex>& verts) {
        float halfW = 0.60f;
        float halfH = 1.05f;
        XMFLOAT3 up{0.0f, 1.0f, 0.0f};
        float angle = sessionRuntime_.mode == RendererRuntimeMode::MainMenu
            ? std::min(exitDoorPresentation_.angle * 1.12f, 1.54f)
            : exitDoorPresentation_.angle;
        XMFLOAT3 right = RotateYVec(exitDoorPresentation_.right, angle);
        XMFLOAT3 normal = RotateYVec(exitDoorPresentation_.normal, angle);
        XMFLOAT3 center = Add3(exitDoorPresentation_.hinge, Add3(Scale3(right, halfW), Scale3(normal, 0.012f)));
        AppendDynamicBoxAxes(verts, center, right, up, normal, {halfW, halfH, 0.030f}, 6.0f);

        auto localCenter = [&](float x, float y, float z) {
            return Add3(center, OrientedOffset(right, up, normal, x, y, z));
        };

        auto handle = [&](float zSign) {
            XMFLOAT3 plateCenter = localCenter(halfW * 0.63f, -0.08f, zSign * 0.044f);
            AppendDynamicBoxAxes(verts, plateCenter, right, up, normal, {0.050f, 0.115f, 0.005f}, 10.0f);
            XMFLOAT3 neckCenter = localCenter(halfW * 0.63f, -0.08f, zSign * 0.062f);
            AppendDynamicBoxAxes(verts, neckCenter, right, up, normal, {0.030f, 0.026f, 0.018f}, 10.0f);
            XMFLOAT3 leverCenter = localCenter(halfW * 0.63f - 0.090f, -0.08f, zSign * 0.076f);
            AppendDynamicBoxAxes(verts, leverCenter, right, up, normal, {0.125f, 0.015f, 0.016f}, 10.0f);
            XMFLOAT3 leverTip = localCenter(halfW * 0.63f - 0.216f, -0.08f, zSign * 0.076f);
            AppendDynamicBoxAxes(verts, leverTip, right, up, normal, {0.010f, 0.020f, 0.018f}, 10.0f);
        };
        handle(1.0f);
        handle(-1.0f);

    }

    void AppendMenuDoorwayLight(std::vector<Vertex>& transparentVerts) {
        if (sessionRuntime_.mode != RendererRuntimeMode::MainMenu) return;
        float openT = SmoothStep(0.24f, 1.30f, exitDoorPresentation_.angle);
        openT *= openT;
        if (openT <= 0.001f) return;

        XMFLOAT3 up{0.0f, 1.0f, 0.0f};
        XMFLOAT3 right = Normalize3(exitDoorPresentation_.right, {1.0f, 0.0f, 0.0f});
        XMFLOAT3 inward = Normalize3(exitDoorPresentation_.normal, {0.0f, 0.0f, 1.0f});
        float openMat = 19.54f + openT * 0.12f;

        XMFLOAT3 aperture = Add3(exitDoorPresentation_.center, Add3(Scale3(inward, 0.66f), {0.0f, -0.46f, 0.0f}));
        XMFLOAT3 floorHit = Add3(exitDoorPresentation_.center, Add3(Scale3(inward, 2.95f), {0.0f, -0.92f, 0.0f}));
        XMFLOAT3 farDust = Add3(exitDoorPresentation_.center, Add3(Scale3(inward, 3.90f), {0.0f, -0.72f, 0.0f}));
        XMFLOAT3 nearSide = Scale3(right, 0.38f);
        XMFLOAT3 floorSide = Scale3(right, 1.46f);
        XMFLOAT3 dustSide = Scale3(right, 0.86f);
        AppendDynamicQuadUV(transparentVerts,
            Add3(aperture, Scale3(nearSide, -1.0f)),
            Add3(aperture, nearSide),
            Add3(floorHit, floorSide),
            Add3(floorHit, Scale3(floorSide, -1.0f)),
            up, right, {0, 1}, {1, 1}, {1, 0}, {0, 0}, openMat + 0.035f);
        AppendDynamicQuadUV(transparentVerts,
            Add3(aperture, Add3(Scale3(dustSide, -0.72f), Scale3(up, -0.06f))),
            Add3(aperture, Add3(Scale3(dustSide, 0.72f), Scale3(up, -0.06f))),
            Add3(farDust, Add3(Scale3(dustSide, 1.0f), Scale3(up, 0.10f))),
            Add3(farDust, Add3(Scale3(dustSide, -1.0f), Scale3(up, 0.10f))),
            up, right, {0, 1}, {1, 1}, {1, 0}, {0, 0}, openMat + 0.050f);
    }

    void AppendMenuButtonPlaques(std::vector<Vertex>& verts, std::vector<Vertex>& transparentVerts) {
        if (sessionRuntime_.mode != RendererRuntimeMode::MainMenu) return;
        if (menuRuntime_.startTransitionActive || menuRuntime_.startTransitionComplete) return;
        XMFLOAT3 up{0.0f, 1.0f, 0.0f};
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
        for (int i = 0; i < menuRuntime_.buttonCount; ++i) {
            if (menuRuntime_.customViewActive || menuRuntime_.customViewTarget) break;
            bool hover = menuRuntime_.hoverButtonIndex == i;
            float material = hover ? 9.88f : 9.68f;
            MenuPlaquePlacement plaque = MenuButtonPlacement(i);
            AppendDynamicBoxAxes(verts, plaque.center, plaque.right, up, plaque.inward, {plaque.halfW, plaque.halfH, 0.030f}, material);
            XMFLOAT3 capCenter = Add3(plaque.center, Add3(Scale3(plaque.right, -plaque.halfW + 0.012f), Scale3(plaque.inward, -0.017f)));
            AppendDynamicBoxAxes(verts, capCenter, plaque.right, up, plaque.inward, {0.016f, plaque.halfH + 0.010f, 0.024f}, hover ? 9.94f : 9.78f);
            XMFLOAT3 labelCenter = Add3(plaque.center, Scale3(plaque.inward, 0.036f));
            XMFLOAT3 hw = Scale3(plaque.right, std::min(plaque.halfW * 0.72f, 0.76f));
            XMFLOAT3 hh = Scale3(up, 0.096f);
            int labelIndex = std::clamp(menuRuntime_.buttonLabelRows[static_cast<size_t>(i)], 0, 5);
            constexpr float kMenuLabelRows = 6.0f;
            float v0 = (static_cast<float>(labelIndex) + 0.18f) / kMenuLabelRows;
            float v1 = (static_cast<float>(labelIndex) + 0.82f) / kMenuLabelRows;
            float labelMaterial = 18.0f + (hover ? 0.46f : 0.08f);
            AppendDynamicQuadUV(transparentVerts,
                Add3(labelCenter, Add3(Scale3(hw, -1.0f), Scale3(hh, -1.0f))),
                Add3(labelCenter, Add3(hw, Scale3(hh, -1.0f))),
                Add3(labelCenter, Add3(hw, hh)),
                Add3(labelCenter, Add3(Scale3(hw, -1.0f), hh)),
                plaque.inward, plaque.right, {0.08f, v1}, {0.92f, v1}, {0.92f, v0}, {0.08f, v0}, labelMaterial);
        }
    }
