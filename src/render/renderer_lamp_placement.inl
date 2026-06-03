    void AddMazeCeilingLamps(std::vector<Vertex>& vertices,
                             std::vector<uint32_t>& indices,
                             const MazeSurfaceBuildContext& ctx) {
#include "renderer_lamp_placement_setup.inl"
#include "renderer_lamp_placement_tile_loop.inl"
#include "renderer_lamp_placement_finalize.inl"
    }
