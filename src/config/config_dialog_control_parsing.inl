// Config dialog parsing.

std::wstring ConfigLower(std::wstring value) {
    std::transform(value.begin(), value.end(), value.begin(), [](wchar_t c) {
        return static_cast<wchar_t>(std::towlower(c));
    });
    return value;
}

void SetFieldControlValue(HWND control, const ConfigFieldDef& def, const std::wstring& value) {
    if (def.kind == ConfigFieldKind::Bool) {
        Button_SetCheck(control, (value == L"1" || ConfigLower(value) == L"true") ? BST_CHECKED : BST_UNCHECKED);
    } else {
        SetWindowTextW(control, value.c_str());
    }
}

void LoadConfigControls(ConfigState* state, bool defaultsOnly) {
    state->loadingControls = true;
    for (auto& field : state->fields) {
        std::wstring value = defaultsOnly
            ? field.def->fallback
            : IniString(field.def->section, field.def->key, field.def->fallback);
        SetFieldControlValue(field.control, *field.def, value);
    }
    state->loadingControls = false;
}

void SaveConfigControls(ConfigState* state) {
    for (const auto& field : state->fields) {
        std::wstring value;
        if (field.def->kind == ConfigFieldKind::Bool) {
            value = Button_GetCheck(field.control) == BST_CHECKED ? L"1" : L"0";
        } else {
            value = ControlText(field.control);
        }
        WritePrivateProfileStringW(field.def->section, field.def->key, value.c_str(), SettingsPath().c_str());
    }
}

void ResetVisibleConfigControls(ConfigState* state) {
    if (!state) return;
    state->loadingControls = true;
    for (auto& field : state->fields) {
        SetFieldControlValue(field.control, *field.def, field.def->fallback);
    }
    state->loadingControls = false;
    SaveConfigControls(state);
}

const ConfigFieldUi* FindConfigField(const ConfigState* state, const wchar_t* section, const wchar_t* key) {
    for (const auto& field : state->fields) {
        if (wcscmp(field.def->section, section) == 0 && wcscmp(field.def->key, key) == 0) return &field;
    }
    return nullptr;
}

std::wstring ConfigControlValue(const ConfigState* state, const wchar_t* section, const wchar_t* key, const wchar_t* fallback) {
    const ConfigFieldUi* field = FindConfigField(state, section, key);
    if (!field) return IniString(section, key, fallback);
    if (field->def->kind == ConfigFieldKind::Bool) {
        return Button_GetCheck(field->control) == BST_CHECKED ? L"1" : L"0";
    }
    return ControlText(field->control);
}

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
