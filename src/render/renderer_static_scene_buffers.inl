    void CreateStaticSceneBuffers(const std::vector<Vertex>& vertices,
                                  const std::vector<uint32_t>& indices,
                                  const std::vector<Vertex>& instancedVertices,
                                  const std::vector<uint32_t>& instancedIndices,
                                  const std::vector<StaticInstanceData>& instancedInstanceData) {
        D3D11_BUFFER_DESC vb{};
        vb.ByteWidth = static_cast<UINT>(vertices.size() * sizeof(Vertex));
        vb.Usage = D3D11_USAGE_IMMUTABLE;
        vb.BindFlags = D3D11_BIND_VERTEX_BUFFER;
        D3D11_SUBRESOURCE_DATA vd{vertices.data(), 0, 0};
        d3dRuntime_.device->CreateBuffer(&vb, &vd, &renderBuffers_.vertexBuffer);

        D3D11_BUFFER_DESC ib{};
        ib.ByteWidth = static_cast<UINT>(indices.size() * sizeof(uint32_t));
        ib.Usage = D3D11_USAGE_IMMUTABLE;
        ib.BindFlags = D3D11_BIND_INDEX_BUFFER;
        D3D11_SUBRESOURCE_DATA id{indices.data(), 0, 0};
        d3dRuntime_.device->CreateBuffer(&ib, &id, &renderBuffers_.indexBuffer);
        staticSceneGeometry_.indexCount = static_cast<UINT>(indices.size());

        if (!instancedVertices.empty() && !instancedIndices.empty() && !instancedInstanceData.empty()) {
            D3D11_BUFFER_DESC instVb{};
            instVb.ByteWidth = static_cast<UINT>(instancedVertices.size() * sizeof(Vertex));
            instVb.Usage = D3D11_USAGE_IMMUTABLE;
            instVb.BindFlags = D3D11_BIND_VERTEX_BUFFER;
            D3D11_SUBRESOURCE_DATA instVd{instancedVertices.data(), 0, 0};
            d3dRuntime_.device->CreateBuffer(&instVb, &instVd, &renderBuffers_.instancedVertexBuffer);

            D3D11_BUFFER_DESC instIb{};
            instIb.ByteWidth = static_cast<UINT>(instancedIndices.size() * sizeof(uint32_t));
            instIb.Usage = D3D11_USAGE_IMMUTABLE;
            instIb.BindFlags = D3D11_BIND_INDEX_BUFFER;
            D3D11_SUBRESOURCE_DATA instId{instancedIndices.data(), 0, 0};
            d3dRuntime_.device->CreateBuffer(&instIb, &instId, &renderBuffers_.instancedIndexBuffer);

            D3D11_BUFFER_DESC instanceBufferDesc{};
            instanceBufferDesc.ByteWidth = static_cast<UINT>(instancedInstanceData.size() * sizeof(StaticInstanceData));
            instanceBufferDesc.Usage = D3D11_USAGE_IMMUTABLE;
            instanceBufferDesc.BindFlags = D3D11_BIND_VERTEX_BUFFER;
            D3D11_SUBRESOURCE_DATA instanceData{instancedInstanceData.data(), 0, 0};
            d3dRuntime_.device->CreateBuffer(&instanceBufferDesc, &instanceData, &renderBuffers_.instancedInstanceBuffer);
        }

        D3D11_BUFFER_DESC mb{};
        mb.ByteWidth = sizeof(Vertex) * 6;
        mb.Usage = D3D11_USAGE_DYNAMIC;
        mb.BindFlags = D3D11_BIND_VERTEX_BUFFER;
        mb.CPUAccessFlags = D3D11_CPU_ACCESS_WRITE;
        d3dRuntime_.device->CreateBuffer(&mb, nullptr, &renderBuffers_.monsterBuffer);

        D3D11_BUFFER_DESC db{};
        db.ByteWidth = sizeof(Vertex) * kDynamicVertexCapacity;
        db.Usage = D3D11_USAGE_DYNAMIC;
        db.BindFlags = D3D11_BIND_VERTEX_BUFFER;
        db.CPUAccessFlags = D3D11_CPU_ACCESS_WRITE;
        d3dRuntime_.device->CreateBuffer(&db, nullptr, &renderBuffers_.dynamicBuffer);

        D3D11_BUFFER_DESC ob{};
        ob.ByteWidth = sizeof(OverlayVertex) * kOverlayVertexCapacity;
        ob.Usage = D3D11_USAGE_DYNAMIC;
        ob.BindFlags = D3D11_BIND_VERTEX_BUFFER;
        ob.CPUAccessFlags = D3D11_CPU_ACCESS_WRITE;
        d3dRuntime_.device->CreateBuffer(&ob, nullptr, &renderBuffers_.overlayBuffer);
    }
