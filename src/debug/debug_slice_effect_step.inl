// Effect-slice debug toolbar state and Win32 control updates.
// Included from app_runtime.inl after App and gApp are declared.

DebugSliceEffect StepDebugSliceEffect(DebugSliceEffect effect, int delta) {
    int count = static_cast<int>(DebugSliceEffect::Count);
    int index = (static_cast<int>(effect) + delta) % count;
    if (index < 0) index += count;
    return static_cast<DebugSliceEffect>(index);
}
