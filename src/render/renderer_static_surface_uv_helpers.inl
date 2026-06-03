    XMFLOAT2 FloorUv(float x, float z) const {
        return {x / settingsRuntime_.live.floorTextureMeters, z / settingsRuntime_.live.floorTextureMeters};
    }

    XMFLOAT2 CeilingUv(float x, float z) const {
        GameWorldRenderSnapshot world = gameWorld_.BuildRenderSnapshot();
        const Maze* maze = world.maze;
        float scaleX = settingsRuntime_.live.ceilingTextureMeters > 0.001f
            ? std::max(0.2f, settingsRuntime_.live.ceilingTextureMeters)
            : std::max(0.2f, (maze ? maze->tileW : kTile) * 2.0f);
        float scaleZ = settingsRuntime_.live.ceilingTextureMeters > 0.001f
            ? std::max(0.2f, settingsRuntime_.live.ceilingTextureMeters)
            : std::max(0.2f, (maze ? maze->tileD : kTile) * 2.0f);
        float originX = maze ? -static_cast<float>(maze->w) * maze->tileW * 0.5f : 0.0f;
        float originZ = maze ? -static_cast<float>(maze->h) * maze->tileD * 0.5f : 0.0f;
        return {(x - originX) / scaleX, (z - originZ) / scaleZ};
    }

    XMFLOAT2 WallUvX(float x, float y) const {
        return {-x / settingsRuntime_.live.wallTextureMeters, -y / settingsRuntime_.live.wallTextureMeters};
    }

    XMFLOAT2 WallUvZ(float z, float y) const {
        return {-z / settingsRuntime_.live.wallTextureMeters, -y / settingsRuntime_.live.wallTextureMeters};
    }

