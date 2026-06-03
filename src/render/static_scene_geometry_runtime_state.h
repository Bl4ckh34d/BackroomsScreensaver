#pragma once

struct StaticSceneGeometryRuntimeState {
    UINT indexCount = 0;
    UINT floorCeilingStartIndex = 0;
    UINT floorCeilingIndexCount = 0;
    UINT waterStartIndex = 0;
    UINT waterIndexCount = 0;
    UINT transparentStartIndex = 0;
    UINT transparentIndexCount = 0;
    UINT propShadowStartIndex = 0;
    UINT propShadowIndexCount = 0;
    UINT instancedIndexCount = 0;
    UINT instancedInstanceCount = 0;
    std::vector<StaticIndexChunk> opaqueChunks;
    std::vector<StaticIndexChunk> floorCeilingChunks;
    std::vector<StaticIndexChunk> waterChunks;
    std::vector<StaticIndexChunk> transparentChunks;
    std::vector<StaticIndexChunk> propShadowChunks;
    std::vector<StaticInstanceChunk> instancedOpaqueChunks;
    std::vector<StaticInstanceChunk> instancedPropShadowChunks;
};
