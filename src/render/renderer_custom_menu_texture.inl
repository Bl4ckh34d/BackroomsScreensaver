    void UpdateCustomMenuTexture() {
        if (!menuRuntime_.customTextureDirty && runtimeTextures_.customMenuSrv) return;
        menuRuntime_.customTextureDirty = false;

#include "renderer_custom_menu_texture_gdi_setup.inl"
#include "renderer_custom_menu_texture_pages.inl"
#include "renderer_custom_menu_texture_upload.inl"
    }
