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
