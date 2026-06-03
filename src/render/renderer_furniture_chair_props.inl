    bool AddChairProp(StaticPropPlacementBuildContext& build, XMFLOAT3 c, float yaw, bool waitingChair) {
        const StaticPropMesh* chairMesh = PickChairPropMesh(LampHash(c.x + yaw * 0.37f, c.z + (waitingChair ? 9.1f : 17.4f)), waitingChair);
        if (!chairMesh) return false;
        float px = c.x;
        float pz = c.z;
        float placeYaw = yaw;
        if (!ReserveRealisticFloorFootprint(build.floorReservations, px, pz,
                waitingChair ? 1.02f : 1.05f, waitingChair ? 0.96f : 1.02f,
                placeYaw, 0.075f, LampHash(c.x + 2.3f, c.z - 1.7f),
                build.surface.tileW, build.surface.tileD, build.tileMin)) {
            return false;
        }
        float scale = waitingChair ? 1.04f : 1.10f;
        float fabricVariant = LampHash(px + placeYaw * 1.7f + (waitingChair ? 0.31f : 0.73f), pz - placeYaw * 0.9f);
        AddInstancedStaticPropGrounded(*chairMesh, {px, 0.0f, pz}, placeYaw, scale, scale, scale,
            build.instancedVertices, build.instancedIndices, build.pendingStaticInstances, build.instancedMeshRanges,
            0.0f, -1.0f, true, fabricVariant);
        AddBakedPropShadow(build.vertices, build.transparentIndices, build.surface, build.tileAvg,
            px, pz, waitingChair ? 0.88f : 0.94f, waitingChair ? 0.82f : 0.90f,
            waitingChair ? 0.96f : 1.05f, placeYaw, LampHash(px + 5.1f, pz - 2.8f));
        cameraRuntime_.propLookPoints.push_back({px, 0.72f, pz});
        return true;
    }
