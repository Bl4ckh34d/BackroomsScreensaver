#pragma once

struct StartupProgressRuntimeState {
    const StartupProgressSink* sink = nullptr;
    std::wstring lastInitializeError;
    int step = 0;
    int total = 1;
    int fineCurrent = 0;
    int fineTotal = 1;
    int shaderDone = 0;
    int shaderTotal = 0;
    int shaderCompiled = 0;
    int shaderCached = 0;
};
