#pragma once

struct PresentRuntimeState {
    UINT syncInterval = 1;
    UINT flags = 0;
    bool enabled = true;
    bool lastCompleted = true;
};
