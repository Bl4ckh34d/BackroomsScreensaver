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

    bool AddMetalCabinetAgainstWallProp(std::vector<Vertex>& vertices,
                                        std::vector<uint32_t>& transparentIndices,
                                        std::vector<FloorFootprint>& floorReservations,
                                        std::vector<Vertex>& instancedVertices,
                                        std::vector<uint32_t>& instancedIndices,
                                        std::vector<PendingStaticInstance>& pendingStaticInstances,
                                        std::vector<InstancedMeshRange>& instancedMeshRanges,
                                        const MazeSurfaceBuildContext& ctx,
                                        float tileAvg,
                                        float tileMin,
                                        Tile t,
                                        int side,
                                        float seed) {
        XMFLOAT3 c = RenderMazeView().WorldCenter(t, 0.0f);
        bool tall = seed < 0.62f;
        XMFLOAT3 cab = CabinetPropSize(tall);
        float width = cab.x;
        float depth = cab.z;
        float wallSpan = (side == 0 || side == 1) ? ctx.tileW : ctx.tileD;
        float lateralLimit = std::max(0.0f, wallSpan * 0.5f - width * 0.5f - 0.10f);
        float lateral = (LampHash(c.x + seed * 9.1f, c.z - seed * 3.7f) - 0.5f) * lateralLimit * 2.0f;
        float px = c.x;
        float pz = c.z;
        float yaw = 0.0f;
        if (side == 0) {
            yaw = kPi;
            px = c.x + lateral;
            pz = c.z - ctx.tileD * 0.5f + depth * 0.5f + 0.075f;
        } else if (side == 1) {
            yaw = 0.0f;
            px = c.x + lateral;
            pz = c.z + ctx.tileD * 0.5f - depth * 0.5f - 0.075f;
        } else if (side == 2) {
            yaw = -kPi * 0.5f;
            px = c.x - ctx.tileW * 0.5f + depth * 0.5f + 0.075f;
            pz = c.z + lateral;
        } else {
            yaw = kPi * 0.5f;
            px = c.x + ctx.tileW * 0.5f - depth * 0.5f - 0.075f;
            pz = c.z + lateral;
        }
        return AddMetalCabinetProp(vertices, transparentIndices, floorReservations,
            instancedVertices, instancedIndices, pendingStaticInstances, instancedMeshRanges,
            ctx, tileAvg, tileMin, px, pz, yaw, seed, tall);
    }

    bool AddMetalCabinetAgainstWallProp(StaticPropPlacementBuildContext& build, Tile t, int side, float seed) {
        return AddMetalCabinetAgainstWallProp(build.vertices, build.transparentIndices, build.floorReservations,
            build.instancedVertices, build.instancedIndices, build.pendingStaticInstances, build.instancedMeshRanges,
            build.surface, build.tileAvg, build.tileMin, t, side, seed);
    }
