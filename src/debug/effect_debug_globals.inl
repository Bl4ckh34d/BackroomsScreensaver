// Effect debug viewer state, prop labels, and effect-tuning constants.
// Included from main.cpp before settings and renderer code.

#include "effect_debug_constants.h"
#include "debug_slice_effect.h"
#include "startup_progress.h"

// Debug controls belong to the UI thread. Background maze builders use the defaults.
thread_local bool gEffectDebugViewer = false;
thread_local DebugSliceEffect gDebugSliceEffect = DebugSliceEffect::Blood;
DebugSliceEffect gStartupDebugSliceEffect = DebugSliceEffect::Blood;
thread_local int gDebugSliceTiles = 3;
thread_local int gDebugPropIndex = 0;
thread_local bool gBloodDebugEveryWall = false;
thread_local bool gDebugHideMonster = false;
