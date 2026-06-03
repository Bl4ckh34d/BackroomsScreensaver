    bool AddMetalCabinetAgainstWallProp(StaticPropPlacementBuildContext& build, Tile t, int side, float seed) {
        return AddMetalCabinetAgainstWallProp(build.vertices, build.transparentIndices, build.floorReservations,
            build.instancedVertices, build.instancedIndices, build.pendingStaticInstances, build.instancedMeshRanges,
            build.surface, build.tileAvg, build.tileMin, t, side, seed);
    }
