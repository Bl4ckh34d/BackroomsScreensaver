    void AddLiquidCeilingFootprintReservation(LiquidCeilingFootprintReservations& reservations,
                                              float px,
                                              float pz,
                                              float width,
                                              float depth,
                                              float yaw,
                                              float pad) const {
        reservations.reservations.push_back(MakeFloorFootprint(px, pz, width, depth, yaw, pad));
    }
