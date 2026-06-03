    void AppendStaticIndexChunks(const std::vector<Vertex>& vertices,
                                 const std::vector<uint32_t>& sourceIndices,
                                 UINT rangeStart,
                                 UINT rangeCount,
                                 std::vector<uint32_t>& destIndices,
                                 std::vector<StaticIndexChunk>& chunks,
                                 int chunkTiles = 4,
                                 int tilePadding = 1) const {
#include "renderer_static_index_chunk_setup.inl"
#include "renderer_static_index_chunk_triangles.inl"
#include "renderer_static_index_chunk_finalize.inl"
    }
