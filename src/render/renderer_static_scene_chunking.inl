    void ChunkStaticSceneIndices(const std::vector<Vertex>& vertices,
                                 std::vector<uint32_t>& indices,
                                 const std::vector<uint32_t>& liquidIndices,
                                 const std::vector<uint32_t>& transparentIndices,
                                 const std::vector<uint32_t>& propShadowIndices) {
        {
            std::vector<uint32_t> chunkedStaticIndices;
            chunkedStaticIndices.reserve(indices.size());
            UINT oldOpaqueCount = staticSceneGeometry_.floorCeilingStartIndex;
            UINT oldFloorCeilingStart = staticSceneGeometry_.floorCeilingStartIndex;
            UINT oldFloorCeilingCount = staticSceneGeometry_.floorCeilingIndexCount;
            AppendStaticIndexChunks(vertices, indices, 0, oldOpaqueCount, chunkedStaticIndices, staticSceneGeometry_.opaqueChunks, 1, 0);
            staticSceneGeometry_.floorCeilingStartIndex = static_cast<UINT>(chunkedStaticIndices.size());
            AppendStaticIndexChunks(vertices, indices, oldFloorCeilingStart, oldFloorCeilingCount, chunkedStaticIndices, staticSceneGeometry_.floorCeilingChunks);
            staticSceneGeometry_.floorCeilingIndexCount =
                static_cast<UINT>(chunkedStaticIndices.size()) - staticSceneGeometry_.floorCeilingStartIndex;
            indices.swap(chunkedStaticIndices);
        }

        staticSceneGeometry_.waterStartIndex = static_cast<UINT>(indices.size());
        AppendStaticIndexChunks(vertices, liquidIndices, 0, static_cast<UINT>(liquidIndices.size()), indices, staticSceneGeometry_.waterChunks);
        staticSceneGeometry_.waterIndexCount = static_cast<UINT>(indices.size()) - staticSceneGeometry_.waterStartIndex;
        staticSceneGeometry_.transparentStartIndex = static_cast<UINT>(indices.size());
        AppendStaticIndexChunks(vertices, transparentIndices, 0, static_cast<UINT>(transparentIndices.size()), indices, staticSceneGeometry_.transparentChunks);
        staticSceneGeometry_.transparentIndexCount = static_cast<UINT>(indices.size()) - staticSceneGeometry_.transparentStartIndex;
        staticSceneGeometry_.propShadowStartIndex = static_cast<UINT>(indices.size());
        AppendStaticIndexChunks(vertices, propShadowIndices, 0, static_cast<UINT>(propShadowIndices.size()), indices, staticSceneGeometry_.propShadowChunks, 2, 1);
        staticSceneGeometry_.propShadowIndexCount = static_cast<UINT>(indices.size()) - staticSceneGeometry_.propShadowStartIndex;
    }
