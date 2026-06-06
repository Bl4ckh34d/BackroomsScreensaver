#pragma once

#include "../core/maze_types.h"
#include "../core/constants.h"
#include "../debug/effect_debug_constants.h"

#include "settings_input_bindings.h"
#include "settings_types.h"

struct ImageRGBA {
    int width = 0;
    int height = 0;
    std::vector<uint8_t> pixels;

    bool Valid() const {
        return width > 0 && height > 0 && pixels.size() == static_cast<size_t>(width * height * 4);
    }
};

class ScopedCom {
public:
    ScopedCom();
    ~ScopedCom();
    bool Ok() const;

private:
    HRESULT hr_ = E_FAIL;
};

std::filesystem::path ModuleDirectory();
std::filesystem::path SettingsPath();
std::filesystem::path PackagedSettingsPath();
std::filesystem::path GameSavePath();
std::filesystem::path CacheDirectory();

std::vector<std::filesystem::path> CandidateAssetRoots();
std::filesystem::path FindAssetFile(const wchar_t* filename);
bool LoadImageWic(const std::filesystem::path& path, int targetW, int targetH, ImageRGBA& out);
std::filesystem::path ResolveAsset(const Settings& settings, const std::wstring& filename);

bool WriteTextFile(const std::filesystem::path& path, const std::wstring& text);
std::wstring ReadTextFile(const std::filesystem::path& path);
void EnsureSettingsFile();
std::wstring IniString(const wchar_t* section, const wchar_t* key, const wchar_t* fallback);
float IniFloat(const wchar_t* section, const wchar_t* key, float fallback);
float NormalizeFlashlightShadowBias(float value);
int IniInt(const wchar_t* section, const wchar_t* key, int fallback);
Settings LoadSettings();
std::wstring DefaultConfigText();
int NormalizeAntiAliasingMode(int value);
int NormalizeTextureAnisotropy(int value);
int AntiAliasingMsaaSamples(int value);
bool AntiAliasingUsesFxaa(int value);

bool StartupProfileEnabled();
bool MarkerOrEnvEnabled(const wchar_t* envName, const wchar_t* markerName);
bool BenchmarkDemoEnabled();
float BenchmarkDemoDurationSeconds();
bool AutoplayBenchmarkEnabled();
float AutoplayBenchmarkDurationSeconds();
int AutoplayBenchmarkStartLevel();
int AutoplayBenchmarkBossLevel();
bool AutoplayBenchmarkExploreLevel();
bool RuntimeProfileEnabled();
double ProfileNowMs();
void StartupProfileLine(const std::wstring& line);
void RuntimeProfileFrameLine(const std::wstring& line);
void RuntimeProfileGpuLine(const std::wstring& line);
void RuntimeProfileRenderCpuLine(const std::wstring& line);

class StartupProfile {
public:
    explicit StartupProfile(const wchar_t* name);
    void Mark(const wchar_t* label);

private:
    std::wstring name_;
    bool enabled_ = false;
    double start_ = 0.0;
    double last_ = 0.0;
};

uint32_t ResolveRuntimeSeed(uint32_t configuredSeed);
void ApplyRuntimeVariation(Settings& s, uint32_t seed);
