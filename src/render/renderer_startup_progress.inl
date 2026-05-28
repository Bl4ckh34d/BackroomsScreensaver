    static std::wstring WidenAscii(const char* text) {
        if (!text) return {};
        return std::wstring(text, text + std::strlen(text));
    }

    void ReportStartupActivity(const wchar_t* phase, const std::wstring& detail = L"") {
        if (!startupProgress_ || !startupProgress_->callback) return;
        StartupProgressUpdate update{};
        update.phase = phase;
        update.detail = detail.c_str();
        update.current = startupProgressStep_;
        update.total = std::max(1, startupProgressTotal_);
        update.shaderDone = startupShaderDone_;
        update.shaderTotal = startupShaderTotal_;
        update.shaderCompiled = startupShaderCompiled_;
        update.shaderCached = startupShaderCached_;
        startupProgress_->Report(update);
    }

    void ReportStartupStep(const wchar_t* phase, const std::wstring& detail = L"") {
        startupProgressStep_ = std::min(startupProgressStep_ + 1, std::max(1, startupProgressTotal_));
        ReportStartupActivity(phase, detail);
    }

    std::wstring ShaderProgressDetail(const wchar_t* action, const char* entry, const char* profile, bool completed) const {
        std::wostringstream detail;
        if (startupShaderTotal_ > 0) {
            int shown = startupShaderDone_ + (completed ? 0 : 1);
            shown = std::clamp(shown, 0, startupShaderTotal_);
            detail << L"Shader " << shown << L"/" << startupShaderTotal_;
        } else {
            detail << L"Shaders";
        }
        if (action && *action) detail << L": " << action;
        if (entry && *entry) detail << L" " << WidenAscii(entry);
        if (profile && *profile) detail << L" (" << WidenAscii(profile) << L")";
        if (startupShaderCompiled_ > 0 || startupShaderCached_ > 0) {
            detail << L" - compiled " << startupShaderCompiled_ << L", cached " << startupShaderCached_;
        }
        return detail.str();
    }

    void ReportShaderActivity(const wchar_t* action, const char* entry, const char* profile) {
        ReportStartupActivity(L"Loading shaders", ShaderProgressDetail(action, entry, profile, false));
    }

    void ReportShaderComplete(const wchar_t* action, const char* entry, const char* profile, bool compiled) {
        ++startupShaderDone_;
        if (compiled) {
            ++startupShaderCompiled_;
        } else {
            ++startupShaderCached_;
        }
        ReportStartupStep(L"Loading shaders", ShaderProgressDetail(action, entry, profile, true));
    }
