// Dynamic geometry buffer update and upload helpers. 
// Included inside Renderer's private section from renderer_dynamic_geometry.inl.

    void UpdateDynamicGeometry() {
        StartupProfile dynamicProfile(L"UpdateDynamicGeometryBreakdown");
        std::vector<Vertex>& opaqueVerts = dynamicGeometry_.opaqueVerts;
        std::vector<Vertex>& transparentVerts = dynamicGeometry_.transparentVerts;
        opaqueVerts.clear();
        transparentVerts.clear();
        if (opaqueVerts.capacity() < 32768) opaqueVerts.reserve(32768);
        if (transparentVerts.capacity() < 131072) transparentVerts.reserve(131072);
        AppendDynamicDoor(opaqueVerts);
        dynamicProfile.Mark(L"Door");
        if (sessionRuntime_.mode == RendererRuntimeMode::MainMenu) {
            AppendMenuDoorwayLight(transparentVerts);
            AppendMenuButtonPlaques(opaqueVerts, transparentVerts);
            dynamicProfile.Mark(L"MenuGeometry");
        } else {
            AppendVentDrops(opaqueVerts);
            AppendCollectiblePages(opaqueVerts);
            AppendSavePoint(opaqueVerts);
            AppendOmukadeGeometry(opaqueVerts, transparentVerts);
            dynamicProfile.Mark(L"MonsterAndVentDrops");
        }
        AppendAirParticleBillboards(transparentVerts);
        dynamicProfile.Mark(L"AirParticles");
        AppendSparkBillboards(transparentVerts);
        dynamicProfile.Mark(L"Sparks");
        if (sessionRuntime_.mode != RendererRuntimeMode::MainMenu) AppendSteamBillboards(transparentVerts);
        dynamicProfile.Mark(L"Steam");
        if (opaqueVerts.size() > kDynamicVertexCapacity) opaqueVerts.resize(kDynamicVertexCapacity);
        size_t remaining = static_cast<size_t>(kDynamicVertexCapacity) - opaqueVerts.size();
        if (transparentVerts.size() > remaining) transparentVerts.resize(remaining);
        dynamicProfile.Mark(L"Clamp");

        dynamicGeometry_.opaqueVertexCount = static_cast<UINT>(opaqueVerts.size());
        dynamicGeometry_.transparentVertexCount = static_cast<UINT>(transparentVerts.size());
        dynamicGeometry_.vertexCount = static_cast<UINT>(opaqueVerts.size() + transparentVerts.size());
        D3D11_MAPPED_SUBRESOURCE mapped{};
        if (renderBuffers_.dynamicBuffer && dynamicGeometry_.vertexCount > 0 &&
            SUCCEEDED(d3dRuntime_.context->Map(renderBuffers_.dynamicBuffer.Get(), 0, D3D11_MAP_WRITE_DISCARD, 0, &mapped))) {
            auto* dst = static_cast<uint8_t*>(mapped.pData);
            size_t opaqueBytes = opaqueVerts.size() * sizeof(Vertex);
            size_t transparentBytes = transparentVerts.size() * sizeof(Vertex);
            if (opaqueBytes > 0) {
                std::memcpy(dst, opaqueVerts.data(), opaqueBytes);
            }
            if (transparentBytes > 0) {
                std::memcpy(dst + opaqueBytes, transparentVerts.data(), transparentBytes);
            }
            d3dRuntime_.context->Unmap(renderBuffers_.dynamicBuffer.Get(), 0);
        }
        dynamicProfile.Mark(L"Upload");
    }

    void UploadSceneConstants(const SceneConstants& cb) {
        D3D11_MAPPED_SUBRESOURCE mapped{};
        if (SUCCEEDED(d3dRuntime_.context->Map(renderBuffers_.constantBuffer.Get(), 0, D3D11_MAP_WRITE_DISCARD, 0, &mapped))) {
            std::memcpy(mapped.pData, &cb, sizeof(cb));
            d3dRuntime_.context->Unmap(renderBuffers_.constantBuffer.Get(), 0);
        }
    }

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
