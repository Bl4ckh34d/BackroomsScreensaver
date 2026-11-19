#pragma once

#include "debug_slice_effect.h"

extern thread_local bool gEffectDebugViewer;
extern thread_local DebugSliceEffect gDebugSliceEffect;
extern DebugSliceEffect gStartupDebugSliceEffect;
extern thread_local int gDebugSliceTiles;
extern thread_local int gDebugPropIndex;
extern thread_local bool gBloodDebugEveryWall;
extern thread_local bool gDebugHideMonster;

extern const int kDebugPropCount;

const wchar_t* DebugSliceEffectName(DebugSliceEffect effect);
bool DebugSliceEffectIsWater(DebugSliceEffect effect);
int WrapDebugPropIndex(int index);
const wchar_t* DebugPropName(int index);
float DebugPropCameraDistanceScale(int index);
float DebugPropCameraTargetY(int index);
