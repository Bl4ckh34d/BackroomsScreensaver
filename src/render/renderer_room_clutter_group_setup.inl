        if (!IsRoomLike(t)) return false;
        XMFLOAT3 c = RenderMazeView().WorldCenter(t, 0.0f);
        const float tileW = build.surface.tileW;
        const float tileD = build.surface.tileD;
        float yaw = Rand01(groupIndex, 311, scatterSeed) * kPi * 2.0f;
        float kind = Rand01(groupIndex, 313, scatterSeed);
        float px = c.x + (Rand01(groupIndex, 317, scatterSeed) - 0.5f) * tileW * 0.44f;
        float pz = c.z + (Rand01(groupIndex, 331, scatterSeed) - 0.5f) * tileD * 0.44f;
        auto longFootprintClear = [&](float cx, float cz, float width, float depth, float cyaw, float pad) {
            return LongFloorFootprintClear(build.floorReservations, cx, cz, width, depth, cyaw, build.tileMin, pad);
        };
