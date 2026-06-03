// Minimap and AI debug map overlay drawing helper. 
// Included inside Renderer's private section from renderer_overlays.inl.

    void DrawMapOverlay() {
        GameWorldRenderSnapshot world = gameWorld_.BuildRenderSnapshot();
        const Maze* mapMaze = world.maze;
        if ((!settingsRuntime_.live.mapOverlay && !settingsRuntime_.live.debugAiMapOverlay) || !renderBuffers_.overlayBuffer || !shaders_.overlayVertexShader || !shaders_.overlayPixelShader ||
            hostRuntime_.width <= 0 || hostRuntime_.height <= 0 || !mapMaze || mapMaze->w <= 0 || mapMaze->h <= 0 || mapMaze->open.empty()) {
            return;
        }
        const Maze& maze = *mapMaze;
        static const std::vector<Tile> kEmptyMonsterPath;
        static const std::vector<PlayerAudibleSoundPulse> kEmptySoundPulses;
        const std::vector<Tile>& monsterPath = world.monsterPath ? *world.monsterPath : kEmptyMonsterPath;
        const std::vector<PlayerAudibleSoundPulse>& soundPulses = world.playerSoundPulses ? *world.playerSoundPulses : kEmptySoundPulses;

        bool cacheValid = !mapOverlayRuntime_.cachedVerts.empty() &&
            mapOverlayRuntime_.cacheWidth == hostRuntime_.width &&
            mapOverlayRuntime_.cacheHeight == hostRuntime_.height &&
            mapOverlayRuntime_.cacheMazeW == maze.w &&
            mapOverlayRuntime_.cacheMazeH == maze.h &&
            mapOverlayRuntime_.cacheStyle == sessionRuntime_.mapOverlayStyle;
        if (cacheValid && timeRuntime_.time < mapOverlayRuntime_.nextUpdateTime) {
            DrawOverlayVertices(mapOverlayRuntime_.cachedVerts);
            return;
        }

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

        for (int y = 0; y < maze.h; ++y) {
            for (int x = 0; x < maze.w; ++x) {
                MazeWallFeature feature = maze.WallFeature(x, y);
                if (feature == MazeWallFeature::None) continue;
                Tile t{x, y};
                if (!featureDiscovered(t)) continue;
                XMFLOAT4 color = feature == MazeWallFeature::Window
                    ? XMFLOAT4{0.035f, 0.037f, 0.038f, 0.72f}
                    : XMFLOAT4{0.62f, 0.62f, 0.58f, 0.70f};
                float inset = std::max(0.16f, cell * 0.09f);
                float px = mapTileX(x);
                float py = y0 + static_cast<float>(y) * cell;
                pushRect(px + inset, py + inset, std::max(0.55f, cell - inset * 2.0f), std::max(0.55f, cell - inset * 2.0f), color);
            }
        }

        for (int y = 0; y < maze.h; ++y) {
            for (int x = 0; x < maze.w; ++x) {
                if (!maze.IsOpen(x, y)) continue;
                Tile t{x, y};
                if (playerExplorationMap && VisitCount(t) == 0 && !(t == cameraTile)) continue;
                XMFLOAT4 color{0.74f, 0.66f, 0.47f, playerExplorationMap ? 0.30f : 0.36f};
                if (t == maze.exit) color = {0.20f, 0.88f, 0.38f, 0.78f};
                float px = mapTileX(x);
                float py = y0 + static_cast<float>(y) * cell;
                float inset = std::max(0.18f, cell * 0.10f);
                pushRect(px + inset, py + inset, std::max(0.6f, cell - inset * 2.0f), std::max(0.6f, cell - inset * 2.0f), color);
            }
        }

        if (IsPlayableSimulationMode(sessionRuntime_.mode) || settingsRuntime_.live.debugAiMapOverlay) {
            for (const PlayerAudibleSoundPulse& pulse : soundPulses) {
                if (pulse.radius <= 0.05f || pulse.life <= 0.0f || pulse.age >= pulse.life) continue;
                float radiusSq = pulse.radius * pulse.radius;
                float fade = 1.0f - Clamp01(pulse.age / pulse.life);
                XMFLOAT4 hearingColor = pulse.heardByMonster
                    ? XMFLOAT4{1.0f, 0.02f, 0.02f, 0.18f + 0.30f * fade}
                    : XMFLOAT4{1.0f, 0.03f, 0.02f, 0.10f + 0.22f * fade};
                for (int y = 0; y < maze.h; ++y) {
                    for (int x = 0; x < maze.w; ++x) {
                        if (!maze.IsOpen(x, y)) continue;
                        Tile t{x, y};
                        if (playerExplorationMap && VisitCount(t) == 0 && !(t == cameraTile)) continue;
                        XMFLOAT3 center = maze.WorldCenter(t, 0.0f);
                        float dx = center.x - pulse.pos.x;
                        float dz = center.z - pulse.pos.z;
                        if (dx * dx + dz * dz <= radiusSq) {
                            pushTile(t, hearingColor, 0.02f);
                        }
                    }
                }
            }
        }

        bool drawAiDebug = settingsRuntime_.live.debugAiMapOverlay ||
            sessionRuntime_.mapOverlayStyle == GameSessionMapOverlayStyle::AiDebug;
        if (drawAiDebug) {
            bool alertPath = world.monsterHasSoundTarget || world.monsterHasLastKnownTarget || world.monsterChasingVisible;
            XMFLOAT4 pathColor = alertPath ? XMFLOAT4{1.0f, 0.12f, 0.04f, 0.72f} : XMFLOAT4{1.0f, 0.56f, 0.10f, 0.52f};
            size_t pathRemaining = monsterPath.size() > world.monsterPathIndex ? monsterPath.size() - world.monsterPathIndex : 0;
            size_t pathStep = std::max<size_t>(1, (pathRemaining + 95) / 96);
            for (size_t i = world.monsterPathIndex; i < monsterPath.size(); i += pathStep) {
                float t = monsterPath.size() > world.monsterPathIndex
                    ? static_cast<float>(i - world.monsterPathIndex) / static_cast<float>(std::max<size_t>(1, monsterPath.size() - world.monsterPathIndex))
                    : 0.0f;
                XMFLOAT4 color = pathColor;
                color.w *= Lerp(1.0f, 0.46f, Clamp01(t));
                pushTile(monsterPath[i], color, 0.26f);
            }
            if (world.monsterHeardPlayerNow) markTile(CameraTile(), {1.0f, 0.0f, 0.0f, 1.0f}, 2.35f);
            if (world.monsterHasSoundTarget) markTile(world.monsterSoundTile, {1.0f, 0.02f, 0.02f, 0.96f}, 2.05f);
            if (world.monsterHasLastKnownTarget) markTile(world.monsterLastKnownTile, {1.0f, 0.88f, 0.16f, 0.90f}, 1.85f);
            if (ValidMonsterTile(world.monsterGoal)) markTile(world.monsterGoal, {1.0f, 0.38f, 0.02f, 0.78f}, 1.62f);
            if (ValidMonsterTile(world.monsterRoamTile) && !world.monsterHasSoundTarget && !world.monsterHasLastKnownTarget) {
                markTile(world.monsterRoamTile, {0.82f, 0.68f, 0.42f, 0.56f}, 1.42f);
            }
            for (Tile occupied : MonsterBodyOccupiedTiles()) {
                markTile(occupied, {0.88f, 0.02f, 0.04f, 0.58f}, 1.34f);
            }
            markTile(MonsterTile(), {1.0f, 0.02f, 0.02f, 0.78f}, 1.55f);
        }

        markTile(cameraTile, {0.20f, 0.72f, 1.0f, 0.82f}, 1.70f);

        mapOverlayRuntime_.cachedVerts = verts;
        mapOverlayRuntime_.cacheWidth = hostRuntime_.width;
        mapOverlayRuntime_.cacheHeight = hostRuntime_.height;
        mapOverlayRuntime_.cacheMazeW = maze.w;
        mapOverlayRuntime_.cacheMazeH = maze.h;
        mapOverlayRuntime_.cacheStyle = sessionRuntime_.mapOverlayStyle;
        mapOverlayRuntime_.nextUpdateTime = timeRuntime_.time + (settingsRuntime_.live.debugAiMapOverlay ? 0.085f : 0.16f);
        DrawOverlayVertices(verts);
    }
