
        StartupProfile dynamicProfile(L"UpdateDynamicGeometryBreakdown");
        std::vector<Vertex>& opaqueVerts = dynamicGeometry_.opaqueVerts;
        std::vector<Vertex>& transparentVerts = dynamicGeometry_.transparentVerts;
        opaqueVerts.clear();
        transparentVerts.clear();
        if (opaqueVerts.capacity() < 32768) opaqueVerts.reserve(32768);
        if (transparentVerts.capacity() < 131072) transparentVerts.reserve(131072);
