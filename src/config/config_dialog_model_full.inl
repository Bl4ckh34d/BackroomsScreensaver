void BuildFullConfigModel(ConfigState* state) {
    state->tabLabels.assign(std::begin(kConfigTabs), std::end(kConfigTabs));
    state->tabNotes.assign(std::begin(kConfigNotes), std::end(kConfigNotes));
    state->fieldDefs.assign(std::begin(kConfigFields), std::end(kConfigFields));
}
