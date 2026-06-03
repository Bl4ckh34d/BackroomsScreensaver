    bool AddMetalCabinetProp(std::vector<Vertex>& vertices,
                             std::vector<uint32_t>& transparentIndices,
                             std::vector<FloorFootprint>& floorReservations,
                             std::vector<Vertex>& instancedVertices,
                             std::vector<uint32_t>& instancedIndices,
                             std::vector<PendingStaticInstance>& pendingStaticInstances,
                             std::vector<InstancedMeshRange>& instancedMeshRanges,
                             const MazeSurfaceBuildContext& ctx,
                             float tileAvg,
                             float tileMin,
                             float px,
                             float pz,
                             float yaw,
                             float seed,
                             bool tall) {
        XMFLOAT3 cab = CabinetPropSize(tall);
        float width = cab.x;
        float height = cab.y;
        float depth = cab.z;
        if (renderAssets_.cabinetPropMesh.vertices.empty()) return false;
        if (!ReserveFloorFootprint(floorReservations, px, pz, width, depth, yaw, tileMin, 0.090f)) return false;
        float scaleX = width / PropMeshSpan(renderAssets_.cabinetPropMesh, 0);
        float scaleY = height / PropMeshSpan(renderAssets_.cabinetPropMesh, 1);
        float scaleZ = depth / PropMeshSpan(renderAssets_.cabinetPropMesh, 2);
        AddInstancedStaticProp(renderAssets_.cabinetPropMesh, {px, 0.0f, pz}, yaw, scaleX, scaleY, scaleZ,
            instancedVertices, instancedIndices, pendingStaticInstances, instancedMeshRanges,
            0.0f, 10.0f, true);
        AddBakedPropShadow(vertices, transparentIndices, ctx, tileAvg, px, pz, width, depth, height, yaw, seed);
        cameraRuntime_.propLookPoints.push_back({px, height * 0.74f, pz});
        return true;
    }
