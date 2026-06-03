// Liquid, water, and decal placement helpers.
// Included inside Renderer private section before maze mesh construction.

    struct WaterTileSurface {
        bool active = false;
        bool suppressCard = false;
        int side = 0;
        int mode = 0;
        float seed = 0.0f;
        float score = -1.0f;
    };

    struct PendingWallWaterPool {
        Tile owner{};
        int side = 0;
        float cx = 0.0f;
        float cz = 0.0f;
        float width = 0.0f;
        float depth = 0.0f;
        float yaw = 0.0f;
        float seed = 0.0f;
        float score = 0.0f;
    };

    struct LiquidCanvasSurface {
        bool active = false;
        uint32_t sourceMask = 0;
        bool centerSource = false;
        bool downstream = false;
        float seed = 0.0f;
        float score = -1.0e9f;
    };

    struct WallLiquidCanvasSurface {
        bool active = false;
        float minAlong = 0.0f;
        float maxAlong = 0.0f;
        float seed = 0.0f;
        float score = -1.0e9f;
    };

    struct PendingLiquidFloorSeam {
        Tile owner{};
        int side = 0;
        float cx = 0.0f;
        float cz = 0.0f;
        float width = 0.0f;
        float depth = 0.0f;
        float yaw = 0.0f;
        float material = 25.0f;
        float sourceX = 0.0f;
        float sourceZ = 0.0f;
        bool water = false;
    };

    struct LiquidWallLeakFrame {
        XMFLOAT3 normal{0.0f, 0.0f, 1.0f};
        XMFLOAT3 right{1.0f, 0.0f, 0.0f};
        XMFLOAT3 inward{0.0f, 0.0f, 1.0f};
        XMFLOAT3 center{0.0f, 0.0f, 0.0f};
        XMFLOAT3 a{0.0f, 0.0f, 0.0f};
        XMFLOAT3 b{0.0f, 0.0f, 0.0f};
        XMFLOAT3 c{0.0f, 0.0f, 0.0f};
        XMFLOAT3 d{0.0f, 0.0f, 0.0f};
    };

    struct LiquidWallProjectionFit {
        XMFLOAT3 center{0.0f, 0.0f, 0.0f};
        XMFLOAT3 source{0.0f, 0.0f, 0.0f};
        float width = 0.0f;
        float depth = 0.0f;
    };

    struct LiquidTileTouchInfo {
        float halfX = 0.0f;
        float halfZ = 0.0f;
        bool touches[4]{};
    };

    struct LiquidDamageCoverage {
        std::unordered_set<int> blockedTiles;
        std::unordered_set<int> centerCoveredTiles;
        std::unordered_map<int, int> ceilingLayers;
    };

    struct LiquidCeilingFootprintReservations {
        std::vector<FloorFootprint> reservations;
    };

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

    #include "renderer_liquid_damage_coverage.inl"
    #include "renderer_liquid_wall_support.inl"
    #include "renderer_liquid_canvas_access.inl"
    #include "renderer_liquid_canvas_tiles.inl"
    #include "renderer_liquid_wall_canvas.inl"
    #include "renderer_liquid_floor_seams.inl"
    #include "renderer_liquid_wall_helpers.inl"
    #include "renderer_liquid_fit_helpers.inl"
    #include "renderer_water_shared_helpers.inl"
