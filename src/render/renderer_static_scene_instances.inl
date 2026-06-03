    void BuildStaticInstanceChunks(std::vector<PendingStaticInstance>& pendingStaticInstances,
                                   const std::vector<InstancedMeshRange>& instancedMeshRanges,
                                   const std::vector<uint32_t>& instancedIndices,
                                   std::vector<StaticInstanceData>& instancedInstanceData) {
#include "renderer_static_scene_instance_sort.inl"
#include "renderer_static_scene_instance_chunk_loop.inl"
#include "renderer_static_scene_instance_counts.inl"
    }
