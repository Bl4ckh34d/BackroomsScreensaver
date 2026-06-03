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
