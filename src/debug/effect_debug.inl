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

bool DebugSliceEffectIsWater(DebugSliceEffect effect) {
    return effect == DebugSliceEffect::FloorWater ||
        effect == DebugSliceEffect::CeilingWater ||
        effect == DebugSliceEffect::WallWater;
}

constexpr int kDebugPropCount = 16;

int WrapDebugPropIndex(int index) {
    int count = std::max(1, kDebugPropCount);
    index %= count;
    if (index < 0) index += count;
    return index;
}

const wchar_t* DebugPropName(int index) {
    switch (WrapDebugPropIndex(index)) {
    case 0: return L"Office chair modern";
    case 1: return L"Office chair classic";
    case 2: return L"Office chair task";
    case 3: return L"Office chair tipped";
    case 4: return L"Filing cabinet";
    case 5: return L"Office desk";
    case 6: return L"Trash bin upright";
    case 7: return L"Trash bin tipped";
    case 8: return L"Desk lamp";
    case 9: return L"Audio cassette";
    case 10: return L"Air vent";
    case 11: return L"Exit sign";
    case 12: return L"Ceiling lamp 01";
    case 13: return L"Ceiling lamp 02";
    case 14: return L"Ceiling lamp 03";
    case 15: return L"Ceiling lamp 04";
    default: return L"Prop";
    }
}

float DebugPropCameraDistanceScale(int index) {
    switch (WrapDebugPropIndex(index)) {
    case 4:
    case 5:
        return 2.15f;
    case 8:
    case 9:
        return 1.12f;
    case 10:
    case 11:
        return 1.30f;
    default:
        return 1.62f;
    }
}

float DebugPropCameraTargetY(int index) {
    switch (WrapDebugPropIndex(index)) {
    case 8:
    case 9:
        return 0.22f;
    case 4:
        return 0.76f;
    case 5:
        return 0.58f;
    case 10:
    case 11:
        return 0.46f;
    default:
        return 0.52f;
    }
}

