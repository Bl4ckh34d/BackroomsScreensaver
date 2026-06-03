// Renderer mesh prop library helpers.

    bool LoadPropMeshes() {
        renderAssets_.propMeshesLoaded = false;
        for (StaticPropMesh& mesh : renderAssets_.chairPropMeshes) mesh = {};
        renderAssets_.cabinetPropMesh = {};
        renderAssets_.deskPropMesh = {};
        renderAssets_.trashBinPropMesh = {};
        renderAssets_.deskLampPropMesh = {};
        renderAssets_.cassettePropMesh = {};
        renderAssets_.airVentPropMesh = {};
        renderAssets_.exitSignPropMesh = {};
        for (StaticPropMesh& mesh : renderAssets_.ceilingLampPropMeshes) mesh = {};

        int loaded = 0;
        const std::array<std::wstring, 3> chairPaths = {
            L"assets\\models\\runtime\\office_chair_modern.brmesh",
            L"assets\\models\\runtime\\office_chair_classic.brmesh",
            L"assets\\models\\runtime\\office_chair_task.brmesh"
        };
        const std::array<float, 3> chairMaterials = {16.0f, 20.0f, 22.0f};
        for (size_t i = 0; i < chairPaths.size(); ++i) {
            if (LoadStaticPropObj(chairPaths[i], chairMaterials[i], renderAssets_.chairPropMeshes[i])) {
                for (Vertex& v : renderAssets_.chairPropMeshes[i].vertices) {
                    int materialId = static_cast<int>(std::floor(v.material));
                    if (materialId == 19 || materialId == 20 || materialId == 21) {
                        v.material = chairMaterials[i];
                    }
                }
                ++loaded;
            }
        }
        bool cabinetLoaded = LoadStaticPropObj(L"assets\\models\\runtime\\filing_cabinet.brmesh", 10.0f, renderAssets_.cabinetPropMesh);
        bool deskLoaded = LoadStaticPropObj(L"assets\\models\\runtime\\office_desk.brmesh", 8.0f, renderAssets_.deskPropMesh);
        bool trashBinLoaded = LoadStaticPropObj(L"assets\\models\\runtime\\trashbin.brmesh", 10.0f, renderAssets_.trashBinPropMesh);
        bool deskLampLoaded = LoadStaticPropObj(L"assets\\models\\runtime\\desklamp.brmesh", 21.0f, renderAssets_.deskLampPropMesh);
        bool cassetteLoaded = LoadStaticPropObj(L"assets\\models\\runtime\\audio_caset.brmesh", 23.0f, renderAssets_.cassettePropMesh);
        bool airVentLoaded = LoadStaticPropObj(L"assets\\models\\runtime\\air_vent.brmesh", 10.0f, renderAssets_.airVentPropMesh);
        bool exitSignLoaded = LoadStaticPropObj(L"assets\\models\\runtime\\emergency_exit_sign.brmesh", 7.0f, renderAssets_.exitSignPropMesh);
        int ceilingLampLoaded = 0;
        for (size_t i = 0; i < renderAssets_.ceilingLampPropMeshes.size(); ++i) {
            wchar_t path[96]{};
            swprintf_s(path, L"assets\\models\\runtime\\ceiling_lamp_%02zu.brmesh", i + 1);
            if (LoadStaticPropObj(path, 21.0f, renderAssets_.ceilingLampPropMeshes[i])) {
                ++ceilingLampLoaded;
            }
        }
        if (cabinetLoaded) ++loaded;
        if (deskLoaded) ++loaded;
        if (trashBinLoaded) ++loaded;
        if (deskLampLoaded) ++loaded;
        if (cassetteLoaded) ++loaded;
        if (airVentLoaded) ++loaded;
        if (exitSignLoaded) ++loaded;
        loaded += ceilingLampLoaded;
        StartupProfileLine(L"Loaded prop meshes: " + std::to_wstring(loaded) + L"/14");
        {
            auto tris = [](const StaticPropMesh& mesh) {
                return static_cast<uint64_t>(mesh.vertices.size() / 3);
            };
            uint64_t chairTris = 0;
            for (const StaticPropMesh& mesh : renderAssets_.chairPropMeshes) chairTris += tris(mesh);
            uint64_t ceilingLampTris = 0;
            for (const StaticPropMesh& mesh : renderAssets_.ceilingLampPropMeshes) ceilingLampTris += tris(mesh);
            std::wstringstream counts;
            counts << L"Prop mesh triangle library: chairs=" << chairTris
                << L", ceilingLamps=" << ceilingLampTris
                << L", cabinet=" << tris(renderAssets_.cabinetPropMesh)
                << L", desk=" << tris(renderAssets_.deskPropMesh)
                << L", trashBin=" << tris(renderAssets_.trashBinPropMesh)
                << L", deskLamp=" << tris(renderAssets_.deskLampPropMesh)
                << L", cassette=" << tris(renderAssets_.cassettePropMesh)
                << L", airVent=" << tris(renderAssets_.airVentPropMesh)
                << L", exitSign=" << tris(renderAssets_.exitSignPropMesh);
            StartupProfileLine(counts.str());
        }
        renderAssets_.propMeshesLoaded = loaded > 0;
        renderAssets_.menuPropMeshesLoaded = renderAssets_.menuPropMeshesLoaded || exitSignLoaded;
        return renderAssets_.propMeshesLoaded;
    }

    bool LoadMenuPropMeshes() {
        if (renderAssets_.menuPropMeshesLoaded && !renderAssets_.exitSignPropMesh.vertices.empty()) return true;
        renderAssets_.exitSignPropMesh = {};
        bool exitSignLoaded = LoadStaticPropObj(L"assets\\models\\runtime\\emergency_exit_sign.brmesh", 7.0f, renderAssets_.exitSignPropMesh);
        renderAssets_.menuPropMeshesLoaded = exitSignLoaded;
        StartupProfileLine(exitSignLoaded ? L"Loaded menu prop mesh: emergency exit sign" : L"Menu exit sign mesh missing");
        return exitSignLoaded;
    }
