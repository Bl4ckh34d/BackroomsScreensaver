// Static prop placement context loose papers.

    static float PropPlacementTileHash(int x, int y, float salt) {
        return LampHash(static_cast<float>(x) + salt * 3.17f, static_cast<float>(y) - salt * 5.31f);
    }

    struct StaticPropPlacementBuildContext {
        std::vector<Vertex>& vertices;
        std::vector<uint32_t>& indices;
        std::vector<uint32_t>& transparentIndices;
        std::vector<FloorFootprint>& floorReservations;
        std::vector<Vertex>& instancedVertices;
        std::vector<uint32_t>& instancedIndices;
        std::vector<PendingStaticInstance>& pendingStaticInstances;
        std::vector<InstancedMeshRange>& instancedMeshRanges;
        const MazeSurfaceBuildContext& surface;
        float tileAvg = 0.0f;
        float tileMin = 0.0f;
    };
