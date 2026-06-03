    void UploadLampDamageTexture() {
        if (!effectRuntime_.lampDamageDirty || !runtimeTextures_.lampDamageTexture || effectRuntime_.lampDamagePixels.empty()) return;
        GameWorldRenderSnapshot world = gameWorld_.BuildRenderSnapshot();
        if (!world.maze) return;
        d3dRuntime_.context->UpdateSubresource(
            runtimeTextures_.lampDamageTexture.Get(),
            0,
            nullptr,
            effectRuntime_.lampDamagePixels.data(),
            static_cast<UINT>(world.maze->w),
            0);
        effectRuntime_.lampDamageDirty = false;
    }
