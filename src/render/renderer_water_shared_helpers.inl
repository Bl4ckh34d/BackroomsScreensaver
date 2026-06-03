// Liquid placement shared water helpers.

    static float WaterDecalMaterial(float seed, float bandStart, float bandWidth) {
        float h = std::fmod(std::abs(seed) * 37.719f + 0.137f, 1.0f);
        float safeWidth = std::max(0.0f, std::min(bandWidth, 0.043f - bandStart));
        return 11.006f + bandStart + h * safeWidth;
    }

    static int MergeWaterMode(int a, int b) {
        if (a == 3 && b == 3) return 3;
        if (a == 3) a = 0;
        if (b == 3) b = 0;
        bool central = a == 0 || a == 1 || b == 0 || b == 1;
        bool edge = a == 1 || a == 2 || b == 1 || b == 2;
        if (central && edge) return 1;
        if (edge) return 2;
        return 0;
    }

    static int OppositeMazeSide(int side) {
        if (side == 0) return 1;
        if (side == 1) return 0;
        if (side == 2) return 3;
        return 2;
    }

    static Tile NeighborForMazeSide(Tile t, int side) {
        if (side == 0) return Tile{t.x, t.y - 1};
        if (side == 1) return Tile{t.x, t.y + 1};
        if (side == 2) return Tile{t.x - 1, t.y};
        return Tile{t.x + 1, t.y};
    }

    static XMFLOAT3 DirectionForMazeSide(int side) {
        if (side == 0) return XMFLOAT3{0.0f, 0.0f, -1.0f};
        if (side == 1) return XMFLOAT3{0.0f, 0.0f, 1.0f};
        if (side == 2) return XMFLOAT3{-1.0f, 0.0f, 0.0f};
        return XMFLOAT3{1.0f, 0.0f, 0.0f};
    }

    static float ForwardYawForMazeSide(int side) {
        if (side == 0) return kPi;
        if (side == 1) return 0.0f;
        if (side == 2) return -kPi * 0.5f;
        return kPi * 0.5f;
    }

    static float LiquidCardYawForSide(int side) {
        if (side == 0) return 0.0f;
        if (side == 1) return kPi;
        if (side == 2) return kPi * 0.5f;
        return -kPi * 0.5f;
    }

    static uint32_t LiquidSourceSideToward(Tile tile, Tile sourceTile) {
        int dx = sourceTile.x - tile.x;
        int dy = sourceTile.y - tile.y;
        if (dx == 0 && dy == 0) return 0u;
        if (std::abs(dx) >= std::abs(dy) && dx != 0) {
            return 1u << static_cast<uint32_t>(dx < 0 ? 2 : 3);
        }
        return 1u << static_cast<uint32_t>(dy < 0 ? 0 : 1);
    }

    static float LiquidCanvasMaterial(bool water, float seed) {
        float h = std::fmod(std::abs(seed) * 37.719f + 0.137f, 1.0f);
        return (water ? 25.0f : 14.0f) + 0.990f + h * 0.0085f;
    }

    static float LiquidSurfaceMaterial(bool water, float rawSeed) {
        return (water ? 25.0f : 14.0f) + rawSeed;
    }

    static float WaterLikeSurfaceMaterial(float seed, float rawSeed) {
        return 25.0f + rawSeed + std::fmod(std::abs(seed) * 0.0017f, 0.0009f);
    }

    static float WaterWallCanvasMaterial(float seed) {
        return 25.965f + std::fmod(std::abs(seed), 1.0f) * 0.0245f;
    }
