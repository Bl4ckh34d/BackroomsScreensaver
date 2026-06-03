// Effect debug viewer state, prop labels, and effect-tuning constants.
// Included from main.cpp before settings and renderer code.

#include "effect_debug_constants.h"
#include "debug_slice_effect.h"
#include "startup_progress.h"

bool gEffectDebugViewer = false;
DebugSliceEffect gDebugSliceEffect = DebugSliceEffect::Blood;
DebugSliceEffect gStartupDebugSliceEffect = DebugSliceEffect::Blood;
int gDebugSliceTiles = 3;
int gDebugPropIndex = 0;
bool gBloodDebugEveryWall = false;
bool gDebugHideMonster = false;
