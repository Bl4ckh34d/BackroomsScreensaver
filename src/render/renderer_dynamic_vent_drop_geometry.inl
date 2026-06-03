    void AppendVentDrops(std::vector<Vertex>& verts) {
        GameWorldRenderSnapshot world = gameWorld_.BuildRenderSnapshot();
        float tileAverage = world.maze ? world.maze->TileAverage() : kTile;
        float maxDist = std::max(7.0f, tileAverage * 5.5f);
        for (const VentDrop& d : effectRuntime_.ventDrops) {
            if (!DynamicVisualCandidate(d.pos, std::max(d.halfW, d.halfH) * 2.0f, maxDist)) continue;
            XMFLOAT3 right = RotateYVec({1.0f, 0.0f, 0.0f}, d.yaw);
            XMFLOAT3 forward = RotateYVec({0.0f, 0.0f, 1.0f}, d.yaw);
            XMVECTOR upVec = XMVector3Normalize(XMVectorSet(0, std::cos(d.roll), 0, 0) + XMLoadFloat3(&forward) * std::sin(d.roll));
            XMVECTOR rightVec = XMLoadFloat3(&right);
            XMVECTOR normalVec = XMVector3Normalize(XMVector3Cross(upVec, rightVec));
            XMVECTOR center = XMLoadFloat3(&d.pos);
            XMVECTOR halfW = rightVec * d.halfW;
            XMVECTOR halfH = upVec * d.halfH;
            XMFLOAT3 n, t, a, b, c, e;
            XMStoreFloat3(&n, normalVec);
            XMStoreFloat3(&t, rightVec);
            XMStoreFloat3(&a, center - halfW - halfH);
            XMStoreFloat3(&b, center + halfW - halfH);
            XMStoreFloat3(&c, center + halfW + halfH);
            XMStoreFloat3(&e, center - halfW + halfH);
            AppendDynamicQuad(verts, a, b, c, e, n, t, 10.0f);
            AppendDynamicQuad(verts, b, a, e, c, Scale3(n, -1.0f), Scale3(t, -1.0f), 10.0f);
        }
    }
