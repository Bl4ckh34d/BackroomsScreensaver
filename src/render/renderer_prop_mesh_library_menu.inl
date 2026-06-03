    bool LoadMenuPropMeshes() {
        if (renderAssets_.menuPropMeshesLoaded && !renderAssets_.exitSignPropMesh.vertices.empty()) return true;
        renderAssets_.exitSignPropMesh = {};
        bool exitSignLoaded = LoadStaticPropObj(L"assets\\models\\runtime\\emergency_exit_sign.brmesh", 7.0f, renderAssets_.exitSignPropMesh);
        renderAssets_.menuPropMeshesLoaded = exitSignLoaded;
        StartupProfileLine(exitSignLoaded ? L"Loaded menu prop mesh: emergency exit sign" : L"Menu exit sign mesh missing");
        return exitSignLoaded;
    }
