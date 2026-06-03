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
