#pragma once

#include "playable_level_spec.h"

#include <sstream>
#include <string>
#include <unordered_map>
struct PlayableLevelResult {
    int layer = 1;
    int levelInLayer = 1;
    float levelSeconds = 0.0f;
    float runSeconds = 0.0f;
    int score = 0;
    bool bossEncounter = false;
};

struct PlayableLevelCompletionUpdate {
    PlayableLevelResult result{};
    bool finalRun = false;
    int levelSecretTotal = 0;
    int levelSecretsFound = 0;
};

struct PlayableCustomScareGate {
    bool allowed = true;
    bool requiresRoll = false;
    float chance = 1.0f;
};

struct PlayableSavePointSpawnPlan {
    bool eligible = false;
    bool mustSpawn = false;
};

using SavedRunKeyValues = std::unordered_map<std::wstring, std::wstring>;

inline SavedRunKeyValues ParseSavedRunKeyValues(const std::wstring& text) {
    SavedRunKeyValues values;
    std::wistringstream in(text);
    std::wstring line;
    while (std::getline(in, line)) {
        if (!line.empty() && line.back() == L'\r') line.pop_back();
        size_t eq = line.find(L'=');
        if (eq == std::wstring::npos) continue;
        values[line.substr(0, eq)] = line.substr(eq + 1);
    }
    return values;
}

inline int SavedRunInt(const SavedRunKeyValues& values, const wchar_t* key, int fallback) {
    auto it = values.find(key);
    if (it == values.end()) return fallback;
    wchar_t* end = nullptr;
    long parsed = std::wcstol(it->second.c_str(), &end, 10);
    return end != it->second.c_str() ? static_cast<int>(parsed) : fallback;
}

inline float SavedRunFloat(const SavedRunKeyValues& values, const wchar_t* key, float fallback) {
    auto it = values.find(key);
    if (it == values.end()) return fallback;
    wchar_t* end = nullptr;
    float parsed = std::wcstof(it->second.c_str(), &end);
    return end != it->second.c_str() ? parsed : fallback;
}

inline std::wstring SavedRunString(const SavedRunKeyValues& values, const wchar_t* key) {
    auto it = values.find(key);
    return it == values.end() ? std::wstring{} : it->second;
}
