// Liquid placement canvas.

    bool MarkLiquidCanvasTile(LiquidCanvasBuildContext& build,
                              Tile tile,
                              bool water,
                              bool ceiling,
                              uint32_t sourceMask,
                              bool centerSource,
                              bool downstream,
                              float seed,
                              float score) {
        if (!RenderMazeView().IsOpen(tile.x, tile.y) ||
            (!gEffectDebugViewer && (tile == RenderMazeView().start || tile == RenderMazeView().exit))) {
            return false;
        }
        size_t idx = WaterTileIndex(tile);
        std::vector<LiquidCanvasSurface>& canvas = LiquidCanvasVector(build, water, ceiling);
        if (idx >= canvas.size()) return false;
        LiquidCanvasSurface& surface = canvas[idx];
        surface.active = true;
        surface.sourceMask |= sourceMask & 0x0fu;
        surface.centerSource = surface.centerSource || centerSource;
        surface.downstream = surface.downstream || downstream;
        if (score >= surface.score) {
            surface.seed = seed;
            surface.score = score;
        }
        if (ceiling) {
            MarkWetCeilingTile(tile);
        } else {
            MarkWetFootstepTile(tile);
        }
        return true;
    }

    bool MarkLiquidCanvasArea(LiquidCanvasBuildContext& build,
                              float px,
                              float pz,
                              float width,
                              float depth,
                              float yaw,
                              bool water,
                              bool ceiling,
                              uint32_t sourceMask,
                              bool centerSource,
                              float seed,
                              float score,
                              bool downstream = false,
                              float sourceX = std::numeric_limits<float>::quiet_NaN(),
                              float sourceZ = std::numeric_limits<float>::quiet_NaN()) {
        if (width <= 0.02f || depth <= 0.02f) return false;
        FloorFootprint area = MakeFloorFootprint(px, pz, width, depth, yaw, 0.0f);
        if (!std::isfinite(sourceX)) sourceX = px;
        if (!std::isfinite(sourceZ)) sourceZ = pz;
        Tile sourceTile = RenderMazeView().TileFromWorld(sourceX, sourceZ);
        float cYaw = std::cos(yaw);
        float sYaw = std::sin(yaw);
        XMFLOAT3 right{cYaw, 0.0f, -sYaw};
        XMFLOAT3 forward{sYaw, 0.0f, cYaw};
        float minX = std::numeric_limits<float>::max();
        float maxX = -std::numeric_limits<float>::max();
        float minZ = std::numeric_limits<float>::max();
        float maxZ = -std::numeric_limits<float>::max();
        const float xs[] = {-width * 0.5f, width * 0.5f};
        const float zs[] = {-depth * 0.5f, depth * 0.5f};
        for (float lx : xs) {
            for (float lz : zs) {
                XMFLOAT3 p = Add3({px, 0.0f, pz}, OrientedOffset(right, {0.0f, 1.0f, 0.0f}, forward, lx, 0.0f, lz));
                minX = std::min(minX, p.x);
                maxX = std::max(maxX, p.x);
                minZ = std::min(minZ, p.z);
                maxZ = std::max(maxZ, p.z);
            }
        }
        int x0 = std::clamp(static_cast<int>(std::floor((minX - build.surface.ox) / build.surface.tileW)) - 1,
            0, std::max(0, RenderMazeView().w - 1));
        int x1 = std::clamp(static_cast<int>(std::floor((maxX - build.surface.ox) / build.surface.tileW)) + 1,
            0, std::max(0, RenderMazeView().w - 1));
        int y0 = std::clamp(static_cast<int>(std::floor((minZ - build.surface.oz) / build.surface.tileD)) - 1,
            0, std::max(0, RenderMazeView().h - 1));
        int y1 = std::clamp(static_cast<int>(std::floor((maxZ - build.surface.oz) / build.surface.tileD)) + 1,
            0, std::max(0, RenderMazeView().h - 1));
        bool marked = false;
        for (int ty = y0; ty <= y1; ++ty) {
            for (int tx = x0; tx <= x1; ++tx) {
                Tile tile{tx, ty};
                if (!RenderMazeView().IsOpen(tx, ty)) continue;
                XMFLOAT3 tc = RenderMazeView().WorldCenter(tile, 0.0f);
                FloorFootprint tileArea = MakeFloorFootprint(tc.x, tc.z,
                    build.surface.tileW * 1.002f, build.surface.tileD * 1.002f, 0.0f, 0.0f);
                if (!FloorFootprintsOverlap(area, tileArea)) continue;
                bool tileCenterSource = centerSource && tile == sourceTile;
                uint32_t tileMask = 0u;
                if (!tileCenterSource) {
                    tileMask = (tile == sourceTile)
                        ? (sourceMask & 0x0fu)
                        : LiquidSourceSideToward(tile, sourceTile);
                }
                marked = MarkLiquidCanvasTile(build, tile, water, ceiling, tileMask, tileCenterSource,
                    downstream, seed, score) || marked;
            }
        }
        return marked;
    }

    void EmitLiquidCanvasTiles(LiquidCanvasBuildContext& build) {
        auto emitSet = [&](std::vector<LiquidCanvasSurface>& canvas, bool water, bool ceiling) {
            for (int ty = 0; ty < RenderMazeView().h; ++ty) {
                for (int tx = 0; tx < RenderMazeView().w; ++tx) {
                    size_t idx = static_cast<size_t>(ty * RenderMazeView().w + tx);
                    if (idx >= canvas.size()) continue;
                    const LiquidCanvasSurface& surface = canvas[idx];
                    if (!surface.active || !RenderMazeView().IsOpen(tx, ty)) continue;
                    float l = build.surface.ox + static_cast<float>(tx) * build.surface.tileW;
                    float r = l + build.surface.tileW;
                    float z0 = build.surface.oz + static_cast<float>(ty) * build.surface.tileD;
                    float z1 = z0 + build.surface.tileD;
                    float y = ceiling ? build.ceilingY : build.floorY;
                    uint32_t code = (surface.sourceMask & 0x0fu) | (surface.centerSource ? 0x10u : 0u);
                    auto edgeContinues = [&](int nx, int ny) {
                        if (!RenderMazeView().IsOpen(nx, ny)) return true;
                        size_t nidx = static_cast<size_t>(ny * RenderMazeView().w + nx);
                        return nidx < canvas.size() && canvas[nidx].active;
                    };
                    uint32_t continueMask = 0;
                    if (edgeContinues(tx, ty - 1)) continueMask |= 1u << 0;
                    if (edgeContinues(tx, ty + 1)) continueMask |= 1u << 1;
                    if (edgeContinues(tx - 1, ty)) continueMask |= 1u << 2;
                    if (edgeContinues(tx + 1, ty)) continueMask |= 1u << 3;
                    float ux = static_cast<float>(code);
                    float vy = static_cast<float>(continueMask | (surface.downstream ? 0x10u : 0u));
                    float u0 = ux + 0.001f;
                    float u1 = ux + 0.999f;
                    float v0 = vy + 0.001f;
                    float v1 = vy + 0.999f;
                    if (water && !ceiling) y = build.waterFloorY;
                    if (ceiling) {
                        AddQuadUV(build.vertices, build.liquidIndices,
                            {l, y, z0}, {r, y, z0}, {r, y, z1}, {l, y, z1},
                            {0.0f, -1.0f, 0.0f}, {1.0f, 0.0f, 0.0f},
                            {u0, v0}, {u1, v0}, {u1, v1}, {u0, v1},
                            LiquidCanvasMaterial(water, surface.seed));
                    } else {
                        AddQuadUV(build.vertices, build.liquidIndices,
                            {l, y, z1}, {r, y, z1}, {r, y, z0}, {l, y, z0},
                            {0.0f, 1.0f, 0.0f}, {1.0f, 0.0f, 0.0f},
                            {u0, v1}, {u1, v1}, {u1, v0}, {u0, v0},
                            LiquidCanvasMaterial(water, surface.seed));
                    }
                }
            }
        };
        emitSet(build.floorBloodCanvas, false, false);
        emitSet(build.ceilingBloodCanvas, false, true);
        emitSet(build.floorWaterCanvas, true, false);
        emitSet(build.ceilingWaterCanvas, true, true);
    }
