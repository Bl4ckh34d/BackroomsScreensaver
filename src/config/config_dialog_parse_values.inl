int ParseConfigInt(const ConfigState* state, const wchar_t* section, const wchar_t* key, int fallback) {
    std::wstring value = ConfigControlValue(state, section, key, L"");
    wchar_t* end = nullptr;
    long parsed = std::wcstol(value.c_str(), &end, 10);
    return end != value.c_str() ? static_cast<int>(parsed) : fallback;
}

float ParseConfigFloat(const ConfigState* state, const wchar_t* section, const wchar_t* key, float fallback) {
    std::wstring value = ConfigControlValue(state, section, key, L"");
    wchar_t* end = nullptr;
    float parsed = std::wcstof(value.c_str(), &end);
    return end != value.c_str() ? parsed : fallback;
}

std::wstring ParseConfigString(const ConfigState* state, const wchar_t* section, const wchar_t* key, const wchar_t* fallback) {
    return ConfigControlValue(state, section, key, fallback);
}
