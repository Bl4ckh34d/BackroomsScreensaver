    bool AddDeskLampOnSurfaceProp(StaticPropPlacementBuildContext& build,
                                  float px,
                                  float pz,
                                  float surfaceY,
                                  float tableYaw,
                                  float tableWidth,
                                  float tableDepth,
                                  float seed) {
        if (renderAssets_.deskLampPropMesh.vertices.empty()) return false;
        float c = std::cos(tableYaw);
        float s = std::sin(tableYaw);
        XMFLOAT3 right{c, 0.0f, -s};
        XMFLOAT3 forward{s, 0.0f, c};
        float ox = (seed - 0.5f) * tableWidth * 0.34f;
        float oz = (LampHash(px + seed * 7.1f, pz - seed * 5.3f) - 0.5f) * tableDepth * 0.30f;
        XMFLOAT3 p = Add3({px, 0.0f, pz}, Add3(Scale3(right, ox), Scale3(forward, oz)));
        constexpr float targetHeight = 0.42f;
        float scale = targetHeight / PropMeshSpan(renderAssets_.deskLampPropMesh, 1);
        AddInstancedStaticPropGrounded(renderAssets_.deskLampPropMesh, {p.x, surfaceY + 0.012f, p.z},
            tableYaw + (seed - 0.5f) * 0.85f, scale, scale, scale,
            build.instancedVertices, build.instancedIndices, build.pendingStaticInstances, build.instancedMeshRanges,
            0.0f, -1.0f, true);
        cameraRuntime_.propLookPoints.push_back({p.x, surfaceY + targetHeight * 0.62f, p.z});
        return true;
    }
