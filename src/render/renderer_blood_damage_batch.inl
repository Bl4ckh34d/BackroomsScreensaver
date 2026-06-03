    void EmitBloodDamagePlacementBatch(WaterSurfaceBuildContext& waterBuild,
                                       LiquidCanvasBuildContext& liquidBuild,
                                       std::vector<FloorFootprint>& floorReservations,
                                       LiquidCeilingFootprintReservations& ceilingReservations,
                                       std::vector<PendingLiquidFloorSeam>& pendingFloorSeams,
                                       LiquidDamageCoverage& coverage,
                                       const std::vector<Tile>& openTiles,
                                       int scatterSeed,
                                       float tileAvg,
                                       float tileW,
                                       float tileD,
                                       float tileMin,
                                       float floorReservationPad,
                                       bool waterLiquid) {
#include "renderer_blood_damage_study_view.inl"
#include "renderer_blood_damage_world_geometry.inl"
#include "renderer_blood_damage_debug_every_wall.inl"
    }
