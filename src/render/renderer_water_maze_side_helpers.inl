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
