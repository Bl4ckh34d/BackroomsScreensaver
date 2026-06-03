// Config dialog field persistence.

std::wstring ControlText(HWND hwnd) {
    int len = GetWindowTextLengthW(hwnd);
    std::wstring text(static_cast<size_t>(len + 1), L'\0');
    GetWindowTextW(hwnd, text.data(), len + 1);
    text.resize(static_cast<size_t>(len));
    return text;
}

void SaveConfigFieldControl(const ConfigFieldUi& field) {
    if (!field.def || !field.control) return;
    std::wstring value;
    if (field.def->kind == ConfigFieldKind::Bool) {
        value = Button_GetCheck(field.control) == BST_CHECKED ? L"1" : L"0";
    } else {
        value = ControlText(field.control);
    }
    WritePrivateProfileStringW(field.def->section, field.def->key, value.c_str(), SettingsPath().c_str());
}

ConfigFieldUi* FindConfigFieldById(ConfigState* state, int id) {
    if (!state) return nullptr;
    for (auto& field : state->fields) {
        if (field.def && field.def->id == id) return &field;
    }
    return nullptr;
}
