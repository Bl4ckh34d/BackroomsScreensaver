    bool AddInstancedStaticProp(const StaticPropMesh& mesh,
                                XMFLOAT3 origin,
                                float yaw,
                                float scaleX,
                                float scaleY,
                                float scaleZ,
                                std::vector<Vertex>& instancedVertices,
                                std::vector<uint32_t>& instancedIndices,
                                std::vector<PendingStaticInstance>& pendingStaticInstances,
                                std::vector<InstancedMeshRange>& instancedMeshRanges,
                                float pitch = 0.0f,
                                float materialOverride = -1.0f,
                                bool castsShadow = false,
                                float materialVariant = 0.0f) {
#include "renderer_instanced_static_add_axes.inl"
#include "renderer_instanced_static_add_bounds.inl"
#include "renderer_instanced_static_add_pending.inl"
    }
