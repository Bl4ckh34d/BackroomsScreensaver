    void MarkWetFootstepArea(float px, float pz, float width, float depth, float yaw, float extra = 0.06f, float wetDelaySeconds = 0.0f) {
        WetFloorFootprint fp{};
        fp.center = {px, pz};
        float c = std::cos(yaw);
        float s = std::sin(yaw);
        fp.right = {c, -s};
        fp.forward = {s, c};
        fp.halfW = std::max(0.01f, width * 0.5f + extra);
        fp.halfD = std::max(0.01f, depth * 0.5f + extra);
        fp.wetDelaySeconds = std::max(0.0f, wetDelaySeconds);
        effectRuntime_.wetFloorFootprints.push_back(fp);
        if (effectRuntime_.wetFloorFootprints.size() > 4096) {
            effectRuntime_.wetFloorFootprints.erase(effectRuntime_.wetFloorFootprints.begin(), effectRuntime_.wetFloorFootprints.begin() + 512);
        }

        auto mark = [&](float lx, float lz) {
            float x = px + fp.right.x * lx + fp.forward.x * lz;
            float z = pz + fp.right.y * lx + fp.forward.y * lz;
            MarkWetFootstepTile(gameWorld_.maze.TileFromWorld(x, z));
        };
        MarkWetFootstepTile(gameWorld_.maze.TileFromWorld(px, pz));
        mark(-fp.halfW, -fp.halfD);
        mark( fp.halfW, -fp.halfD);
        mark(-fp.halfW,  fp.halfD);
        mark( fp.halfW,  fp.halfD);
    }
