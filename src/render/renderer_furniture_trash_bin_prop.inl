    bool AddTrashBinProp(StaticPropPlacementBuildContext& build,
                         float px,
                         float pz,
                         float yaw,
                         bool tipped,
                         float seed) {
        if (renderAssets_.trashBinPropMesh.vertices.empty()) return false;
        seed = std::clamp(seed, 0.0f, 1.0f);
        constexpr float targetHeight = 0.48f;
        float scale = targetHeight / PropMeshSpan(renderAssets_.trashBinPropMesh, 1);
        float diameter = std::max(PropMeshSpan(renderAssets_.trashBinPropMesh, 0), PropMeshSpan(renderAssets_.trashBinPropMesh, 2)) * scale;
        float footprintW = diameter + 0.08f;
        float footprintD = tipped ? targetHeight + 0.16f : footprintW;
        if (!ReserveRealisticFloorFootprint(build.floorReservations, px, pz, footprintW, footprintD,
                yaw, tipped ? 0.065f : 0.055f, seed, build.surface.tileW, build.surface.tileD, build.tileMin)) {
            return false;
        }
        float pitch = tipped ? (seed < 0.5f ? kPi * 0.5f : -kPi * 0.5f) : 0.0f;
        AddInstancedStaticPropGrounded(renderAssets_.trashBinPropMesh, {px, 0.0f, pz}, yaw, scale, scale, scale,
            build.instancedVertices, build.instancedIndices, build.pendingStaticInstances, build.instancedMeshRanges,
            pitch, -1.0f, true);
        AddBakedPropShadow(build.vertices, build.transparentIndices, build.surface, build.tileAvg,
            px, pz, footprintW, footprintD, tipped ? diameter * 0.62f : targetHeight, yaw, seed);
        cameraRuntime_.propLookPoints.push_back({px, tipped ? diameter * 0.45f : targetHeight * 0.55f, pz});
        return true;
    }
