// Config dialog mode-specific model builders.

struct ConfigCreateParams {
    ConfigDialogMode mode = ConfigDialogMode::Full;
    bool embedded = false;
};

const ConfigFieldDef* FindBaseConfigField(const wchar_t* section, const wchar_t* key) {
    for (const auto& def : kConfigFields) {
        if (std::wcscmp(def.section, section) == 0 && std::wcscmp(def.key, key) == 0) return &def;
    }
    return nullptr;
}

void AddConfigFieldCopy(std::vector<ConfigFieldDef>& fields, const wchar_t* section, const wchar_t* key,
    int tab, int column, const wchar_t* group, const wchar_t* labelOverride = nullptr) {
    const ConfigFieldDef* base = FindBaseConfigField(section, key);
    if (!base) return;
    ConfigFieldDef copy = *base;
    copy.tab = tab;
    copy.column = column;
    copy.group = group;
    if (labelOverride) copy.label = labelOverride;
    fields.push_back(copy);
}

void AddCustomConfigField(std::vector<ConfigFieldDef>& fields, int tab, int column, int id,
    const wchar_t* group, const wchar_t* section, const wchar_t* key, const wchar_t* label,
    const wchar_t* fallback, ConfigFieldKind kind, int width) {
    fields.push_back({tab, column, id, group, section, key, label, fallback, kind, width});
}
