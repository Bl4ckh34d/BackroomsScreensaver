// Renderer frame presentation coordinator.
// Included inside Renderer private section after overlay helpers.

    void Render() {
        #include "renderer_present_begin_camera.inl"
        #include "renderer_present_scene_constants.inl"
        #include "renderer_present_dynamic_update.inl"
        #include "renderer_present_visibility.inl"
        #include "renderer_present_shadow_pass.inl"
        #include "renderer_present_scene_pass.inl"
        #include "renderer_present_finish.inl"
    }
