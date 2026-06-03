    void AddMazeWallRunsWithExitPortal(std::vector<Vertex>& vertices,
                                       std::vector<uint32_t>& indices,
                                       const MazeSurfaceBuildContext& ctx,
                                       const ExitPortal& exitPortal,
                                       float tileAvg) {
#include "renderer_exit_portal_wall_run_lambdas.inl"
#include "renderer_exit_portal_horizontal_wall_runs.inl"
#include "renderer_exit_portal_vertical_wall_runs.inl"
#include "renderer_exit_portal_wall_features.inl"
    }
