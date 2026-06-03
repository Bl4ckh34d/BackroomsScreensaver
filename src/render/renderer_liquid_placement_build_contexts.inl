    struct WaterSurfaceBuildContext {
        std::vector<Vertex>& vertices;
        std::vector<uint32_t>& waterIndices;
        std::vector<WaterTileSurface>& floorWaterTiles;
        std::vector<WaterTileSurface>& ceilingWaterTiles;
        std::vector<PendingWallWaterPool>& pendingWallWaterPools;
        const MazeSurfaceBuildContext& surface;
        float tileMin = 0.0f;
        float floorLift = 0.0f;
        float ceilingY = 0.0f;
        float wallH = 0.0f;
    };

    struct LiquidCanvasBuildContext {
        std::vector<Vertex>& vertices;
        std::vector<uint32_t>& liquidIndices;
        std::vector<LiquidCanvasSurface>& floorBloodCanvas;
        std::vector<LiquidCanvasSurface>& ceilingBloodCanvas;
        std::vector<LiquidCanvasSurface>& floorWaterCanvas;
        std::vector<LiquidCanvasSurface>& ceilingWaterCanvas;
        std::vector<WallLiquidCanvasSurface>& wallWaterCanvas;
        const MazeSurfaceBuildContext& surface;
        float floorY = 0.0f;
        float waterFloorY = 0.0f;
        float ceilingY = 0.0f;
        float wallH = 0.0f;
    };
