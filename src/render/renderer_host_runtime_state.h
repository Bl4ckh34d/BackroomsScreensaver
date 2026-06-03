#pragma once

struct RendererHostRuntimeState {
    HWND hwnd = nullptr;
    LONG width = 1;
    LONG height = 1;
};

struct RendererTimeRuntimeState {
    float time = 0.0f;
    float effectAnimationStartTime = 0.0f;
    ULONGLONG lastTicks = 0;
};

struct RendererDebugRuntimeState {
    ULONGLONG bloodDebugStartTicks = 0;
    int debugSliceLoopCycle = -1;
};
