#pragma once

struct StaticPropMesh {
    std::vector<Vertex> vertices;
    XMFLOAT3 min{};
    XMFLOAT3 max{};
    bool generatedUvFallback = false;
};

struct RenderAssetRuntimeState {
    std::vector<Vertex> skullMesh;
    bool monsterUsingAltSkull = false;
    bool monsterSkullNativeMaskAxes = false;
    std::array<StaticPropMesh, 3> chairPropMeshes;
    StaticPropMesh cabinetPropMesh;
    StaticPropMesh deskPropMesh;
    StaticPropMesh trashBinPropMesh;
    StaticPropMesh deskLampPropMesh;
    StaticPropMesh cassettePropMesh;
    StaticPropMesh airVentPropMesh;
    StaticPropMesh exitSignPropMesh;
    std::array<StaticPropMesh, 4> ceilingLampPropMeshes;
    bool monsterMeshLoaded = false;
    bool propMeshesLoaded = false;
    bool menuPropMeshesLoaded = false;
};
