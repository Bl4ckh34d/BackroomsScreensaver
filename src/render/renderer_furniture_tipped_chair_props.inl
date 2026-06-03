    bool AddTippedChairProp(StaticPropPlacementBuildContext& build,
                            float px,
                            float pz,
                            float yaw,
                            bool waitingChair,
                            float seed) {
        const StaticPropMesh* chairMesh = PickChairPropMesh(seed, waitingChair);
        if (!chairMesh) return false;
        if (!ReserveRealisticFloorFootprint(build.floorReservations, px, pz,
                waitingChair ? 1.10f : 1.18f, waitingChair ? 1.26f : 1.34f,
                yaw, 0.075f, seed, build.surface.tileW, build.surface.tileD, build.tileMin)) {
            return false;
        }
        float scale = waitingChair ? 1.02f : 1.08f;
        float pitch = seed < 0.5f ? kPi * 0.5f : -kPi * 0.5f;
        AddInstancedStaticPropGrounded(*chairMesh, {px, 0.0f, pz}, yaw, scale, scale, scale,
            build.instancedVertices, build.instancedIndices, build.pendingStaticInstances, build.instancedMeshRanges,
            pitch, -1.0f, true, seed);
        AddBakedPropShadow(build.vertices, build.transparentIndices, build.surface, build.tileAvg,
            px, pz, waitingChair ? 1.10f : 1.18f, waitingChair ? 1.26f : 1.34f, 0.46f, yaw, seed);
        cameraRuntime_.propLookPoints.push_back({px, 0.22f, pz});
        return true;
    }
