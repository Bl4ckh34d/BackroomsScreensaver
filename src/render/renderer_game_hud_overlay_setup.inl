
        if (!IsPlayableSimulationMode(sessionRuntime_.mode) || !renderBuffers_.overlayBuffer ||
            !shaders_.overlayVertexShader || !shaders_.overlayPixelShader || hostRuntime_.width <= 0 || hostRuntime_.height <= 0) {
            return;
        }

        std::vector<OverlayVertex> verts;
        verts.reserve(96);
        GameWorldRenderSnapshot world = gameWorld_.BuildRenderSnapshot();
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
        auto pushBar = [&](float x, float y, float w, float h, float fill, XMFLOAT4 fillColor) {
            fill = Clamp01(fill);
            pushRect(x - 2.0f, y - 2.0f, w + 4.0f, h + 4.0f, {0.015f, 0.014f, 0.012f, 0.78f});
            pushRect(x, y, w, h, {0.03f, 0.028f, 0.024f, 0.84f});
            float fillW = std::round(w * fill);
            if (fillW > 0.5f) {
                pushRect(x, y, fillW, h, fillColor);
                pushRect(x, y, fillW, std::max(1.0f, h * 0.22f), {1.0f, 1.0f, 1.0f, 0.16f});
            }
        };
