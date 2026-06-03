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
