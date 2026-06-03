// Shared draw state and dynamic geometry update/profiling.

        UINT stride = sizeof(Vertex);
        UINT offset = 0;
        float blendFactor[4] = {};

        UpdateDynamicGeometry();
        renderProfile.Mark(L"UpdateDynamicGeometry");
        MarkGpuProfile(GpuProfileMarker::DynamicGeometry);
        if (StartupProfileEnabled()) {
            std::wstringstream counts;
            counts << L"Dynamic scene geometry: opaqueVertices=" << dynamicGeometry_.opaqueVertexCount
                << L", transparentVertices=" << dynamicGeometry_.transparentVertexCount
                << L", airParticles=" << effectRuntime_.airParticles.size()
                << L", sparks=" << effectRuntime_.sparks.size()
                << L", steam=" << effectRuntime_.steam.size()
                << L", ventDrops=" << effectRuntime_.ventDrops.size();
            StartupProfileLine(counts.str());
        }

