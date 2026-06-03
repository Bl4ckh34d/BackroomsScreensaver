        if (propIndex == 10) {
            float panelY = ctx.wallH * 0.54f;
            float panelZ = centerZ - ctx.tileD * 0.34f;
            AddOrientedBox(vertices, indices, {centerX, panelY, panelZ}, {0.52f, 0.18f, 0.018f}, kPi, 10.0f);
            AddOrientedBox(vertices, indices, {centerX, panelY, panelZ - 0.026f}, {0.42f, 0.11f, 0.010f}, kPi, 5.0f);
            for (int slot = -3; slot <= 3; ++slot) {
                AddOrientedBox(vertices, indices,
                    {centerX, panelY + static_cast<float>(slot) * 0.030f, panelZ - 0.044f},
                    {0.36f, 0.0048f, 0.006f}, kPi, 8.0f);
            }
            cameraRuntime_.propLookPoints.push_back({centerX, panelY, panelZ});
            return;
        }
