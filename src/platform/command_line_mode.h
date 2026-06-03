#pragma once

#include "../app/run_mode.h"
#include "../debug/debug_slice_effect.h"

struct ParsedRunMode {
    RunMode mode = RunMode::Configure;
    HWND previewParent = nullptr;
    DebugSliceEffect startupDebugSliceEffect = DebugSliceEffect::Blood;
    bool hideDebugMonster = false;
};

ParsedRunMode ParseCommandLineMode(int argc, wchar_t** argv);
