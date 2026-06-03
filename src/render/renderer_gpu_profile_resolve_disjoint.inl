    void ResolveGpuProfileFrame(GpuProfileFrame& frame) {
        if (!gpuProfileRuntime_.available || !d3dRuntime_.context || !frame.issued || frame.open) return;

        D3D11_QUERY_DATA_TIMESTAMP_DISJOINT disjoint{};
        HRESULT hr = d3dRuntime_.context->GetData(frame.disjoint.Get(), &disjoint, sizeof(disjoint), D3D11_ASYNC_GETDATA_DONOTFLUSH);
        if (hr == S_FALSE) return;
        if (FAILED(hr)) {
