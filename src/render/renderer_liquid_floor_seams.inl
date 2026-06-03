// Liquid placement seams.

    static void QueueLiquidFloorSeam(std::vector<PendingLiquidFloorSeam>& seams,
                                     Tile owner,
                                     int side,
                                     float cx,
                                     float cz,
                                     float width,
                                     float depth,
                                     float yaw,
                                     float material,
                                     float sourceX,
                                     float sourceZ,
                                     bool water) {
        seams.push_back({owner, side, cx, cz, width, depth, yaw, material, sourceX, sourceZ, water});
    }

    void DrawLiquidFloorSeam(LiquidCanvasBuildContext& build, const PendingLiquidFloorSeam& seam) {
        MarkLiquidCanvasArea(build, seam.cx, seam.cz, seam.width, seam.depth, seam.yaw,
            seam.water, false, 1u << static_cast<uint32_t>(seam.side), false, seam.material,
            std::max(seam.width, seam.depth), false, seam.sourceX, seam.sourceZ);
        MarkWetFootstepArea(seam.cx, seam.cz, seam.width, seam.depth, seam.yaw, 0.02f, seam.water ? 7.5f : 0.0f);
    }

    void EmitMergedLiquidFloorSeams(LiquidCanvasBuildContext& build, std::vector<PendingLiquidFloorSeam>& seams) {
        if (seams.empty()) return;
        for (const PendingLiquidFloorSeam& seam : seams) {
            DrawLiquidFloorSeam(build, seam);
        }
        seams.clear();
    }
