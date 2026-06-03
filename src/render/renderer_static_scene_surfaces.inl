// Static scene finalization, chunking, profiling, and buffer upload.
// Included inside Renderer private section before maze mesh construction.

    struct InstancedMeshRange {
        const StaticPropMesh* mesh = nullptr;
        UINT startIndex = 0;
        UINT indexCount = 0;
        INT baseVertex = 0;
        XMFLOAT3 min{};
        XMFLOAT3 max{};
    };

    struct PendingStaticInstance {
        UINT meshId = 0;
        StaticInstanceData data{};
        XMFLOAT3 min{};
        XMFLOAT3 max{};
        XMFLOAT3 center{};
        float radius = 0.0f;
        int minTileX = 0;
        int minTileY = 0;
        int maxTileX = 0;
        int maxTileY = 0;
        bool castsShadow = false;
    };

    void AddMazeFloorCeilingSurfaces(std::vector<Vertex>& vertices,
                                     std::vector<uint32_t>& indices,
                                     const MazeSurfaceBuildContext& ctx) {
        GameWorldRenderSnapshot world = gameWorld_.BuildRenderSnapshot();
        if (!world.maze) return;
        const Maze& maze = *world.maze;
        staticSceneGeometry_.floorCeilingStartIndex = static_cast<UINT>(indices.size());
        for (int y = 0; y < maze.h; ++y) {
            int x = 0;
            while (x < maze.w) {
                while (x < maze.w && !maze.IsOpen(x, y)) ++x;
                int start = x;
                while (x < maze.w && maze.IsOpen(x, y)) ++x;
                if (start < x) AddMazeFloorCeilingRun(vertices, indices, ctx, y, start, x);
            }
        }
        for (int y = 0; y < maze.h; ++y) {
            for (int x = 0; x < maze.w; ++x) {
                if (maze.IsOpen(x, y)) continue;
                MazeWallFeature feature = maze.WallFeature(x, y);
                if (feature == MazeWallFeature::Tunnel) {
                    AddMazeFloorTile(vertices, indices, ctx, x, y);
                } else if (feature == MazeWallFeature::Window) {
                    AddMazeCeilingTile(vertices, indices, ctx, x, y);
                }
            }
        }
        staticSceneGeometry_.floorCeilingIndexCount =
            static_cast<UINT>(indices.size()) - staticSceneGeometry_.floorCeilingStartIndex;
    }
