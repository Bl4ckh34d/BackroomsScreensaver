        effectRuntime_.steamEmitters.push_back({emitterPos, forward, PropPlacementTileHash(t.x, t.y, 45.0f + seed) * 5.0f, false});
                cameraRuntime_.propLookPoints.push_back(center);
                return;
            }
        }
        XMFLOAT3 right{std::cos(yaw), 0.0f, -std::sin(yaw)};
        XMFLOAT3 up{0.0f, 1.0f, 0.0f};
        XMFLOAT3 forward{std::sin(yaw), 0.0f, std::cos(yaw)};
        auto voff = [&](float x, float y, float z) {
            return Add3(center, OrientedOffset(right, up, forward, x, y, z));
        };
        AddOrientedBox(vertices, indices, voff(0.0f, 0.0f, 0.010f), {ventW * 0.50f, ventH * 0.50f, 0.010f}, yaw, 8.0f);
        AddOrientedBox(vertices, indices, voff(0.0f, 0.0f, 0.022f), {ventW * 0.38f, ventH * 0.31f, 0.006f}, yaw, 5.0f);
        AddOrientedBox(vertices, indices, voff(-ventW * 0.47f, 0.0f, 0.027f), {0.014f, ventH * 0.43f, 0.010f}, yaw, 8.0f);
        AddOrientedBox(vertices, indices, voff(ventW * 0.47f, 0.0f, 0.027f), {0.014f, ventH * 0.43f, 0.010f}, yaw, 8.0f);
        AddOrientedBox(vertices, indices, voff(0.0f, -ventH * 0.43f, 0.027f), {ventW * 0.45f, 0.012f, 0.010f}, yaw, 8.0f);
        AddOrientedBox(vertices, indices, voff(0.0f, ventH * 0.43f, 0.027f), {ventW * 0.45f, 0.012f, 0.010f}, yaw, 8.0f);
        for (int s = -3; s <= 3; ++s) {
            float yOff = static_cast<float>(s) * ventH * 0.115f;
            float stagger = (s & 1) ? 0.006f : -0.004f;
            AddOrientedBox(vertices, indices, voff(stagger, yOff, 0.034f + static_cast<float>(s + 3) * 0.0008f),
                {ventW * 0.36f, 0.006f, 0.007f}, yaw, 8.0f);
        }
        const XMFLOAT2 screws[] = {{-0.43f, -0.37f}, {0.43f, -0.37f}, {-0.43f, 0.37f}, {0.43f, 0.37f}};
        for (const XMFLOAT2& screw : screws) {
            AddOrientedBox(vertices, indices, voff(screw.x * ventW, screw.y * ventH, 0.039f), {0.014f, 0.014f, 0.005f}, yaw, 10.0f);
        }
        effectRuntime_.steamEmitters.push_back({voff(0.0f, lowVent ? 0.02f : -0.02f, 0.082f), forward,
            PropPlacementTileHash(t.x, t.y, 45.0f + seed) * 5.0f, false});
        cameraRuntime_.propLookPoints.push_back(center);
