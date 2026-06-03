#include "game_settings_panel_internal.h"

bool PtInRectInclusive(const RECT& rc, POINT p) {
    return p.x >= rc.left && p.x <= rc.right && p.y >= rc.top && p.y <= rc.bottom;
}

void FillSolid(HDC dc, const RECT& rc, COLORREF color) {
    HBRUSH brush = CreateSolidBrush(color);
    FillRect(dc, &rc, brush);
    DeleteObject(brush);
}

void DrawTextLine(HDC dc, const std::wstring& text, RECT rc, COLORREF color, UINT format) {
    SetBkMode(dc, TRANSPARENT);
    SetTextColor(dc, color);
    DrawTextW(dc, text.c_str(), -1, &rc, format);
}

void AddGameSettingsHit(GameSettingsPanelState* state, RECT rc, int id, GameSettingsControlKind kind) {
    if (state) state->hits.push_back({rc, id, kind});
}

int FindGameSettingsHitId(GameSettingsPanelState* state, POINT p) {
    if (!state) return 0;
    for (auto it = state->hits.rbegin(); it != state->hits.rend(); ++it) {
        if (PtInRectInclusive(it->rect, p)) return it->id;
    }
    return 0;
}

void DrawGameSettingsButton(HDC dc, GameSettingsPanelState* state, RECT rc, int id, const wchar_t* label, bool active) {
    FillSolid(dc, rc, active ? RGB(126, 96, 48) : RGB(46, 43, 37));
    FrameRect(dc, &rc, GetSysColorBrush(COLOR_GRAYTEXT));
    RECT textRc = rc;
    InflateRect(&textRc, -10, 0);
    DrawTextLine(dc, label, textRc, RGB(235, 229, 210), DT_CENTER | DT_VCENTER | DT_SINGLELINE);
    AddGameSettingsHit(state, rc, id, GameSettingsControlKind::Button);
}

void DrawGameSettingsCheck(HDC dc, GameSettingsPanelState* state, int x, int y, int id, const wchar_t* label, bool checked) {
    RECT box{x, y, x + 18, y + 18};
    FillSolid(dc, box, RGB(25, 24, 21));
    FrameRect(dc, &box, GetSysColorBrush(COLOR_GRAYTEXT));
    if (checked) {
        HPEN pen = CreatePen(PS_SOLID, 3, RGB(220, 176, 84));
        HGDIOBJ old = SelectObject(dc, pen);
        MoveToEx(dc, x + 4, y + 9, nullptr);
        LineTo(dc, x + 8, y + 14);
        LineTo(dc, x + 15, y + 4);
        SelectObject(dc, old);
        DeleteObject(pen);
    }
    RECT labelRc{x + 28, y - 3, x + 280, y + 23};
    DrawTextLine(dc, label, labelRc, RGB(226, 221, 205));
    RECT hit{x, y - 6, x + 300, y + 26};
    AddGameSettingsHit(state, hit, id, GameSettingsControlKind::Check);
}

float SliderValueFromX(const RECT& track, int x, float minValue, float maxValue) {
    float t = std::clamp(static_cast<float>(x - track.left) / static_cast<float>(std::max<LONG>(1, track.right - track.left)), 0.0f, 1.0f);
    return Lerp(minValue, maxValue, t);
}

void DrawGameSettingsSlider(HDC dc, GameSettingsPanelState* state, int x, int y, int id, const wchar_t* label,
    float value, float minValue, float maxValue, const std::wstring& displayValue) {
    RECT labelRc{x, y, x + 230, y + 24};
    DrawTextLine(dc, label, labelRc, RGB(226, 221, 205));
    RECT track{x + 250, y + 8, x + 520, y + 14};
    FillSolid(dc, track, RGB(38, 36, 31));
    float t = Clamp01((value - minValue) / std::max(0.0001f, maxValue - minValue));
    int knobX = track.left + static_cast<int>(std::round(t * (track.right - track.left)));
    RECT fill{track.left, track.top, knobX, track.bottom};
    FillSolid(dc, fill, RGB(172, 126, 48));
    RECT knob{knobX - 6, y + 2, knobX + 6, y + 20};
    FillSolid(dc, knob, RGB(218, 188, 118));
    RECT valueRc{x + 536, y, x + 620, y + 24};
    DrawTextLine(dc, displayValue, valueRc, RGB(226, 221, 205), DT_RIGHT | DT_VCENTER | DT_SINGLELINE);
    RECT hit{track.left - 8, y - 6, track.right + 8, y + 28};
    AddGameSettingsHit(state, hit, id, GameSettingsControlKind::Slider);
}

void DrawGameSettingsDropdown(HDC dc, GameSettingsPanelState* state, int x, int y, int id, const wchar_t* label,
    const std::wstring& value) {
    RECT labelRc{x, y, x + 230, y + 28};
    DrawTextLine(dc, label, labelRc, RGB(226, 221, 205));
    RECT box{x + 250, y, x + 520, y + 30};
    FillSolid(dc, box, RGB(32, 30, 26));
    FrameRect(dc, &box, GetSysColorBrush(COLOR_GRAYTEXT));
    RECT valueRc{box.left + 12, box.top, box.right - 36, box.bottom};
    DrawTextLine(dc, value, valueRc, RGB(235, 229, 210));
    RECT arrow{box.right - 28, box.top + 7, box.right - 10, box.bottom - 7};
    DrawTextLine(dc, L"v", arrow, RGB(220, 176, 84), DT_CENTER | DT_VCENTER | DT_SINGLELINE);
    AddGameSettingsHit(state, box, id, GameSettingsControlKind::Dropdown);

    if (state && state->resolutionDropdownOpen) {
        int maxVisible = std::min<int>(static_cast<int>(state->resolutionOptions.size()), 8);
        for (int i = 0; i < maxVisible; ++i) {
            RECT option{box.left, box.bottom + i * 28, box.right, box.bottom + (i + 1) * 28};
            POINT res = state->resolutionOptions[static_cast<size_t>(i)];
            bool selected = res.x == state->settings.gameResolutionWidth && res.y == state->settings.gameResolutionHeight;
            FillSolid(dc, option, selected ? RGB(126, 96, 48) : RGB(36, 34, 29));
            FrameRect(dc, &option, GetSysColorBrush(COLOR_GRAYTEXT));
            RECT textRc{option.left + 12, option.top, option.right - 12, option.bottom};
            DrawTextLine(dc, FormatResolution(res.x, res.y), textRc, RGB(235, 229, 210));
            AddGameSettingsHit(state, option, kGameSettingsResolutionOptionBase + i, GameSettingsControlKind::DropdownOption);
        }
    }
}

void DrawGameSettingsKeybind(HDC dc, GameSettingsPanelState* state, int x, int y, int actionIndex) {
    if (!state || actionIndex < 0 || actionIndex >= kGameInputActionCount) return;
    const GameInputBindingDef& binding = kGameInputBindings[static_cast<size_t>(actionIndex)];
    RECT labelRc{x, y, x + 230, y + 30};
    DrawTextLine(dc, binding.label, labelRc, RGB(226, 221, 205));

    RECT button{x + 250, y, x + 430, y + 30};
    bool capturing = state->capturingKeyAction == actionIndex;
    FillSolid(dc, button, capturing ? RGB(132, 70, 48) : RGB(46, 43, 37));
    FrameRect(dc, &button, GetSysColorBrush(COLOR_GRAYTEXT));
    RECT textRc = button;
    InflateRect(&textRc, -10, 0);
    std::wstring text = capturing
        ? L"Press a key..."
        : KeyDisplayName(GameActionKey(state->settings, binding.action));
    DrawTextLine(dc, text, textRc, RGB(235, 229, 210), DT_CENTER | DT_VCENTER | DT_SINGLELINE);
    AddGameSettingsHit(state, button, kGameSettingsKeybindBase + actionIndex, GameSettingsControlKind::Button);
}
