// Static prop placement debug inspection.

    void AddDebugPropInspectionModel(std::vector<Vertex>& vertices,
                                     std::vector<uint32_t>& indices,
                                     std::vector<uint32_t>& propShadowIndices,
                                     const MazeSurfaceBuildContext& ctx) {
#include "renderer_prop_debug_mesh_select.inl"
#include "renderer_prop_debug_pose.inl"
#include "renderer_prop_debug_mesh_emit.inl"
#include "renderer_prop_debug_air_vent_fallback.inl"
#include "renderer_prop_debug_box_fallback.inl"
    }
