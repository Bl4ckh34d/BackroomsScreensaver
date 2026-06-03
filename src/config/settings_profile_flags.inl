bool StartupProfileEnabled() {
    static const bool enabled = []() {
        wchar_t value[8]{};
        if (GetEnvironmentVariableW(L"BACKROOMS_PROFILE_STARTUP", value, ARRAYSIZE(value)) > 0) return true;
        std::error_code ec;
        return std::filesystem::exists(ModuleDirectory() / L"BackroomsMaze.startup_profile.enable", ec);
    }();
    return enabled;
}

bool MarkerOrEnvEnabled(const wchar_t* envName, const wchar_t* markerName) {
    wchar_t value[8]{};
    if (GetEnvironmentVariableW(envName, value, ARRAYSIZE(value)) > 0) return true;
    std::error_code ec;
    return std::filesystem::exists(ModuleDirectory() / markerName, ec);
}

bool BenchmarkDemoEnabled() {
    static const bool enabled = MarkerOrEnvEnabled(
        L"BACKROOMS_BENCHMARK_DEMO",
        L"BackroomsMaze.benchmark_demo.enable");
    return enabled;
}

float BenchmarkDemoDurationSeconds() {
    static const float duration = []() {
        wchar_t value[32]{};
        if (GetEnvironmentVariableW(L"BACKROOMS_BENCHMARK_DEMO_SECONDS", value, ARRAYSIZE(value)) > 0) {
            wchar_t* end = nullptr;
            float seconds = std::wcstof(value, &end);
            if (end != value) return std::clamp(seconds, 0.0f, 3600.0f);
        }
        return 120.0f;
    }();
    return duration;
}

bool RuntimeProfileEnabled() {
    static const bool enabled = []() {
        wchar_t value[8]{};
        if (GetEnvironmentVariableW(L"BACKROOMS_PROFILE_RUNTIME", value, ARRAYSIZE(value)) > 0) return true;
        if (BenchmarkDemoEnabled()) return true;
        std::error_code ec;
        return std::filesystem::exists(ModuleDirectory() / L"BackroomsMaze.runtime_profile.enable", ec);
    }();
    return enabled;
}
