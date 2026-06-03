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
