    void CreateGpuProfileQueries() {
        gpuProfileRuntime_.available = false;
        gpuProfileRuntime_.frameOpen = false;
        gpuProfileRuntime_.writeIndex = 0;
        gpuProfileRuntime_.frameCounter = 0;
        for (GpuProfileFrame& frame : gpuProfileRuntime_.frames) {
            frame = {};
        }
        if ((!StartupProfileEnabled() && !RuntimeProfileEnabled()) || !d3dRuntime_.device) return;

        D3D11_QUERY_DESC disjointDesc{};
        disjointDesc.Query = D3D11_QUERY_TIMESTAMP_DISJOINT;
        D3D11_QUERY_DESC timestampDesc{};
        timestampDesc.Query = D3D11_QUERY_TIMESTAMP;

        for (GpuProfileFrame& frame : gpuProfileRuntime_.frames) {
            if (FAILED(d3dRuntime_.device->CreateQuery(&disjointDesc, &frame.disjoint))) {
                StartupProfileLine(L"GPU profile queries unavailable: timestamp disjoint query creation failed.");
                for (GpuProfileFrame& resetFrame : gpuProfileRuntime_.frames) resetFrame = {};
                return;
            }
            for (ComPtr<ID3D11Query>& timestamp : frame.timestamps) {
                if (FAILED(d3dRuntime_.device->CreateQuery(&timestampDesc, &timestamp))) {
                    StartupProfileLine(L"GPU profile queries unavailable: timestamp query creation failed.");
                    for (GpuProfileFrame& resetFrame : gpuProfileRuntime_.frames) resetFrame = {};
                    return;
                }
            }
        }
        gpuProfileRuntime_.available = true;
        StartupProfileLine(L"GPU profile queries ready.");
    }
