#pragma once

// Config dialog constants and data types.

constexpr int kConfigTabId = 2001;
constexpr int kConfigSaveId = 2002;
constexpr int kConfigResetId = 2003;
constexpr int kConfigOpenId = 2004;
constexpr int kConfigPreviewId = 2005;
constexpr int kConfigPreviewTimerId = 2006;
constexpr int kConfigScrollId = 2007;
constexpr int kConfigPreviewUpdateId = 2008;
constexpr int kConfigFieldBaseId = 3000;
constexpr int kConfigMaxTabCount = 8;
constexpr int kConfigContentTop = 92;
constexpr int kConfigContentBottom = 646;
constexpr int kConfigScrollStep = 42;
constexpr int kConfigMouseSensitivityId = kConfigFieldBaseId + 166;
constexpr int kConfigInvertMouseYId = kConfigFieldBaseId + 167;
constexpr int kConfigAudioMutedId = kConfigFieldBaseId + 168;
constexpr int kConfigAudioMasterVolumeId = kConfigFieldBaseId + 169;
constexpr int kConfigAudioMusicVolumeId = kConfigFieldBaseId + 170;
constexpr int kConfigAudioEffectsVolumeId = kConfigFieldBaseId + 171;
constexpr int kConfigAudioAmbienceVolumeId = kConfigFieldBaseId + 172;
constexpr int kConfigAudioMonsterVolumeId = kConfigFieldBaseId + 173;
constexpr int kConfigGameFullscreenId = kConfigFieldBaseId + 174;
constexpr int kConfigGameResolutionWidthId = kConfigFieldBaseId + 175;
constexpr int kConfigGameResolutionHeightId = kConfigFieldBaseId + 176;
constexpr int kConfigGameFrameRateLimitId = kConfigFieldBaseId + 177;

enum class ConfigFieldKind {
    Text,
    Bool
};

struct ConfigFieldDef {
    int tab;
    int column;
    int id;
    const wchar_t* group;
    const wchar_t* section;
    const wchar_t* key;
    const wchar_t* label;
    const wchar_t* fallback;
    ConfigFieldKind kind;
    int width;
};

struct ConfigFieldUi {
    const ConfigFieldDef* def = nullptr;
    HWND label = nullptr;
    HWND control = nullptr;
    HWND slider = nullptr;
    int labelBaseY = 0;
    int controlBaseY = 0;
    int sliderBaseY = 0;
};

struct ConfigHeaderUi {
    int tab = 0;
    HWND control = nullptr;
    int baseY = 0;
};

struct ConfigState {
    HWND hwnd = nullptr;
    HWND tab = nullptr;
    HWND note = nullptr;
    HWND scrollPane = nullptr;
    HWND scrollBar = nullptr;
    HWND preview = nullptr;
    HWND previewStatus = nullptr;
    HWND previewHint = nullptr;
    HWND previewUpdateButton = nullptr;
    std::vector<ConfigHeaderUi> headers;
    std::vector<ConfigFieldUi> fields;
    std::vector<ConfigFieldDef> fieldDefs;
    std::vector<std::wstring> tabLabels;
    std::vector<std::wstring> tabNotes;
    std::array<int, kConfigMaxTabCount> scrollOffset{};
    std::array<int, kConfigMaxTabCount> contentHeight{};
    std::unique_ptr<Renderer> previewRenderer;
    ConfigDialogMode mode = ConfigDialogMode::Full;
    int tabCount = 0;
    int activeTab = 0;
    bool loadingControls = false;
    bool previewPending = false;
    ULONGLONG previewApplyAt = 0;
    bool previewOrbitDragging = false;
    bool embedded = false;
    POINT previewLastMouse{};
    bool previewManualOrbit = false;
    float previewOrbitYaw = 0.0f;
    float previewOrbitPitch = -0.18f;
    float previewOrbitDistance = 3.15f;
};

