    void AddWallVentProp(std::vector<Vertex>& vertices,
                         std::vector<uint32_t>& indices,
                         std::vector<Vertex>& instancedVertices,
                         std::vector<uint32_t>& instancedIndices,
                         std::vector<PendingStaticInstance>& pendingStaticInstances,
                         std::vector<InstancedMeshRange>& instancedMeshRanges,
                         const MazeSurfaceBuildContext& ctx,
                         Tile t,
                         int side,
                         float seed) {
#include "renderer_wall_vent_prop_pose.inl"
#include "renderer_wall_vent_prop_instanced.inl"
#include "renderer_wall_vent_prop_fallback.inl"
#include "renderer_wall_vent_prop_emitters.inl"
    }
