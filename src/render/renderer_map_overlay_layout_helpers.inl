        std::vector<OverlayVertex> verts;
        verts.reserve(std::min(static_cast<size_t>(kOverlayVertexCapacity),
            static_cast<size_t>(maze.w * maze.h * 12 + monsterPath.size() * 6 + 192)));
        auto ndcX = [&](float px) { return px / static_cast<float>(hostRuntime_.width) * 2.0f - 1.0f; };
        auto ndcY = [&](float py) { return 1.0f - py / static_cast<float>(hostRuntime_.height) * 2.0f; };
        auto pushRect = [&](float x, float y, float w, float h, XMFLOAT4 color) {
            if (verts.size() + 6 > static_cast<size_t>(kOverlayVertexCapacity)) return;
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

        float maxW = std::clamp(static_cast<float>(hostRuntime_.width) * 0.22f, 125.0f, 245.0f);
        float maxH = std::clamp(static_cast<float>(hostRuntime_.height) * 0.22f, 110.0f, 210.0f);
        float cell = std::max(1.15f, std::min(maxW / static_cast<float>(maze.w), maxH / static_cast<float>(maze.h)));
        float mapW = cell * static_cast<float>(maze.w);
        float mapH = cell * static_cast<float>(maze.h);
        float pad = 7.0f;
        float x0 = static_cast<float>(hostRuntime_.width) - mapW - pad - 18.0f;
        float y0 = static_cast<float>(hostRuntime_.height) - mapH - pad - 18.0f;
        Tile cameraTile = CameraTile();
        bool playerExplorationMap = sessionRuntime_.mapOverlayStyle == GameSessionMapOverlayStyle::PlayerExploration &&
            !settingsRuntime_.live.debugAiMapOverlay;

        pushRect(x0 - pad, y0 - pad, mapW + pad * 2.0f, mapH + pad * 2.0f, {0.0f, 0.0f, 0.0f, 0.32f});
        pushRect(x0 - 1.0f, y0 - 1.0f, mapW + 2.0f, mapH + 2.0f, {0.53f, 0.46f, 0.31f, 0.24f});
        pushRect(x0, y0, mapW, mapH, {0.025f, 0.022f, 0.017f, 0.44f});
        auto mapTileX = [&](int tileX) {
            return x0 + static_cast<float>(maze.w - 1 - tileX) * cell;
        };
        auto pushTile = [&](Tile t, XMFLOAT4 color, float insetScale = 0.14f) {
            if (!maze.InBounds(t.x, t.y)) return;
            float inset = std::max(0.12f, cell * insetScale);
            float px = mapTileX(t.x);
            float py = y0 + static_cast<float>(t.y) * cell;
            pushRect(px + inset, py + inset, std::max(0.45f, cell - inset * 2.0f), std::max(0.45f, cell - inset * 2.0f), color);
        };
        auto markTile = [&](Tile t, XMFLOAT4 color, float scale) {
            if (!maze.InBounds(t.x, t.y)) return;
            float size = std::max(3.0f, cell * scale);
            float px = mapTileX(t.x) + cell * 0.5f - size * 0.5f;
            float py = y0 + (static_cast<float>(t.y) + 0.5f) * cell - size * 0.5f;
            pushRect(px, py, size, size, color);
        };
        auto featureDiscovered = [&](Tile t) {
            if (!playerExplorationMap) return true;
            const Tile neighbors[] = {{t.x + 1, t.y}, {t.x - 1, t.y}, {t.x, t.y + 1}, {t.x, t.y - 1}};
            for (Tile n : neighbors) {
                if (!maze.IsOpen(n.x, n.y)) continue;
                if (VisitCount(n) > 0 || n == cameraTile) return true;
            }
            return false;
        };
