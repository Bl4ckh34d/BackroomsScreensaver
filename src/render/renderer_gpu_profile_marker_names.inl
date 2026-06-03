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
