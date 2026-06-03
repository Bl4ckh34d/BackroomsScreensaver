const wchar_t* DebugSliceEffectName(DebugSliceEffect effect) {
    switch (effect) {
    case DebugSliceEffect::Blood: return L"Blood";
    case DebugSliceEffect::FloorWater: return L"Floor water";
    case DebugSliceEffect::CeilingWater: return L"Ceiling water";
    case DebugSliceEffect::WallWater: return L"Wall water";
    case DebugSliceEffect::CeilingLamps: return L"Ceiling lamps";
    case DebugSliceEffect::BrokenLamps: return L"Broken lamps";
    case DebugSliceEffect::AirVents: return L"Air vents";
    case DebugSliceEffect::Props: return L"Props";
    default: return L"Unknown";
    }
}
