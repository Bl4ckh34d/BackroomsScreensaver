// Settings and cache path helpers.

std::filesystem::path ModuleDirectory() {
    wchar_t buffer[MAX_PATH]{};
    DWORD len = GetModuleFileNameW(nullptr, buffer, ARRAYSIZE(buffer));
    if (len == 0) return std::filesystem::current_path();
    return std::filesystem::path(buffer).parent_path();
}

std::filesystem::path SettingsPath() {
    wchar_t localAppData[MAX_PATH]{};
    DWORD len = GetEnvironmentVariableW(L"LOCALAPPDATA", localAppData, ARRAYSIZE(localAppData));
    std::filesystem::path base = (len > 0 && len < ARRAYSIZE(localAppData))
        ? std::filesystem::path(localAppData)
        : ModuleDirectory();
    std::filesystem::path dir = base / L"BackroomsMazeScreensaver";
    std::error_code ec;
    std::filesystem::create_directories(dir, ec);
    return dir / L"BackroomsMaze.ini";
}

std::filesystem::path PackagedSettingsPath() {
    return ModuleDirectory() / L"BackroomsMaze.ini";
}

std::filesystem::path GameSavePath() {
    return SettingsPath().parent_path() / L"BackroomsMaze.save";
}

std::filesystem::path CacheDirectory() {
    wchar_t localAppData[MAX_PATH]{};
    DWORD len = GetEnvironmentVariableW(L"LOCALAPPDATA", localAppData, ARRAYSIZE(localAppData));
    std::filesystem::path base = (len > 0 && len < ARRAYSIZE(localAppData))
        ? std::filesystem::path(localAppData)
        : ModuleDirectory();
    std::filesystem::path dir = base / L"BackroomsMazeScreensaver" / L"Cache";
    std::error_code ec;
    std::filesystem::create_directories(dir, ec);
    return dir;
}

