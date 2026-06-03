// Startup, runtime, and GPU profiling toggles/logging.

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

double ProfileNowMs() {
    LARGE_INTEGER frequency{};
    LARGE_INTEGER counter{};
    QueryPerformanceFrequency(&frequency);
    QueryPerformanceCounter(&counter);
    return static_cast<double>(counter.QuadPart) * 1000.0 / static_cast<double>(std::max<LONGLONG>(1, frequency.QuadPart));
}

void StartupProfileLine(const std::wstring& line) {
    if (!StartupProfileEnabled()) return;
    std::wofstream out(ModuleDirectory() / L"BackroomsMaze.profile.log", std::ios::app);
    if (out) out << line << L"\r\n";
}

void RuntimeProfileFrameLine(const std::wstring& line) {
    if (!RuntimeProfileEnabled()) return;
    static std::wofstream out(ModuleDirectory() / L"BackroomsMaze.runtime_profile.csv", std::ios::app);
    static bool wroteHeader = false;
    static uint32_t rowsSinceFlush = 0;
    if (!out) return;
    if (!wroteHeader) {
        out << L"frame,mode,total_ms,progress_ms,budget_ms,simulation_ms,audio_ms,render_ms,indices,instanced_indices,instanced_instances,floor_ceiling_indices,water_indices,transparent_indices,dyn_opaque_vertices,dyn_transparent_vertices,air_particles,sparks,steam,lamps\r\n";
        wroteHeader = true;
    }
    out << line << L"\r\n";
    if (++rowsSinceFlush >= 30) {
        out.flush();
        rowsSinceFlush = 0;
    }
}

void RuntimeProfileGpuLine(const std::wstring& line) {
    if (!RuntimeProfileEnabled()) return;
    static std::wofstream out(ModuleDirectory() / L"BackroomsMaze.gpu_profile.csv", std::ios::app);
    static bool wroteHeader = false;
    static uint32_t rowsSinceFlush = 0;
    if (!out) return;
    if (!wroteHeader) {
        out << L"frame,total_ms,clear_ms,dynamic_geometry_ms,flashlight_shadow_ms,fixture_shadow_ms,uploads_ms,main_opaque_ms,floor_ceiling_ms,dynamic_opaque_ms,static_water_ms,static_transparent_ms,dynamic_transparent_ms,post_process_ms,overlays_ms\r\n";
        wroteHeader = true;
    }
    out << line << L"\r\n";
    if (++rowsSinceFlush >= 30) {
        out.flush();
        rowsSinceFlush = 0;
    }
}

void RuntimeProfileRenderCpuLine(const std::wstring& line) {
    if (!RuntimeProfileEnabled()) return;
    static std::wofstream out(ModuleDirectory() / L"BackroomsMaze.render_cpu_profile.csv", std::ios::app);
    static bool wroteHeader = false;
    static uint32_t rowsSinceFlush = 0;
    if (!out) return;
    if (!wroteHeader) {
        out << L"frame,total_ms,clear_ms,dynamic_geometry_ms,flashlight_shadow_ms,fixture_shadow_ms,uploads_ms,main_opaque_ms,floor_ceiling_ms,dynamic_opaque_ms,static_water_ms,static_transparent_ms,dynamic_transparent_ms,post_process_ms,overlays_ms\r\n";
        wroteHeader = true;
    }
    out << line << L"\r\n";
    if (++rowsSinceFlush >= 30) {
        out.flush();
        rowsSinceFlush = 0;
    }
}

StartupProfile::StartupProfile(const wchar_t* name) : name_(name), enabled_(StartupProfileEnabled()) {
    if (!enabled_) return;
    start_ = ProfileNowMs();
    last_ = start_;
    StartupProfileLine(L"[" + name_ + L"]");
}

void StartupProfile::Mark(const wchar_t* label) {
    if (!enabled_) return;
    double now = ProfileNowMs();
    std::wostringstream line;
    line << std::fixed << std::setprecision(3);
    line << name_ << L" " << label
         << L": +" << (now - last_) << L" ms"
         << L", total " << (now - start_) << L" ms";
    StartupProfileLine(line.str());
    last_ = now;
}

