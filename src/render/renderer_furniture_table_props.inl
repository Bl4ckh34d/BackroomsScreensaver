    bool AddStandingTableProp(StaticPropPlacementBuildContext& build,
                              float px,
                              float pz,
                              float width,
                              float depth,
                              float yaw,
                              float seed) {
        if (renderAssets_.deskPropMesh.vertices.empty()) return false;
        if (!ReserveFloorFootprint(build.floorReservations, px, pz, width + 0.22f, depth + 0.22f,
                yaw, build.tileMin, 0.085f)) {
            return false;
        }
        float topY = 0.70f + seed * 0.05f;
        float scaleX = depth / PropMeshSpan(renderAssets_.deskPropMesh, 0);
        float scaleY = topY + 0.035f;
        float scaleZ = width / PropMeshSpan(renderAssets_.deskPropMesh, 2);
        AddInstancedStaticProp(renderAssets_.deskPropMesh, {px, 0.0f, pz}, yaw + kPi * 0.5f, scaleX, scaleY, scaleZ,
            build.instancedVertices, build.instancedIndices, build.pendingStaticInstances, build.instancedMeshRanges,
            0.0f, -1.0f, true);
        AddBakedPropShadow(build.vertices, build.transparentIndices, build.surface, build.tileAvg,
            px, pz, width, depth, topY + 0.08f, yaw, seed);
        cameraRuntime_.propLookPoints.push_back({px, topY, pz});
        return true;
    }

    bool AddSideTableProp(StaticPropPlacementBuildContext& build,
                          float px,
                          float pz,
                          float width,
                          float depth,
                          float yaw,
                          float seed) {
        if (renderAssets_.deskPropMesh.vertices.empty()) return false;
        if (!ReserveFloorFootprint(build.floorReservations, px, pz, width + 0.24f, depth * 0.72f + 0.36f,
                yaw, build.tileMin, 0.085f)) {
            return false;
        }
        float height = 0.68f + seed * 0.10f;
        float scaleX = depth / PropMeshSpan(renderAssets_.deskPropMesh, 0);
        float scaleY = height + 0.030f;
        float scaleZ = width / PropMeshSpan(renderAssets_.deskPropMesh, 2);
        AddInstancedStaticProp(renderAssets_.deskPropMesh, {px, 0.0f, pz}, yaw + kPi * 0.5f, scaleX, scaleY, scaleZ,
            build.instancedVertices, build.instancedIndices, build.pendingStaticInstances, build.instancedMeshRanges,
            0.0f, -1.0f, true);
        AddBakedPropShadow(build.vertices, build.transparentIndices, build.surface, build.tileAvg,
            px, pz, width, depth, height + 0.06f, yaw, seed);
        cameraRuntime_.propLookPoints.push_back({px, height * 0.72f, pz});
        return true;
    }
