    static std::wstring WidenAscii(const char* text) {
        if (!text) return {};
        return std::wstring(text, text + std::strlen(text));
    }

    void ReportStartupActivity(const wchar_t* phase, const std::wstring& detail = L"") {
        if (!startupRuntime_.sink || !startupRuntime_.sink->callback) return;
        StartupProgressUpdate update{};
        update.phase = phase;
        update.detail = detail.c_str();
        update.current = startupRuntime_.step;
        update.total = std::max(1, startupRuntime_.total);
        update.fineCurrent = startupRuntime_.fineCurrent;
        update.fineTotal = std::max(1, startupRuntime_.fineTotal);
        update.shaderDone = startupRuntime_.shaderDone;
        update.shaderTotal = startupRuntime_.shaderTotal;
        update.shaderCompiled = startupRuntime_.shaderCompiled;
        update.shaderCached = startupRuntime_.shaderCached;
        startupRuntime_.sink->Report(update);
    }

    void ReportStartupStep(const wchar_t* phase, const std::wstring& detail = L"") {
        startupRuntime_.step = std::min(startupRuntime_.step + 1, std::max(1, startupRuntime_.total));
        startupRuntime_.fineCurrent = std::min(startupRuntime_.step * kStartupProgressUnitsPerStep,
            std::max(1, startupRuntime_.fineTotal));
        ReportStartupActivity(phase, detail);
    }

    void ReportStartupSubStep(const wchar_t* phase, const std::wstring& detail, int subStep) {
        int base = startupRuntime_.step * kStartupProgressUnitsPerStep;
        int fine = base + std::clamp(subStep, 0, kStartupProgressUnitsPerStep - 1);
        startupRuntime_.fineCurrent = std::max(startupRuntime_.fineCurrent,
            std::min(fine, std::max(1, startupRuntime_.fineTotal)));
        ReportStartupActivity(phase, detail);
    }

    std::wstring ShaderProgressDetail(const wchar_t* action, const char* entry, const char* profile, bool completed) const {
        std::wostringstream detail;
        if (startupRuntime_.shaderTotal > 0) {
            int shown = startupRuntime_.shaderDone + (completed ? 0 : 1);
            shown = std::clamp(shown, 0, startupRuntime_.shaderTotal);
            detail << L"Shader " << shown << L"/" << startupRuntime_.shaderTotal;
        } else {
            detail << L"Shaders";
        }
        if (action && *action) detail << L": " << action;
        if (entry && *entry) detail << L" " << WidenAscii(entry);
        if (profile && *profile) detail << L" (" << WidenAscii(profile) << L")";
        if (startupRuntime_.shaderCompiled > 0 || startupRuntime_.shaderCached > 0) {
            detail << L" - compiled " << startupRuntime_.shaderCompiled << L", cached " << startupRuntime_.shaderCached;
        }
        return detail.str();
    }

    void ReportShaderActivity(const wchar_t* action, const char* entry, const char* profile) {
        int subStep = action && std::wcscmp(action, L"Compiling") == 0 ? 2 : 1;
        ReportStartupSubStep(L"Loading shaders", ShaderProgressDetail(action, entry, profile, false), subStep);
    }

    void ReportShaderComplete(const wchar_t* action, const char* entry, const char* profile, bool compiled) {
        ++startupRuntime_.shaderDone;
        if (compiled) {
            ++startupRuntime_.shaderCompiled;
        } else {
            ++startupRuntime_.shaderCached;
        }
        ReportStartupStep(L"Loading shaders", ShaderProgressDetail(action, entry, profile, true));
    }
