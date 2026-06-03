#pragma once

struct StartupProgressUpdate {
    const wchar_t* phase = L"";
    const wchar_t* detail = L"";
    int current = 0;
    int total = 1;
    int fineCurrent = 0;
    int fineTotal = 0;
    int shaderDone = 0;
    int shaderTotal = 0;
    int shaderCompiled = 0;
    int shaderCached = 0;
};

using StartupProgressCallback = void (*)(void*, const StartupProgressUpdate&);

struct StartupProgressSink {
    StartupProgressCallback callback = nullptr;
    void* context = nullptr;

    void Report(const StartupProgressUpdate& update) const {
        if (callback) callback(context, update);
    }
};
