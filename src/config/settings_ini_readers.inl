void EnsureSettingsFile() {
    std::error_code ec;
    auto path = SettingsPath();
    if (!std::filesystem::exists(path, ec)) {
        auto packaged = PackagedSettingsPath();
        if (std::filesystem::exists(packaged, ec)) {
            std::filesystem::copy_file(packaged, path, std::filesystem::copy_options::overwrite_existing, ec);
        }
        if (!std::filesystem::exists(path, ec)) {
            WriteTextFile(path, DefaultConfigText());
        }
    }
}

std::wstring IniString(const wchar_t* section, const wchar_t* key, const wchar_t* fallback) {
    EnsureSettingsFile();
    wchar_t buffer[1024]{};
    GetPrivateProfileStringW(section, key, fallback, buffer, ARRAYSIZE(buffer), SettingsPath().c_str());
    return buffer;
}

float IniFloat(const wchar_t* section, const wchar_t* key, float fallback) {
    wchar_t fb[64]{};
    swprintf_s(fb, L"%.6g", fallback);
    std::wstring value = IniString(section, key, fb);
    wchar_t* end = nullptr;
    float parsed = std::wcstof(value.c_str(), &end);
    return end != value.c_str() ? parsed : fallback;
}

float NormalizeFlashlightShadowBias(float value) {
    if (!std::isfinite(value)) {
        return 0.00075f;
    }
    if (value > 0.02f) {
        value *= 0.001f;
    }
    return std::clamp(value, 0.00005f, 0.006f);
}

int IniInt(const wchar_t* section, const wchar_t* key, int fallback) {
    return GetPrivateProfileIntW(section, key, fallback, SettingsPath().c_str());
}
