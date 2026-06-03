    bool AddWaterLikeLeakPlacement(LiquidCanvasBuildContext& build,
                                   std::vector<FloorFootprint>& floorReservations,
                                   LiquidCeilingFootprintReservations& ceilingReservations,
                                   LiquidDamageCoverage& coverage,
                                   Tile tile,
                                   int side,
                                   int leakIndex,
                                   bool wallOnly,
                                   int scatterSeed,
                                   float tileW,
                                   float tileD,
                                   float tileMin,
                                   float floorReservationPad,
                                   float wallH) {
#include "renderer_water_like_leak_wall_card.inl"
#include "renderer_water_like_leak_floor_ceiling.inl"
#include "renderer_water_like_leak_scare_point.inl"
#include "renderer_water_like_leak_return.inl"
    }
