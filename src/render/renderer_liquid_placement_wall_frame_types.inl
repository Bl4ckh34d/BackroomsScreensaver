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
