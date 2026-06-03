    void AddMazeFloorCeilingRun(std::vector<Vertex>& vertices,
                                std::vector<uint32_t>& indices,
                                const MazeSurfaceBuildContext& ctx,
                                int y,
                                int x0,
                                int x1) {
        for (int x = x0; x < x1; ++x) {
            AddMazeFloorTile(vertices, indices, ctx, x, y);
            AddMazeCeilingTile(vertices, indices, ctx, x, y);
        }
    }
