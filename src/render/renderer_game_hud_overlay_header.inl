// Playable HUD and dread meter overlay helpers. 
// Included inside Renderer's private section from renderer_overlays.inl.

    void DrawDreadMeterOverlay() {
        if (!settingsRuntime_.live.dreadEnabled || !settingsRuntime_.live.dreadDebugMeter || !renderBuffers_.overlayBuffer ||
            !shaders_.overlayVertexShader || !shaders_.overlayPixelShader || hostRuntime_.width <= 0 || hostRuntime_.height <= 0) {
            return;
        }

        std::vector<OverlayVertex> verts;
        verts.reserve(48);
        auto ndcX = [&](float px) { return px / static_cast<float>(hostRuntime_.width) * 2.0f - 1.0f; };
        auto ndcY = [&](float py) { return 1.0f - py / static_cast<float>(hostRuntime_.height) * 2.0f; };
        auto pushRect = [&](float x, float y, float w, float h, XMFLOAT4 color) {
            float l = ndcX(x);
            float r = ndcX(x + w);
            float t = ndcY(y);
            float b = ndcY(y + h);
            verts.push_back({{l, b}, color});
            verts.push_back({{l, t}, color});
            verts.push_back({{r, t}, color});
            verts.push_back({{l, b}, color});
            verts.push_back({{r, t}, color});
            verts.push_back({{r, b}, color});
        };

        float x = 18.0f;
        float y = 18.0f;
        float w = 170.0f;
        float h = 13.0f;
        float fill = std::round((w - 4.0f) * Clamp01(viewRuntime_.dreadMeterLevel));
        pushRect(x - 2.0f, y - 2.0f, w + 4.0f, h + 4.0f, {0.07f, 0.025f, 0.025f, 0.74f});
        pushRect(x, y, w, h, {0.015f, 0.008f, 0.008f, 0.86f});
        if (fill > 0.5f) {
            pushRect(x + 2.0f, y + 2.0f, fill, h - 4.0f, {0.72f, 0.055f, 0.042f, 0.93f});
            pushRect(x + 2.0f, y + 2.0f, fill, 2.0f, {1.0f, 0.20f, 0.13f, 0.38f});
        }
        pushRect(x - 2.0f, y - 2.0f, w + 4.0f, 1.0f, {0.40f, 0.08f, 0.07f, 0.86f});
        pushRect(x - 2.0f, y + h + 1.0f, w + 4.0f, 1.0f, {0.23f, 0.035f, 0.035f, 0.86f});
        pushRect(x - 2.0f, y - 2.0f, 1.0f, h + 4.0f, {0.33f, 0.055f, 0.052f, 0.86f});
        pushRect(x + w + 1.0f, y - 2.0f, 1.0f, h + 4.0f, {0.33f, 0.055f, 0.052f, 0.86f});

        DrawOverlayVertices(verts);
    }
