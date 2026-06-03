    void StartupProfileStaticSceneGeometry(const std::vector<Vertex>& vertices,
                                           const std::vector<uint32_t>& indices,
                                           const std::vector<Vertex>& instancedVertices,
                                           const std::vector<uint32_t>& instancedIndices,
                                           const std::vector<StaticInstanceData>& instancedInstanceData) {
        std::wstringstream counts;
        counts << L"Static scene geometry: vertices=" << vertices.size()
            << L", indices=" << indices.size()
            << L", instancedVertices=" << instancedVertices.size()
            << L", instancedIndices=" << instancedIndices.size()
            << L", instancedInstances=" << instancedInstanceData.size()
            << L", instancedChunks=" << staticSceneGeometry_.instancedOpaqueChunks.size()
            << L", opaqueIndices=" << std::min(staticSceneGeometry_.floorCeilingStartIndex, static_cast<UINT>(indices.size()))
            << L", floorCeilingIndices=" << staticSceneGeometry_.floorCeilingIndexCount
            << L", waterIndices=" << staticSceneGeometry_.waterIndexCount
            << L", transparentIndices=" << staticSceneGeometry_.transparentIndexCount
            << L", propShadowIndices=" << staticSceneGeometry_.propShadowIndexCount
            << L", runtimeLamps=" << effectRuntime_.runtimeLamps.size()
            << L", sparkEmitters=" << effectRuntime_.sparkEmitters.size()
            << L", steamEmitters=" << effectRuntime_.steamEmitters.size();
        StartupProfileLine(counts.str());
    }
