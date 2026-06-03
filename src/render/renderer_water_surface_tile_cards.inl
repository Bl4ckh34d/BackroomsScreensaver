// Water surface placement tile cards.

    void EmitWaterTileCard(WaterSurfaceBuildContext& build, Tile t, bool ceiling, const WaterTileSurface& surface) {
        if (!surface.active || surface.suppressCard) return;

        const float tileW = build.surface.tileW;
        const float tileD = build.surface.tileD;
        float l = build.surface.ox + static_cast<float>(t.x) * tileW;
        float r = l + tileW;
        float z0 = build.surface.oz + static_cast<float>(t.y) * tileD;
        float z1 = z0 + tileD;
        float y = ceiling ? build.ceilingY : build.floorLift;
        float u = static_cast<float>(surface.side);
        int neighborMask = 0;
        auto neighborWet = [&](int nx, int ny) {
            if (!RenderMazeView().IsOpen(nx, ny)) return false;
            const WaterTileSurface& neighbor =
                (ceiling ? build.ceilingWaterTiles : build.floorWaterTiles)[static_cast<size_t>(ny * RenderMazeView().w + nx)];
            return neighbor.active && !neighbor.suppressCard;
        };
        auto neighborOpen = [&](int nx, int ny) {
            return RenderMazeView().IsOpen(nx, ny);
        };
        if (neighborWet(t.x, t.y - 1)) neighborMask |= 1;
        if (neighborWet(t.x, t.y + 1)) neighborMask |= 2;
        if (neighborWet(t.x - 1, t.y)) neighborMask |= 4;
        if (neighborWet(t.x + 1, t.y)) neighborMask |= 8;
        if (neighborWet(t.x - 1, t.y - 1)) neighborMask |= 16;
        if (neighborWet(t.x + 1, t.y - 1)) neighborMask |= 32;
        if (neighborWet(t.x - 1, t.y + 1)) neighborMask |= 64;
        if (neighborWet(t.x + 1, t.y + 1)) neighborMask |= 128;
        float vMode = static_cast<float>(surface.mode + neighborMask * 8);
        float material = WaterDecalMaterial(surface.seed, 0.0f, 0.014f);
        float h0 = LampHash(surface.seed * 17.0f + static_cast<float>(t.x), static_cast<float>(t.y) + 3.1f);
        float h1 = LampHash(surface.seed * 23.0f - static_cast<float>(t.y), static_cast<float>(t.x) + 5.7f);
        float h2 = LampHash(surface.seed * 31.0f + static_cast<float>(t.x) * 0.5f, static_cast<float>(t.y) * 0.5f);
        float h3 = LampHash(surface.seed * 41.0f + static_cast<float>(t.x) * 1.7f, static_cast<float>(t.y) * 2.3f);
        float sizeScore = std::clamp(0.70f + (surface.score - 0.75f) * 0.22f, 0.54f, 1.05f);
        float halfW = tileW * (ceiling ? (0.30f + h0 * 0.20f) : (0.20f + h0 * 0.17f)) * sizeScore;
        float halfD = tileD * (ceiling ? (0.30f + h1 * 0.20f) : (0.18f + h1 * 0.16f)) * sizeScore;
        if (surface.mode == 3) {
            halfW *= 0.62f;
            halfD *= 0.62f;
        }
        float cx = (l + r) * 0.5f + (h2 - 0.5f) * tileW * (ceiling ? 0.18f : 0.24f);
        float cz = (z0 + z1) * 0.5f + (h3 - 0.5f) * tileD * (ceiling ? 0.18f : 0.22f);
        auto pullTowardSide = [&](int side, float amount) {
            if (side == 0) cz -= tileD * amount;
            else if (side == 1) cz += tileD * amount;
            else if (side == 2) cx -= tileW * amount;
            else cx += tileW * amount;
        };
        if (surface.mode > 0 && surface.mode != 3) {
            pullTowardSide(surface.side, ceiling ? 0.16f : 0.13f);
            if (surface.side == 0 || surface.side == 1) halfD = std::max(halfD, tileD * (ceiling ? 0.43f : 0.36f));
            else halfW = std::max(halfW, tileW * (ceiling ? 0.43f : 0.36f));
        }
        float wetOverlapZ = tileD * (ceiling ? 0.010f : 0.030f);
        float wetOverlapX = tileW * (ceiling ? 0.010f : 0.030f);
        float floorSeepZ = ceiling ? 0.0f : tileD * 0.085f;
        float floorSeepX = ceiling ? 0.0f : tileW * 0.085f;
        if (neighborMask & 1) z0 -= wetOverlapZ;
        else if (!ceiling && neighborOpen(t.x, t.y - 1)) z0 = std::max(z0 - floorSeepZ, cz - halfD);
        else z0 = std::max(z0 + tileD * 0.055f, cz - halfD);
        if (neighborMask & 2) z1 += wetOverlapZ;
        else if (!ceiling && neighborOpen(t.x, t.y + 1)) z1 = std::min(z1 + floorSeepZ, cz + halfD);
        else z1 = std::min(z1 - tileD * 0.055f, cz + halfD);
        if (neighborMask & 4) l -= wetOverlapX;
        else if (!ceiling && neighborOpen(t.x - 1, t.y)) l = std::max(l - floorSeepX, cx - halfW);
        else l = std::max(l + tileW * 0.055f, cx - halfW);
        if (neighborMask & 8) r += wetOverlapX;
        else if (!ceiling && neighborOpen(t.x + 1, t.y)) r = std::min(r + floorSeepX, cx + halfW);
        else r = std::min(r - tileW * 0.055f, cx + halfW);
        if (r - l < tileW * 0.12f || z1 - z0 < tileD * 0.12f) return;

        if (ceiling) {
            AddQuadUV(build.vertices, build.waterIndices,
                {l, y, z0}, {r, y, z0}, {r, y, z1}, {l, y, z1},
                {0, -1, 0}, {1, 0, 0},
                {u, vMode}, {u + 1.0f, vMode}, {u + 1.0f, vMode + 1.0f}, {u, vMode + 1.0f}, material);
        } else {
            AddQuadUV(build.vertices, build.waterIndices,
                {l, y, z1}, {r, y, z1}, {r, y, z0}, {l, y, z0},
                {0, 1, 0}, {1, 0, 0},
                {u, vMode}, {u + 1.0f, vMode}, {u + 1.0f, vMode + 1.0f}, {u, vMode + 1.0f}, material);
            MarkWetFootstepArea((l + r) * 0.5f, (z0 + z1) * 0.5f, r - l, z1 - z0, 0.0f);
        }
    }

    void EmitFloorWaterBridge(WaterSurfaceBuildContext& build, Tile t, int side) {
        Tile n = NeighborForMazeSide(t, side);
        if (!RenderMazeView().IsOpen(t.x, t.y) || !RenderMazeView().IsOpen(n.x, n.y)) return;
        const WaterTileSurface& aSurface = build.floorWaterTiles[WaterTileIndex(t)];
        const WaterTileSurface& bSurface = build.floorWaterTiles[WaterTileIndex(n)];
        if (!aSurface.active || !bSurface.active) return;
        if (aSurface.suppressCard || bSurface.suppressCard) return;

        const float tileW = build.surface.tileW;
        const float tileD = build.surface.tileD;
        float l = build.surface.ox + static_cast<float>(t.x) * tileW;
        float r = l + tileW;
        float z0 = build.surface.oz + static_cast<float>(t.y) * tileD;
        float z1 = z0 + tileD;
        constexpr float bridgeLift = 0.0018f;
        float y = build.floorLift + bridgeLift;
        float material = WaterDecalMaterial((aSurface.seed + bSurface.seed) * 0.5f + static_cast<float>(side) * 0.071f, 0.0f, 0.014f);
        float seed = (aSurface.seed + bSurface.seed) * 0.5f + static_cast<float>(side) * 0.173f;
        float h0 = LampHash(seed * 17.0f + static_cast<float>(t.x), static_cast<float>(t.y));
        float h1 = LampHash(seed * 23.0f - static_cast<float>(t.y), static_cast<float>(t.x));
        float h2 = LampHash(seed * 31.0f + static_cast<float>(t.x) * 0.5f, static_cast<float>(t.y) * 0.5f);

        if (side == 1) {
            float seamZ = z1;
            float depth = std::min(tileD * (0.18f + h0 * 0.08f), 0.38f);
            float span = tileW * (0.92f + h1 * 0.08f);
            float cx = (l + r) * 0.5f + (h2 - 0.5f) * std::max(0.0f, tileW - span) * 0.72f;
            float x0 = std::max(l + tileW * 0.006f, cx - span * 0.5f);
            float x1 = std::min(r - tileW * 0.006f, cx + span * 0.5f);
            AddQuadUV(build.vertices, build.waterIndices,
                {x0, y, seamZ + depth * 0.5f},
                {x1, y, seamZ + depth * 0.5f},
                {x1, y, seamZ - depth * 0.5f},
                {x0, y, seamZ - depth * 0.5f},
                {0, 1, 0}, {1, 0, 0},
                {0, 4}, {1, 4}, {1, 5}, {0, 5}, material);
        } else if (side == 3) {
            float seamX = r;
            float width = std::min(tileW * (0.18f + h0 * 0.08f), 0.38f);
            float span = tileD * (0.92f + h1 * 0.08f);
            float cz = (z0 + z1) * 0.5f + (h2 - 0.5f) * std::max(0.0f, tileD - span) * 0.72f;
            float zz0 = std::max(z0 + tileD * 0.006f, cz - span * 0.5f);
            float zz1 = std::min(z1 - tileD * 0.006f, cz + span * 0.5f);
            AddQuadUV(build.vertices, build.waterIndices,
                {seamX - width * 0.5f, y, zz1},
                {seamX + width * 0.5f, y, zz1},
                {seamX + width * 0.5f, y, zz0},
                {seamX - width * 0.5f, y, zz0},
                {0, 1, 0}, {1, 0, 0},
                {0, 4}, {1, 4}, {1, 5}, {0, 5}, material);
        }
    }
