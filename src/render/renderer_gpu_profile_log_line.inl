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
