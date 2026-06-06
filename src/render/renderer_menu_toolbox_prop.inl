        if (sessionRuntime_.mode == RendererRuntimeMode::MainMenu && !renderAssets_.toolBoxPropMesh.vertices.empty()) {
            XMFLOAT3 startCenter = maze.WorldCenter(maze.start, 0.0f);
            float northWallZ = startCenter.z + maze.tileD * 0.5f - 0.034f;
            float spanX = std::max(0.001f, PropMeshSpan(renderAssets_.toolBoxPropMesh, 0));
            float spanY = std::max(0.001f, PropMeshSpan(renderAssets_.toolBoxPropMesh, 1));
            float spanZ = std::max(0.001f, PropMeshSpan(renderAssets_.toolBoxPropMesh, 2));
            float targetW = 0.58f;
            float targetH = 0.34f;
            float targetD = 0.32f;
            float scale = std::clamp(std::min(targetW / spanX, std::min(targetH / spanY, targetD / spanZ)), 0.03f, 3.0f);
            XMFLOAT3 floorCenter{startCenter.x + maze.tileW * 0.58f, 0.012f, northWallZ - 0.46f};
            XMFLOAT3 shadowDir = Normalize3(exitDoorPresentation_.normal, {0.0f, 0.0f, 1.0f});
            XMFLOAT3 shadowCenter = Add3(floorCenter, Scale3(shadowDir, maze.TileAverage() * 0.10f));
            AddFloorCard(vertices, transparentIndices, {shadowCenter.x, 0.0f, shadowCenter.z},
                std::max(0.42f, maze.tileW * 0.26f),
                std::max(0.34f, maze.tileD * 0.21f),
                std::atan2(shadowDir.x, shadowDir.z), 0.013f, 24.32f);
            AddInstancedStaticPropGrounded(renderAssets_.toolBoxPropMesh, floorCenter, kPi,
                scale, scale, scale,
                instancedVertices, instancedIndices, pendingStaticInstances, instancedMeshRanges,
                0.0f, 17.0f, true);
        }
