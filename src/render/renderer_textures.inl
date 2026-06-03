// Renderer material atlas generation, external PBR loading, and texture-cache upload.
// Included inside Renderer private section before flashlight-pattern texture creation.

    #include "renderer_loose_page_textures.inl"
    #include "renderer_runtime_texture_upload.inl"
    #include "renderer_high_res_pbr_textures.inl"

    bool CreateTextures() {
#include "renderer_texture_atlas_setup_cache.inl"
#include "renderer_texture_atlas_load_helpers.inl"
#include "renderer_texture_procedural_materials.inl"
#include "renderer_texture_runtime_materials.inl"
#include "renderer_texture_external_assets.inl"
#include "renderer_texture_normal_build.inl"
#include "renderer_texture_gpu_upload.inl"
    }
