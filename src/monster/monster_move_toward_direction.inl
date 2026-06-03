
        if (distance <= 0.0001f) return;
        XMFLOAT3 start = gameWorld_.monster.position;
        XMFLOAT3 delta{target.x - start.x, 0.0f, target.z - start.z};
        float len = Length3(delta);
        if (len <= 0.0001f) return;
        XMFLOAT3 directDir = Scale3(delta, 1.0f / len);
        XMFLOAT3 dir = directDir;
        bool erraticChase = gameWorld_.monster.chasingVisible || gameWorld_.monster.recognizedForChase || gameWorld_.monster.chaseMemoryTimer > 0.0f;
        if (erraticChase && len > gameWorld_.maze.TileMinimum() * 0.24f) {
            XMFLOAT3 lateral{dir.z, 0.0f, -dir.x};
            float burst = std::pow(std::max(0.0f, std::sin(timeRuntime_.time * 1.15f + gameWorld_.monster.position.x * 0.18f - gameWorld_.monster.position.z * 0.13f)), 5.0f);
            float twitch = std::sin(timeRuntime_.time * 2.6f + gameWorld_.monster.position.z * 0.27f) * 0.028f +
                std::sin(timeRuntime_.time * 4.1f + gameWorld_.monster.position.x * 0.17f) * 0.012f;
            float chaseBlend = std::max(monsterPresentation_.headChaseBlend, gameWorld_.monster.chasingVisible ? 0.72f : 0.36f);
            dir = Normalize3(Add3(dir, Scale3(lateral, twitch * (0.68f + burst * 0.32f) * chaseBlend)), dir);
        }
