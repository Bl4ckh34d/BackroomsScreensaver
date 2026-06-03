// Static prop placement furniture.

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

    bool AddStandingTableProp(StaticPropPlacementBuildContext& build,
                              float px,
                              float pz,
                              float width,
                              float depth,
                              float yaw,
                              float seed) {
        if (renderAssets_.deskPropMesh.vertices.empty()) return false;
        if (!ReserveFloorFootprint(build.floorReservations, px, pz, width + 0.22f, depth + 0.22f,
                yaw, build.tileMin, 0.085f)) {
            return false;
        }
        float topY = 0.70f + seed * 0.05f;
        float scaleX = depth / PropMeshSpan(renderAssets_.deskPropMesh, 0);
        float scaleY = topY + 0.035f;
        float scaleZ = width / PropMeshSpan(renderAssets_.deskPropMesh, 2);
        AddInstancedStaticProp(renderAssets_.deskPropMesh, {px, 0.0f, pz}, yaw + kPi * 0.5f, scaleX, scaleY, scaleZ,
            build.instancedVertices, build.instancedIndices, build.pendingStaticInstances, build.instancedMeshRanges,
            0.0f, -1.0f, true);
        AddBakedPropShadow(build.vertices, build.transparentIndices, build.surface, build.tileAvg,
            px, pz, width, depth, topY + 0.08f, yaw, seed);
        cameraRuntime_.propLookPoints.push_back({px, topY, pz});
        return true;
    }

    bool AddSideTableProp(StaticPropPlacementBuildContext& build,
                          float px,
                          float pz,
                          float width,
                          float depth,
                          float yaw,
                          float seed) {
        if (renderAssets_.deskPropMesh.vertices.empty()) return false;
        if (!ReserveFloorFootprint(build.floorReservations, px, pz, width + 0.24f, depth * 0.72f + 0.36f,
                yaw, build.tileMin, 0.085f)) {
            return false;
        }
        float height = 0.68f + seed * 0.10f;
        float scaleX = depth / PropMeshSpan(renderAssets_.deskPropMesh, 0);
        float scaleY = height + 0.030f;
        float scaleZ = width / PropMeshSpan(renderAssets_.deskPropMesh, 2);
        AddInstancedStaticProp(renderAssets_.deskPropMesh, {px, 0.0f, pz}, yaw + kPi * 0.5f, scaleX, scaleY, scaleZ,
            build.instancedVertices, build.instancedIndices, build.pendingStaticInstances, build.instancedMeshRanges,
            0.0f, -1.0f, true);
        AddBakedPropShadow(build.vertices, build.transparentIndices, build.surface, build.tileAvg,
            px, pz, width, depth, height + 0.06f, yaw, seed);
        cameraRuntime_.propLookPoints.push_back({px, height * 0.72f, pz});
        return true;
    }

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

    bool AddDeskLampOnSurfaceProp(StaticPropPlacementBuildContext& build,
                                  float px,
                                  float pz,
                                  float surfaceY,
                                  float tableYaw,
                                  float tableWidth,
                                  float tableDepth,
                                  float seed) {
        if (renderAssets_.deskLampPropMesh.vertices.empty()) return false;
        float c = std::cos(tableYaw);
        float s = std::sin(tableYaw);
        XMFLOAT3 right{c, 0.0f, -s};
        XMFLOAT3 forward{s, 0.0f, c};
        float ox = (seed - 0.5f) * tableWidth * 0.34f;
        float oz = (LampHash(px + seed * 7.1f, pz - seed * 5.3f) - 0.5f) * tableDepth * 0.30f;
        XMFLOAT3 p = Add3({px, 0.0f, pz}, Add3(Scale3(right, ox), Scale3(forward, oz)));
        constexpr float targetHeight = 0.42f;
        float scale = targetHeight / PropMeshSpan(renderAssets_.deskLampPropMesh, 1);
        AddInstancedStaticPropGrounded(renderAssets_.deskLampPropMesh, {p.x, surfaceY + 0.012f, p.z},
            tableYaw + (seed - 0.5f) * 0.85f, scale, scale, scale,
            build.instancedVertices, build.instancedIndices, build.pendingStaticInstances, build.instancedMeshRanges,
            0.0f, -1.0f, true);
        cameraRuntime_.propLookPoints.push_back({p.x, surfaceY + targetHeight * 0.62f, p.z});
        return true;
    }

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
