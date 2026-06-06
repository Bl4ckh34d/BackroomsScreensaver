    bool LoadMenuPropMeshes() {
        if (renderAssets_.menuPropMeshesLoaded &&
            !renderAssets_.exitSignPropMesh.vertices.empty() &&
            !renderAssets_.toolBoxPropMesh.vertices.empty()) return true;
        renderAssets_.exitSignPropMesh = {};
        renderAssets_.toolBoxPropMesh = {};
        bool exitSignLoaded = LoadStaticPropObj(L"assets\\models\\runtime\\emergency_exit_sign.brmesh", 7.0f, renderAssets_.exitSignPropMesh);
        bool toolBoxLoaded = LoadStaticPropObj(L"assets\\models\\runtime\\tool_box.obj", 17.0f, renderAssets_.toolBoxPropMesh);
        renderAssets_.menuPropMeshesLoaded = exitSignLoaded && toolBoxLoaded;
        StartupProfileLine(exitSignLoaded ? L"Loaded menu prop mesh: emergency exit sign" : L"Menu exit sign mesh missing");
        StartupProfileLine(toolBoxLoaded ? L"Loaded menu prop mesh: tool box" : L"Menu tool box mesh missing");
        return exitSignLoaded && toolBoxLoaded;
    }
