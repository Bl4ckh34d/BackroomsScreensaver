#pragma once

#include "../platform/platform_headers.h"
#include "../core/math_utils.h"
#include "../config/settings.h"

#include "game_settings_panel.h"

enum class GameSettingsControlKind {
    Tab,
    Button,
    Check,
    Slider,
    Dropdown,
    DropdownOption
};

struct GameSettingsHit {
    RECT rect{};
    int id = 0;
    GameSettingsControlKind kind = GameSettingsControlKind::Button;
};

struct GameSettingsPanelState {
    Settings settings;
    GameSettingsPanelHost host;
    int tab = 0;
    int draggingSlider = 0;
    bool resolutionDropdownOpen = false;
    int capturingKeyAction = -1;
    std::vector<POINT> resolutionOptions;
    std::vector<GameSettingsHit> hits;
};

constexpr int kGameSettingsTabSystem = 100;
constexpr int kGameSettingsTabGraphics = 101;
constexpr int kGameSettingsTabGame = 102;
constexpr int kGameSettingsTabControls = 103;
constexpr int kGameSettingsTabAudio = 104;
constexpr int kGameSettingsSave = 200;
constexpr int kGameSettingsClose = 201;
constexpr int kGameSettingsFullscreen = 300;
constexpr int kGameSettingsWarp = 301;
constexpr int kGameSettingsInvertY = 302;
constexpr int kGameSettingsMuted = 303;
constexpr int kGameSettingsMonsterIgnorePlayer = 304;
constexpr int kGameSettingsInfiniteStamina = 305;
constexpr int kGameSettingsInvincible = 306;
constexpr int kGameSettingsResolutionDropdown = 400;
constexpr int kGameSettingsExposure = 402;
constexpr int kGameSettingsBloom = 403;
constexpr int kGameSettingsMotionBlur = 404;
constexpr int kGameSettingsAirDensity = 405;
constexpr int kGameSettingsMouseSensitivity = 406;
constexpr int kGameSettingsMasterVolume = 407;
constexpr int kGameSettingsEffectsVolume = 408;
constexpr int kGameSettingsAmbienceVolume = 409;
constexpr int kGameSettingsMonsterVolume = 410;
constexpr int kGameSettingsResolutionOptionBase = 800;
constexpr int kGameSettingsKeybindBase = 900;

std::wstring FormatSettingValue(float value);
std::wstring FormatSettingValue(int value);
std::wstring FormatResolution(int width, int height);
void BuildGameResolutionOptions(GameSettingsPanelState* state);
void WriteIniFloat(const wchar_t* section, const wchar_t* key, float value);
void WriteIniIntValue(const wchar_t* section, const wchar_t* key, int value);
void SaveGameSettingsPanel(const GameSettingsPanelState* state);
bool PtInRectInclusive(const RECT& rc, POINT p);
void FillSolid(HDC dc, const RECT& rc, COLORREF color);
void DrawTextLine(HDC dc, const std::wstring& text, RECT rc, COLORREF color, UINT format = DT_LEFT | DT_VCENTER | DT_SINGLELINE);
void AddGameSettingsHit(GameSettingsPanelState* state, RECT rc, int id, GameSettingsControlKind kind);
int FindGameSettingsHitId(GameSettingsPanelState* state, POINT p);
float SliderValueFromX(const RECT& track, int x, float minValue, float maxValue);
void DrawGameSettingsButton(HDC dc, GameSettingsPanelState* state, RECT rc, int id, const wchar_t* label, bool active = false);
void DrawGameSettingsCheck(HDC dc, GameSettingsPanelState* state, int x, int y, int id, const wchar_t* label, bool checked);
void DrawGameSettingsSlider(HDC dc, GameSettingsPanelState* state, int x, int y, int id, const wchar_t* label,
    float value, float minValue, float maxValue, const std::wstring& valueText);
void DrawGameSettingsDropdown(HDC dc, GameSettingsPanelState* state, int x, int y, int id, const wchar_t* label,
    const std::wstring& valueText);
void DrawGameSettingsKeybind(HDC dc, GameSettingsPanelState* state, int x, int y, int actionIndex);
void ApplyGameSettingsSlider(GameSettingsPanelState* state, int id, int x);
void ToggleGameSetting(GameSettingsPanelState* state, int id);
void NotifyGameSettingsKeyCapture(GameSettingsPanelState* state, bool active, bool escapeConsumed);
void SaveAndCloseGameSettingsPanel(HWND hwnd, GameSettingsPanelState* state);
void PaintGameSettingsPanel(HWND hwnd, HDC dc, GameSettingsPanelState* state);
LRESULT CALLBACK GameSettingsPanelWndProc(HWND hwnd, UINT msg, WPARAM wParam, LPARAM lParam);
