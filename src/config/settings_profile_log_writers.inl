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
        out << L"frame,mode,total_ms,progress_ms,budget_ms,simulation_ms,audio_ms,render_ms,indices,instanced_indices,instanced_instances,floor_ceiling_indices,water_indices,transparent_indices,dyn_opaque_vertices,dyn_transparent_vertices,air_particles,sparks,steam,lamps,level,level_running,score_screen,exit_transition,boss,run_seconds,level_seconds,autoplay_seconds,player_tile_x,player_tile_y,monster_distance,monster_visible,death_active\r\n";
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
