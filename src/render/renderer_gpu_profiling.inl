    const wchar_t* GpuProfileMarkerName(GpuProfileMarker marker) const {
        switch (marker) {
        case GpuProfileMarker::FrameStart: return L"FrameStart";
        case GpuProfileMarker::ClearTargets: return L"ClearTargets";
        case GpuProfileMarker::DynamicGeometry: return L"DynamicGeometry";
        case GpuProfileMarker::FlashlightShadow: return L"FlashlightShadow";
        case GpuProfileMarker::FixtureShadow: return L"FixtureShadow";
        case GpuProfileMarker::Uploads: return L"Uploads";
        case GpuProfileMarker::MainOpaque: return L"MainOpaque";
        case GpuProfileMarker::FloorCeiling: return L"FloorCeiling";
        case GpuProfileMarker::DynamicOpaque: return L"DynamicOpaque";
        case GpuProfileMarker::StaticWater: return L"StaticWater";
        case GpuProfileMarker::StaticTransparent: return L"StaticTransparent";
        case GpuProfileMarker::DynamicTransparent: return L"DynamicTransparent";
        case GpuProfileMarker::PostProcess: return L"PostProcess";
        case GpuProfileMarker::Overlays: return L"Overlays";
        case GpuProfileMarker::FrameEnd: return L"FrameEnd";
        default: return L"Unknown";
        }
    }

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

    void ResolveGpuProfileFrame(GpuProfileFrame& frame) {
        if (!gpuProfileRuntime_.available || !d3dRuntime_.context || !frame.issued || frame.open) return;

        D3D11_QUERY_DATA_TIMESTAMP_DISJOINT disjoint{};
        HRESULT hr = d3dRuntime_.context->GetData(frame.disjoint.Get(), &disjoint, sizeof(disjoint), D3D11_ASYNC_GETDATA_DONOTFLUSH);
        if (hr == S_FALSE) return;
        if (FAILED(hr)) {
            frame.issued = false;
            StartupProfileLine(L"GPU profile frame dropped: disjoint query read failed.");
            return;
        }
        if (disjoint.Disjoint || disjoint.Frequency == 0) {
            frame.issued = false;
            StartupProfileLine(L"GPU profile frame dropped: timestamp frequency was disjoint.");
            return;
        }

        std::array<UINT64, kGpuProfileMarkerCount> timestamps{};
        for (size_t i = 0; i < kGpuProfileMarkerCount; ++i) {
            hr = d3dRuntime_.context->GetData(frame.timestamps[i].Get(), &timestamps[i], sizeof(UINT64), D3D11_ASYNC_GETDATA_DONOTFLUSH);
            if (hr == S_FALSE) return;
            if (FAILED(hr)) {
                frame.issued = false;
                StartupProfileLine(L"GPU profile frame dropped: timestamp read failed.");
                return;
            }
        }

        auto elapsedMs = [&](size_t from, size_t to) {
            if (timestamps[to] < timestamps[from]) return 0.0;
            return static_cast<double>(timestamps[to] - timestamps[from]) * 1000.0 /
                static_cast<double>(disjoint.Frequency);
        };

        std::wostringstream line;
        line << std::fixed << std::setprecision(3);
        line << L"GPU frame " << frame.frameId << L": total="
             << elapsedMs(static_cast<size_t>(GpuProfileMarker::FrameStart), static_cast<size_t>(GpuProfileMarker::FrameEnd))
             << L" ms";
        for (size_t i = 1; i < kGpuProfileMarkerCount; ++i) {
            line << L", "
                 << GpuProfileMarkerName(static_cast<GpuProfileMarker>(i))
                 << L"=" << elapsedMs(i - 1, i) << L" ms";
        }
        StartupProfileLine(line.str());

        if (RuntimeProfileEnabled()) {
            std::wostringstream csv;
            csv << std::fixed << std::setprecision(3);
            csv << frame.frameId << L","
                << elapsedMs(static_cast<size_t>(GpuProfileMarker::FrameStart), static_cast<size_t>(GpuProfileMarker::FrameEnd));
            for (size_t i = 1; i < kGpuProfileMarkerCount; ++i) {
                csv << L"," << elapsedMs(i - 1, i);
            }
            RuntimeProfileGpuLine(csv.str());

            std::wostringstream cpuCsv;
            cpuCsv << std::fixed << std::setprecision(3);
            auto cpuElapsedMs = [&](size_t from, size_t to) {
                if (frame.cpuMarkersMs[from] <= 0.0 || frame.cpuMarkersMs[to] <= 0.0 ||
                    frame.cpuMarkersMs[to] < frame.cpuMarkersMs[from]) {
                    return 0.0;
                }
                return frame.cpuMarkersMs[to] - frame.cpuMarkersMs[from];
            };
            cpuCsv << frame.frameId << L","
                << cpuElapsedMs(static_cast<size_t>(GpuProfileMarker::FrameStart), static_cast<size_t>(GpuProfileMarker::FrameEnd));
            for (size_t i = 1; i < kGpuProfileMarkerCount; ++i) {
                cpuCsv << L"," << cpuElapsedMs(i - 1, i);
            }
            RuntimeProfileRenderCpuLine(cpuCsv.str());
        }
        frame.issued = false;
    }

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
