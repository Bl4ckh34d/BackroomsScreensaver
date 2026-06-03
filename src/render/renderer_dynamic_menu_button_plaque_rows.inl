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
