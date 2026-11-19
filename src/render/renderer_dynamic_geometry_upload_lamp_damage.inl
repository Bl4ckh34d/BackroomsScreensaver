    void UploadLampDamageTexture() {
        if (!effectRuntime_.lampDamageDirty || !runtimeTextures_.lampDamageTexture || effectRuntime_.lampDamagePixels.empty()) return;
        GameWorldRenderSnapshot world = gameWorld_.BuildRenderSnapshot();
        if (!world.maze) return;
        const auto texels = BuildLampDamageTexels(*world.maze);
        d3dRuntime_.context->UpdateSubresource(
            runtimeTextures_.lampDamageTexture.Get(),
            0,
            nullptr,
            texels.data(),
            static_cast<UINT>(world.maze->w * sizeof(LampDamageTexel)),
            0);
        effectRuntime_.lampDamageDirty = false;
    }
