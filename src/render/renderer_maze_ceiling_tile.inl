    void AddMazeCeilingTile(std::vector<Vertex>& vertices,
                            std::vector<uint32_t>& indices,
                            const MazeSurfaceBuildContext& ctx,
                            int x,
                            int y) {
        float z0 = ctx.oz + y * ctx.tileD;
        float z1 = z0 + ctx.tileD;
        float l = ctx.ox + x * ctx.tileW;
        float r = l + ctx.tileW;
        AddQuadUV(vertices, indices,
            {l, ctx.wallH, z0}, {r, ctx.wallH, z0}, {r, ctx.wallH, z1}, {l, ctx.wallH, z1},
            {0, -1, 0}, {1, 0, 0},
            CeilingUv(l, z0), CeilingUv(r, z0), CeilingUv(r, z1), CeilingUv(l, z1), 2.0f);
    }
