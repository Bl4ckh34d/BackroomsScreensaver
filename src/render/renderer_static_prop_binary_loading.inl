// Renderer mesh static prop helpers.

    static bool LoadStaticPropBinary(const std::filesystem::path& path, StaticPropMesh& out) {
#include "renderer_static_prop_binary_header_read.inl"
#include "renderer_static_prop_binary_raw_vertices.inl"
#include "renderer_static_prop_binary_packed_v2.inl"
#include "renderer_static_prop_binary_packed_v3.inl"
#include "renderer_static_prop_binary_finalize.inl"
    }
