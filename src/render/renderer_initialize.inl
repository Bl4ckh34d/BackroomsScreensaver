    bool Initialize(HWND hwnd, const Settings* settingsOverride = nullptr, bool monsterPreview = false,
                    MonsterPreviewView monsterPreviewView = MonsterPreviewView::Orbit,
                    const StartupProgressSink* startupProgress = nullptr) {
#include "renderer_initialize_startup_context.inl"
#include "renderer_initialize_device.inl"
#include "renderer_initialize_gpu_resources.inl"
#include "renderer_initialize_scene_assets.inl"
#include "renderer_initialize_maze_generation.inl"
#include "renderer_initialize_scene_finish.inl"
    }
