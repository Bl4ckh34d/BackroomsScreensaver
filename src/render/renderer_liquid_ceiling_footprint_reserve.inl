    bool ReserveLiquidCeilingFootprint(LiquidCeilingFootprintReservations& reservations,
                                       float px,
                                       float pz,
                                       float width,
                                       float depth,
                                       float yaw,
                                       float tileMin,
                                       float clearPad = 0.012f,
                                       float reservePad = -1.0f) const {
        if (!LiquidCeilingFootprintClear(reservations, px, pz, width, depth, yaw, tileMin, clearPad)) return false;
        reservations.reservations.push_back(MakeFloorFootprint(px, pz, width, depth, yaw,
            reservePad >= 0.0f ? reservePad : clearPad));
        return true;
    }
