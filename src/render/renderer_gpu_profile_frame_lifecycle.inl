    void BeginGpuProfileFrame() {
        if (!gpuProfileRuntime_.available || !d3dRuntime_.context || gpuProfileRuntime_.frameOpen) return;
        for (GpuProfileFrame& frame : gpuProfileRuntime_.frames) {
            ResolveGpuProfileFrame(frame);
        }

        GpuProfileFrame& frame = gpuProfileRuntime_.frames[gpuProfileRuntime_.writeIndex];
        if (frame.issued || !frame.disjoint) return;

        frame.open = true;
        frame.frameId = ++gpuProfileRuntime_.frameCounter;
        frame.cpuMarkersMs.fill(0.0);
        d3dRuntime_.context->Begin(frame.disjoint.Get());
        gpuProfileRuntime_.frameOpen = true;
        MarkGpuProfile(GpuProfileMarker::FrameStart);
    }

    void MarkGpuProfile(GpuProfileMarker marker) {
        if (!gpuProfileRuntime_.available || !d3dRuntime_.context || !gpuProfileRuntime_.frameOpen) return;
        GpuProfileFrame& frame = gpuProfileRuntime_.frames[gpuProfileRuntime_.writeIndex];
        size_t index = static_cast<size_t>(marker);
        if (index >= kGpuProfileMarkerCount || !frame.timestamps[index]) return;
        frame.cpuMarkersMs[index] = ProfileNowMs();
        d3dRuntime_.context->End(frame.timestamps[index].Get());
    }

    void EndGpuProfileFrame() {
        if (!gpuProfileRuntime_.available || !d3dRuntime_.context || !gpuProfileRuntime_.frameOpen) return;
        GpuProfileFrame& frame = gpuProfileRuntime_.frames[gpuProfileRuntime_.writeIndex];
        MarkGpuProfile(GpuProfileMarker::FrameEnd);
        d3dRuntime_.context->End(frame.disjoint.Get());
        frame.open = false;
        frame.issued = true;
        gpuProfileRuntime_.frameOpen = false;
        gpuProfileRuntime_.writeIndex = (gpuProfileRuntime_.writeIndex + 1) % kGpuProfileFrameCount;
    }
