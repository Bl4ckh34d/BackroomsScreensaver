// Maze placement footprint helpers.
// Included inside Renderer private section before maze mesh construction.

    struct FloorFootprint {
        float x = 0.0f;
        float z = 0.0f;
        float hx = 0.0f;
        float hz = 0.0f;
        float c = 1.0f;
        float s = 0.0f;
    };

    struct CandidatePlacement {
        float x = 0.0f;
        float z = 0.0f;
        float yaw = 0.0f;
        float score = -1.0e9f;
    };

    static FloorFootprint MakeFloorFootprint(float px, float pz, float width, float depth, float yaw, float pad) {
        FloorFootprint fp{};
        fp.x = px;
        fp.z = pz;
        fp.hx = width * 0.5f + pad;
        fp.hz = depth * 0.5f + pad;
        fp.c = std::cos(yaw);
        fp.s = std::sin(yaw);
        return fp;
    }

    static float FootprintAxisDot(float ax, float az, float bx, float bz) {
        return ax * bx + az * bz;
    }

    static bool FloorFootprintsOverlap(const FloorFootprint& a, const FloorFootprint& b) {
        auto separatedOn = [&](float ax, float az) {
            float dx = b.x - a.x;
            float dz = b.z - a.z;
            float center = std::abs(FootprintAxisDot(dx, dz, ax, az));
            float ar = a.hx * std::abs(FootprintAxisDot(a.c, -a.s, ax, az)) +
                a.hz * std::abs(FootprintAxisDot(a.s, a.c, ax, az));
            float br = b.hx * std::abs(FootprintAxisDot(b.c, -b.s, ax, az)) +
                b.hz * std::abs(FootprintAxisDot(b.s, b.c, ax, az));
            return center > ar + br;
        };
        if (separatedOn(a.c, -a.s)) return false;
        if (separatedOn(a.s, a.c)) return false;
        if (separatedOn(b.c, -b.s)) return false;
        if (separatedOn(b.s, b.c)) return false;
        return true;
    }

    bool FootprintFitsMaze(float px,
                           float pz,
                           float width,
                           float depth,
                           float yaw,
                           float wallPad,
                           float tileMin) const {
        GameWorldRenderSnapshot world = gameWorld_.BuildRenderSnapshot();
        if (!world.maze) return false;
        const Maze& maze = *world.maze;
        float c = std::cos(yaw);
        float s = std::sin(yaw);
        XMFLOAT3 right{c, 0.0f, -s};
        XMFLOAT3 forward{s, 0.0f, c};
        float hx = width * 0.5f + wallPad;
        float hz = depth * 0.5f + wallPad;
        int sxCount = std::clamp(static_cast<int>(std::ceil((width + wallPad * 2.0f) / (tileMin * 0.18f))) + 2, 3, 13);
        int szCount = std::clamp(static_cast<int>(std::ceil((depth + wallPad * 2.0f) / (tileMin * 0.18f))) + 2, 3, 13);
        for (int sy = 0; sy < szCount; ++sy) {
            float fy = szCount == 1 ? 0.0f : static_cast<float>(sy) / static_cast<float>(szCount - 1);
            float ly = Lerp(-hz, hz, fy);
            for (int sx = 0; sx < sxCount; ++sx) {
                float fx = sxCount == 1 ? 0.0f : static_cast<float>(sx) / static_cast<float>(sxCount - 1);
                float lx = Lerp(-hx, hx, fx);
                XMFLOAT3 p = Add3({px, 0.0f, pz}, OrientedOffset(right, {0, 1, 0}, forward, lx, 0.0f, ly));
                Tile tile = maze.TileFromWorld(p.x, p.z);
                if (!maze.IsOpen(tile.x, tile.y)) return false;
            }
        }
        return true;
    }

    bool FloorFootprintClear(const std::vector<FloorFootprint>& reservations,
                             float px,
                             float pz,
                             float width,
                             float depth,
                             float yaw,
                             float tileMin,
                             float pad = 0.055f) const {
        if (!FootprintFitsMaze(px, pz, width, depth, yaw, pad, tileMin)) return false;
        FloorFootprint candidate = MakeFloorFootprint(px, pz, width, depth, yaw, pad);
        for (const FloorFootprint& reserved : reservations) {
            if (FloorFootprintsOverlap(candidate, reserved)) return false;
        }
        return true;
    }

    bool ReserveFloorFootprint(std::vector<FloorFootprint>& reservations,
                               float px,
                               float pz,
                               float width,
                               float depth,
                               float yaw,
                               float tileMin,
                               float pad = 0.075f) const {
        if (!FloorFootprintClear(reservations, px, pz, width, depth, yaw, tileMin, pad)) return false;
        reservations.push_back(MakeFloorFootprint(px, pz, width, depth, yaw, pad));
        return true;
    }

    bool LongFloorFootprintClear(const std::vector<FloorFootprint>& reservations,
                                 float px,
                                 float pz,
                                 float width,
                                 float depth,
                                 float yaw,
                                 float tileMin,
                                 float pad = 0.055f) const {
        if (!FloorFootprintClear(reservations, px, pz, width, depth, yaw, tileMin, pad)) return false;
        GameWorldRenderSnapshot world = gameWorld_.BuildRenderSnapshot();
        if (!world.maze) return false;
        const Maze& maze = *world.maze;
        float c = std::cos(yaw);
        float s = std::sin(yaw);
        XMFLOAT3 right{c, 0.0f, -s};
        XMFLOAT3 forward{s, 0.0f, c};
        int steps = std::clamp(static_cast<int>(width / (tileMin * 0.30f)) + 2, 4, 22);
        for (int i = 0; i <= steps; ++i) {
            float along = (static_cast<float>(i) / static_cast<float>(steps) - 0.5f) * width;
            const float laterals[] = {-0.46f, 0.0f, 0.46f};
            for (float lateral : laterals) {
                XMFLOAT3 p = Add3({px, 0.0f, pz}, OrientedOffset(right, {0, 1, 0}, forward, along, 0.0f, lateral * depth));
                Tile tile = maze.TileFromWorld(p.x, p.z);
                if (!maze.IsOpen(tile.x, tile.y)) return false;
            }
        }
        return true;
    }
