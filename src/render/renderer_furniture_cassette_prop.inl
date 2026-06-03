    bool AddCassetteProp(StaticPropPlacementBuildContext& build,
                         float px,
                         float pz,
                         float yaw,
                         float floorY,
                         float seed) {
        if (renderAssets_.cassettePropMesh.vertices.empty()) return false;
        float width = 0.100f;
        float depth = 0.064f;
        if (!FloorFootprintClear(build.floorReservations, px, pz, width, depth, yaw, build.tileMin, 0.024f)) return false;
        float scale = width / PropMeshSpan(renderAssets_.cassettePropMesh, 0);
        AddInstancedStaticPropGrounded(renderAssets_.cassettePropMesh, {px, floorY + 0.002f, pz},
            yaw + (seed - 0.5f) * 0.38f, scale, scale, scale,
            build.instancedVertices, build.instancedIndices, build.pendingStaticInstances, build.instancedMeshRanges,
            0.0f, -1.0f, true);
        return true;
    }
