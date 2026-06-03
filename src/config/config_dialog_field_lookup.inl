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
