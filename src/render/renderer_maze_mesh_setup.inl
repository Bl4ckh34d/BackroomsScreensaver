// Maze mesh setup/reset helpers.
// Included inside Renderer private section before maze mesh construction.

    void ResetMazeMeshBuildRuntime() {
        cameraRuntime_.propLookPoints.clear();
        effectRuntime_.sparkEmitters.clear();
        effectRuntime_.runtimeLamps.clear();
        staticSceneGeometry_.opaqueChunks.clear();
        staticSceneGeometry_.floorCeilingChunks.clear();
        staticSceneGeometry_.waterChunks.clear();
        staticSceneGeometry_.transparentChunks.clear();
        staticSceneGeometry_.propShadowChunks.clear();
        staticSceneGeometry_.instancedOpaqueChunks.clear();
        staticSceneGeometry_.instancedPropShadowChunks.clear();
        renderBuffers_.instancedVertexBuffer.Reset();
        renderBuffers_.instancedIndexBuffer.Reset();
        renderBuffers_.instancedInstanceBuffer.Reset();
        staticSceneGeometry_.instancedIndexCount = 0;
        staticSceneGeometry_.instancedInstanceCount = 0;
        mapOverlayRuntime_.cachedVerts.clear();
        mapOverlayRuntime_.nextUpdateTime = 0.0f;
        GameWorldRenderSnapshot world = gameWorld_.BuildRenderSnapshot();
        const int mazeCellCount = world.maze ? std::max(0, world.maze->w * world.maze->h) : 0;
        effectRuntime_.lampDamagePixels.assign(static_cast<size_t>(mazeCellCount), 0);
        effectRuntime_.wetFootstepTiles.assign(static_cast<size_t>(mazeCellCount), 0);
        effectRuntime_.wetCeilingTiles.assign(static_cast<size_t>(mazeCellCount), 0);
        effectRuntime_.wetDripEmitters.clear();
        effectRuntime_.wetFloorFootprints.clear();
        effectRuntime_.lampDamageDirty = true;
        effectRuntime_.steamEmitters.clear();
        scareRuntime_.bloodScarePoints.clear();
        exitDoorPresentation_.signLightPosition = {};
        exitDoorPresentation_.signLightStrength = 0.0f;
    }
