    static uint32_t LiquidSourceSideToward(Tile tile, Tile sourceTile) {
        int dx = sourceTile.x - tile.x;
        int dy = sourceTile.y - tile.y;
        if (dx == 0 && dy == 0) return 0u;
        if (std::abs(dx) >= std::abs(dy) && dx != 0) {
            return 1u << static_cast<uint32_t>(dx < 0 ? 2 : 3);
        }
        return 1u << static_cast<uint32_t>(dy < 0 ? 0 : 1);
    }
