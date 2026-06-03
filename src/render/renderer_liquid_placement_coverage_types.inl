    struct LiquidDamageCoverage {
        std::unordered_set<int> blockedTiles;
        std::unordered_set<int> centerCoveredTiles;
        std::unordered_map<int, int> ceilingLayers;
    };

    struct LiquidCeilingFootprintReservations {
        std::vector<FloorFootprint> reservations;
    };
