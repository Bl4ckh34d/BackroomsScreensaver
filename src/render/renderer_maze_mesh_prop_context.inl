        std::vector<FloorFootprint> floorReservations;
        floorReservations.reserve(512);

        std::vector<Vertex> instancedVertices;
        std::vector<uint32_t> instancedIndices;
        std::vector<StaticInstanceData> instancedInstanceData;
        std::vector<PendingStaticInstance> pendingStaticInstances;
        std::vector<InstancedMeshRange> instancedMeshRanges;
        instancedVertices.reserve(4096);
        instancedIndices.reserve(4096);
        pendingStaticInstances.reserve(maze.w * maze.h / 2);

        StaticPropPlacementBuildContext propBuild{
            vertices,
            indices,
            transparentIndices,
            floorReservations,
            instancedVertices,
            instancedIndices,
            pendingStaticInstances,
            instancedMeshRanges,
            surfaceBuild,
            tileAvg,
            tileMin
        };

        AddDebugPropInspectionModel(vertices, indices, propShadowIndices, surfaceBuild);

        StaticPropMesh loosePaperInstancedMesh = BuildLoosePaperInstancedMesh();

        AddExitDoorPropGeometry(propBuild, exitPortal);
