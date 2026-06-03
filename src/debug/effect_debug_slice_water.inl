bool DebugSliceEffectIsWater(DebugSliceEffect effect) {
    return effect == DebugSliceEffect::FloorWater ||
        effect == DebugSliceEffect::CeilingWater ||
        effect == DebugSliceEffect::WallWater;
}
