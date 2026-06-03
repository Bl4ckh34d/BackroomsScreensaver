// Blood damage placement burst and leak coordinators.

    void AddBloodBurstPlacement(WaterSurfaceBuildContext& waterBuild,
                                LiquidCanvasBuildContext& liquidBuild,
                                std::vector<FloorFootprint>& floorReservations,
                                LiquidCeilingFootprintReservations& ceilingReservations,
                                Tile tile,
                                int burstIndex,
                                int scatterSeed,
                                float tileAvg,
                                float tileMin,
                                float floorReservationPad,
                                bool waterLiquid) {
#include "renderer_blood_burst_floor_ceiling.inl"
#include "renderer_blood_burst_wall_splatter.inl"
#include "renderer_blood_burst_droplets.inl"
    }

    bool AddBloodLeakPlacement(WaterSurfaceBuildContext& waterBuild,
                               LiquidCanvasBuildContext& liquidBuild,
                               std::vector<FloorFootprint>& floorReservations,
                               LiquidCeilingFootprintReservations& ceilingReservations,
                               std::vector<PendingLiquidFloorSeam>& pendingFloorSeams,
                               LiquidDamageCoverage& coverage,
                               Tile tile,
                               int side,
                               int leakIndex,
                               bool wallOnly,
                               int scatterSeed,
                               float tileMin,
                               float floorReservationPad,
                               bool waterLiquid) {
#include "renderer_blood_leak_wall_source.inl"
#include "renderer_blood_leak_seam_helpers.inl"
#include "renderer_blood_leak_ceiling_spread.inl"
#include "renderer_blood_leak_floor_pool.inl"
#include "renderer_blood_leak_scare_point.inl"
    }
