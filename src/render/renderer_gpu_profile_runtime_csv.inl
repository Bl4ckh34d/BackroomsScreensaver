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
