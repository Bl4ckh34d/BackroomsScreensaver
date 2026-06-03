    void AppendSteamBillboards(std::vector<Vertex>& verts) {
        GameWorldRenderSnapshot world = gameWorld_.BuildRenderSnapshot();
        float tileAverage = world.maze ? world.maze->TileAverage() : kTile;
        XMVECTOR cam = XMLoadFloat3(&world.playerPosition);
        XMVECTOR up = XMVectorSet(0, 1, 0, 0);
        float maxDist = std::max(7.0f, std::min(settingsRuntime_.live.fogEndMeters + tileAverage * 2.0f, tileAverage * 5.5f));
        for (size_t i = 0; i < effectRuntime_.steam.size(); ++i) {
            const SteamParticle& sp = effectRuntime_.steam[i];
            float lifeLeft = Clamp01(1.0f - sp.age / std::max(0.001f, sp.life));
            if (lifeLeft <= 0.0f) continue;
            float radius = sp.size * (1.60f + (1.0f - lifeLeft) * 2.2f);
            if (!DynamicBillboardVisible(sp.pos, radius, maxDist, 0.40f)) continue;
            XMFLOAT3 to = Sub3(sp.pos, world.playerPosition);
            float dist = std::sqrt(Dot3(to, to));
            float farT = Clamp01((dist - maxDist * 0.45f) / std::max(0.1f, maxDist * 0.45f));
            if (farT > 0.30f && ((i + static_cast<size_t>(static_cast<int>(timeRuntime_.time * 9.0f))) & 1u) != 0u) continue;
            XMVECTOR pos = XMLoadFloat3(&sp.pos);
            XMVECTOR toCam = XMVector3Normalize(cam - pos);
            XMVECTOR right = XMVector3Normalize(XMVector3Cross(up, toCam));
            float size = sp.size * (0.65f + (1.0f - lifeLeft) * 1.55f);
            if (size * lifeLeft < 0.006f) continue;
            XMVECTOR halfW = right * size;
            XMVECTOR halfH = up * (size * 0.78f);
            XMFLOAT3 n, t, a, b, c, d;
            XMStoreFloat3(&n, toCam);
            XMStoreFloat3(&t, right);
            XMStoreFloat3(&a, pos - halfW - halfH);
            XMStoreFloat3(&b, pos + halfW - halfH);
            XMStoreFloat3(&c, pos + halfW + halfH);
            XMStoreFloat3(&d, pos - halfW + halfH);
            AppendDynamicQuad(verts, a, b, c, d, n, t, 12.58f + lifeLeft * 0.35f);
        }
    }
