    void AddMazeFloorTile(std::vector<Vertex>& vertices,
                          std::vector<uint32_t>& indices,
                          const MazeSurfaceBuildContext& ctx,
                          int x,
                          int y) {
        float z0 = ctx.oz + y * ctx.tileD;
        float z1 = z0 + ctx.tileD;
        float l = ctx.ox + x * ctx.tileW;
        float r = l + ctx.tileW;
        AddQuadUV(vertices, indices,
            {l, 0, z1}, {r, 0, z1}, {r, 0, z0}, {l, 0, z0},
            {0, 1, 0}, {1, 0, 0},
            FloorUv(l, z1), FloorUv(r, z1), FloorUv(r, z0), FloorUv(l, z0), 1.0f);
    }
