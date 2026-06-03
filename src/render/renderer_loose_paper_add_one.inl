    bool AddLoosePaperProp(StaticPropPlacementBuildContext& build,
                           const StaticPropMesh& loosePaperInstancedMesh,
                           float px,
                           float pz,
                           float yaw,
                           float lift,
                           float material) {
        if (!FloorFootprintClear(build.floorReservations, px, pz,
                kLoosePaperShortMeters, kLoosePaperLongMeters, yaw, build.tileMin)) {
            return false;
        }
        float paperY = std::clamp(lift, 0.0025f, 0.0105f);
        return AddInstancedStaticProp(loosePaperInstancedMesh, {px, paperY, pz}, yaw, 1.0f, 1.0f, 1.0f,
            build.instancedVertices, build.instancedIndices, build.pendingStaticInstances, build.instancedMeshRanges,
            0.0f, material, false);
    }
