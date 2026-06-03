    bool LiquidCeilingFootprintClear(const LiquidCeilingFootprintReservations& reservations,
                                     float px,
                                     float pz,
                                     float width,
                                     float depth,
                                     float yaw,
                                     float tileMin,
                                     float pad) const {
        if (!FootprintFitsMaze(px, pz, width, depth, yaw, pad, tileMin)) return false;
        FloorFootprint candidate = MakeFloorFootprint(px, pz, width, depth, yaw, pad);
        for (const FloorFootprint& reserved : reservations.reservations) {
            if (FloorFootprintsOverlap(candidate, reserved)) return false;
        }
        return true;
    }
