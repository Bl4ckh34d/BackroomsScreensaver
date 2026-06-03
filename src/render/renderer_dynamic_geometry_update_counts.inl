        dynamicGeometry_.opaqueVertexCount = static_cast<UINT>(opaqueVerts.size());
        dynamicGeometry_.transparentVertexCount = static_cast<UINT>(transparentVerts.size());
        dynamicGeometry_.vertexCount = static_cast<UINT>(opaqueVerts.size() + transparentVerts.size());
