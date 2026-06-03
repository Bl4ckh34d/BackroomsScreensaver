    void AppendSparkBillboards(std::vector<Vertex>& verts) {
        GameWorldRenderSnapshot world = gameWorld_.BuildRenderSnapshot();
        float tileAverage = world.maze ? world.maze->TileAverage() : kTile;
        XMVECTOR cam = XMLoadFloat3(&world.playerPosition);
        XMVECTOR up = XMVectorSet(0, 1, 0, 0);
        float maxDist = std::max(6.0f, std::min(settingsRuntime_.live.fogEndMeters + tileAverage * 2.0f, tileAverage * 6.0f));
        for (size_t i = 0; i < effectRuntime_.sparks.size(); ++i) {
            const SparkParticle& spark = effectRuntime_.sparks[i];
            float lifeLeft = Clamp01(1.0f - spark.age / std::max(0.001f, spark.life));
            if (lifeLeft <= 0.0f) continue;
            float radius = spark.size * (1.30f + lifeLeft * 2.35f);
            if (!DynamicBillboardVisible(spark.pos, radius, maxDist, 0.34f)) continue;
            XMFLOAT3 to = Sub3(spark.pos, world.playerPosition);
            float distSq = Dot3(to, to);
            float farT = Clamp01((std::sqrt(distSq) - maxDist * 0.48f) / std::max(0.1f, maxDist * 0.42f));
            if (farT > 0.35f && ((i + static_cast<size_t>(static_cast<int>(timeRuntime_.time * 18.0f))) & 1u) != 0u) continue;
            XMVECTOR pos = XMLoadFloat3(&spark.pos);
            XMVECTOR toCam = XMVector3Normalize(cam - pos);
            XMVECTOR right = XMVector3Normalize(XMVector3Cross(up, toCam));
            float size = spark.size * (0.55f + lifeLeft * lifeLeft * 1.35f);
            if (size * lifeLeft < 0.004f) continue;
            XMVECTOR halfW = right * size;
            XMVECTOR halfH = up * size;
            XMFLOAT3 n, t, a, b, c, d;
            XMStoreFloat3(&n, toCam);
            XMStoreFloat3(&t, right);
            XMStoreFloat3(&a, pos - halfW - halfH);
            XMStoreFloat3(&b, pos + halfW - halfH);
            XMStoreFloat3(&c, pos + halfW + halfH);
            XMStoreFloat3(&d, pos - halfW + halfH);
            AppendDynamicQuad(verts, a, b, c, d, n, t, 13.05f + lifeLeft * 0.38f);
        }
    }
