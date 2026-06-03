#pragma once

struct DynamicGeometryRuntimeState {
    std::vector<Vertex> opaqueVerts;
    std::vector<Vertex> transparentVerts;
    UINT opaqueVertexCount = 0;
    UINT transparentVertexCount = 0;
    UINT vertexCount = 0;
};
