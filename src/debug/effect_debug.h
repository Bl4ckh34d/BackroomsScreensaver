#pragma once

#include "debug_slice_effect.h"

extern bool gEffectDebugViewer;
extern DebugSliceEffect gDebugSliceEffect;
extern DebugSliceEffect gStartupDebugSliceEffect;
extern int gDebugSliceTiles;
extern int gDebugPropIndex;
extern bool gBloodDebugEveryWall;
extern bool gDebugHideMonster;

extern const int kDebugPropCount;

const wchar_t* DebugSliceEffectName(DebugSliceEffect effect);
bool DebugSliceEffectIsWater(DebugSliceEffect effect);
int WrapDebugPropIndex(int index);
const wchar_t* DebugPropName(int index);
float DebugPropCameraDistanceScale(int index);
float DebugPropCameraTargetY(int index);
