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
