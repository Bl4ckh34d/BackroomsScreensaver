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
