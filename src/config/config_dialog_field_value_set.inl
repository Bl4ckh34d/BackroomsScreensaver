void SetFieldControlValue(HWND control, const ConfigFieldDef& def, const std::wstring& value) {
    if (def.kind == ConfigFieldKind::Bool) {
        Button_SetCheck(control, (value == L"1" || ConfigLower(value) == L"true") ? BST_CHECKED : BST_UNCHECKED);
    } else {
        SetWindowTextW(control, value.c_str());
    }
}
