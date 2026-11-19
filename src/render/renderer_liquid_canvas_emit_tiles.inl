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
                    auto neighborWet = [&](int nx, int ny) {
                        if (!RenderMazeView().IsOpen(nx, ny)) return false;
                        size_t nidx = static_cast<size_t>(ny * RenderMazeView().w + nx);
                        return nidx < canvas.size() && canvas[nidx].active;
                    };
                    uint32_t continueMask = 0;
                    uint32_t wetNeighborMask = 0;
                    const Tile neighbors[] = {{tx, ty - 1}, {tx, ty + 1}, {tx - 1, ty}, {tx + 1, ty}};
                    for (int edge = 0; edge < 4; ++edge) {
                        const Tile n = neighbors[edge];
                        const bool wet = neighborWet(n.x, n.y);
                        if (wet || !RenderMazeView().IsOpen(n.x, n.y)) continueMask |= 1u << edge;
                        if (wet) wetNeighborMask |= 1u << edge;
                    }
                    float ux = static_cast<float>(code);
                    // Low bits describe reach and downstream flow. Upper bits identify shared liquid edges.
                    float vy = static_cast<float>(continueMask | (surface.downstream ? 0x10u : 0u) | (wetNeighborMask << 5));
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
