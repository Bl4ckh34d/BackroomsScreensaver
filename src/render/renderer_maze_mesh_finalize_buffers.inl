        AddMazeCeilingLamps(vertices, indices, surfaceBuild);
        AddMazeFloorCeilingSurfaces(vertices, indices, surfaceBuild);
        ChunkStaticSceneIndices(vertices, indices, liquidIndices, transparentIndices, propShadowIndices);
        BuildStaticInstanceChunks(pendingStaticInstances, instancedMeshRanges, instancedIndices, instancedInstanceData);
        StartupProfileStaticSceneGeometry(vertices, indices, instancedVertices, instancedIndices, instancedInstanceData);
        CreateStaticSceneBuffers(vertices, indices, instancedVertices, instancedIndices, instancedInstanceData);
