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

    static constexpr float kLoosePaperShortMeters = 0.210f;
    static constexpr float kLoosePaperLongMeters = 0.297f;

    static StaticPropMesh BuildLoosePaperInstancedMesh() {
        StaticPropMesh mesh;
        const float hw = kLoosePaperShortMeters * 0.5f;
        const float hd = kLoosePaperLongMeters * 0.5f;
        mesh.min = {-hw, 0.0f, -hd};
        mesh.max = { hw, 0.0f,  hd};
        mesh.vertices = {
            {{-hw, 0.0f,  hd}, {0, 1, 0}, {1, 0, 0}, {0, 0}, static_cast<float>(kRandomLoosePageMaterial)},
            {{ hw, 0.0f,  hd}, {0, 1, 0}, {1, 0, 0}, {1, 0}, static_cast<float>(kRandomLoosePageMaterial)},
            {{ hw, 0.0f, -hd}, {0, 1, 0}, {1, 0, 0}, {1, 1}, static_cast<float>(kRandomLoosePageMaterial)},
            {{-hw, 0.0f,  hd}, {0, 1, 0}, {1, 0, 0}, {0, 0}, static_cast<float>(kRandomLoosePageMaterial)},
            {{ hw, 0.0f, -hd}, {0, 1, 0}, {1, 0, 0}, {1, 1}, static_cast<float>(kRandomLoosePageMaterial)},
            {{-hw, 0.0f, -hd}, {0, 1, 0}, {1, 0, 0}, {0, 1}, static_cast<float>(kRandomLoosePageMaterial)}
        };
        return mesh;
    }

    static float LoosePaperMaterial(float seed, float variantSeed) {
        if (seed < 0.50f) {
            int slot = std::clamp(static_cast<int>(variantSeed * static_cast<float>(kRandomLoosePageAtlasSlots)), 0, kRandomLoosePageAtlasSlots - 1);
            float encodedSlot = (static_cast<float>(slot) + 0.5f) / static_cast<float>(kRandomLoosePageAtlasSlots);
            return static_cast<float>(kRandomLoosePageMaterial) + encodedSlot;
        }
        return static_cast<float>(kRandomLoosePageMaterial);
    }

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

    void AddLoosePapersProp(StaticPropPlacementBuildContext& build,
                            const StaticPropMesh& loosePaperInstancedMesh,
                            Tile t,
                            int count,
                            bool hallwaySpill) {
        XMFLOAT3 c = RenderMazeView().WorldCenter(t, 0.0f);
        float spreadX = hallwaySpill ? build.surface.tileW * 1.55f : build.surface.tileW * 0.82f;
        float spreadZ = hallwaySpill ? build.surface.tileD * 1.55f : build.surface.tileD * 0.82f;
        for (int p = 0; p < count; ++p) {
            float px = c.x + (PropPlacementTileHash(t.x, t.y, 5.0f + p * 1.71f) - 0.5f) * spreadX;
            float pz = c.z + (PropPlacementTileHash(t.x, t.y, 7.0f + p * 1.93f) - 0.5f) * spreadZ;
            float yaw = PropPlacementTileHash(t.x, t.y, 9.0f + p * 2.11f) * kPi * 2.0f;
            float lift = 0.0030f + p * 0.00012f + PropPlacementTileHash(t.x, t.y, 17.0f + p) * 0.0014f;
            float material = LoosePaperMaterial(
                PropPlacementTileHash(t.x, t.y, 23.0f + p * 2.19f),
                PropPlacementTileHash(t.x, t.y, 25.0f + p * 2.31f));
            if (AddLoosePaperProp(build, loosePaperInstancedMesh, px, pz, yaw, lift, material) &&
                PropPlacementTileHash(t.x, t.y, 31.0f + p * 2.37f) < 0.010f) {
                AddCassetteProp(build, px, pz, yaw, lift + 0.003f, PropPlacementTileHash(t.x, t.y, 37.0f + p));
            }
        }
        cameraRuntime_.propLookPoints.push_back({c.x, 0.18f, c.z});
    }
