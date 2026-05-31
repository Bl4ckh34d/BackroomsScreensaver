#include "../debug/effect_debug.inl"

#include "render_types.inl"

#include "../core/math_utils.inl"

#include "../config/settings.inl"

#include "../audio/audio_engine.inl"

#include "../maze/maze.inl"

#include "../gameplay/playable_progression_types.inl"

#include "renderer_public_types.inl"

class Renderer {
public:
    #include "renderer_lifecycle.inl"

private:
    static constexpr int kStartupProgressPreShaderSteps = 3;
    static constexpr int kStartupProgressPostShaderSteps = 10;
    static constexpr int kStartupProgressUnitsPerStep = 4;

    struct StaticPropMesh {
        std::vector<Vertex> vertices;
        XMFLOAT3 min{};
        XMFLOAT3 max{};
        bool generatedUvFallback = false;
    };

    #include "../gameplay/playable_snapshot.inl"

    #include "renderer_scene_assets.inl"

    #include "renderer_startup_progress.inl"

    #include "../debug/debug_slice_runtime.inl"

    #include "renderer_state.inl"

    #include "../gameplay/playable_progression_runtime.inl"

    #include "../audio/renderer_audio.inl"

    #include "renderer_menu_scene.inl"

    #include "renderer_backbuffer.inl"

    #include "renderer_shader_cache.inl"

    #include "renderer_texture_cache.inl"

    #include "renderer_mesh_loading.inl"

    #include "renderer_shaders.inl"

    #include "renderer_device_resources.inl"

    #include "renderer_textures.inl"

    #include "renderer_runtime_textures.inl"

    #include "renderer_maze_mesh.inl"

    #include "../monster/monster_ai.inl"

    #include "../gameplay/scare_effect_events.inl"

    #include "../gameplay/renderer_update.inl"

    #include "renderer_dynamic_geometry.inl"

    #include "renderer_overlays.inl"

    #include "renderer_present.inl"
};
