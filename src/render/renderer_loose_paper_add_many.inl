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
