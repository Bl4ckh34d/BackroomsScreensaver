        auto contactPoint = [&](XMFLOAT3 root, XMFLOAT3 sideDir, XMFLOAT3 tangentDir, XMFLOAT3 upDir, int limbIndex, int cycle, float maxReach) {
            Tile t = maze.TileFromWorld(root.x, root.z);
            XMFLOAT3 c = maze.WorldCenter(t, 0.0f);
            float halfW = std::max(0.12f, maze.tileW * 0.5f - 0.018f);
            float halfD = std::max(0.12f, maze.tileD * 0.5f - 0.018f);
            float crawl = static_cast<float>((limbIndex * 37 + cycle * 17) & 1023);
            sideDir = Normalize3(sideDir, right);
            tangentDir = Normalize3(tangentDir, forward);
            upDir = Normalize3(upDir, up);
            maxReach = std::max(0.20f, maxReach);
            float reach = std::clamp(maze.TileMinimum() * (0.50f + Rand01(limbIndex * 11 + cycle * 3, 719, sessionRuntime_.runtimeSeed) * 0.18f),
                maxReach * 0.50f, maxReach * 0.82f);
            float sideBias = 0.60f + Rand01(limbIndex * 13 + cycle * 5, 727, sessionRuntime_.runtimeSeed) * 0.32f;
            float tangentBias = (Rand01(limbIndex * 17 + cycle * 7, 731, sessionRuntime_.runtimeSeed) - 0.5f) * 0.58f;
            float verticalBias = (Rand01(limbIndex * 19 + cycle * 11, 733, sessionRuntime_.runtimeSeed) - 0.5f) * 0.72f;
            if (cycle % 5 == 1) verticalBias += 0.42f;
            if (cycle % 7 == 2) verticalBias -= 0.42f;
            XMFLOAT3 radial = Normalize3(Add3(Scale3(sideDir, sideBias),
                Add3(Scale3(tangentDir, tangentBias), Scale3(upDir, verticalBias))), sideDir);
            XMFLOAT3 desired = Add3(root, Scale3(radial, reach));
            desired.y = std::clamp(desired.y, 0.035f, settingsRuntime_.live.wallHeightMeters - 0.045f);
            XMFLOAT3 anchorBase = Lerp3(root, desired, 0.92f);
            float jitterA = (Rand01(limbIndex + cycle * 3, 739, sessionRuntime_.runtimeSeed) - 0.5f) * maze.TileMinimum() * 0.042f;
            float jitterB = (Rand01(limbIndex + cycle * 5, 743, sessionRuntime_.runtimeSeed) - 0.5f) * maze.TileMinimum() * 0.042f;
            float jitterY = (Rand01(limbIndex + cycle * 7, 751, sessionRuntime_.runtimeSeed) - 0.5f) * maze.TileMinimum() * 0.032f;
            float surfaceY = std::clamp(anchorBase.y + jitterY, 0.040f, settingsRuntime_.live.wallHeightMeters - 0.045f);
