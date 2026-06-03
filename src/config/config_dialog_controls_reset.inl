void ResetVisibleConfigControls(ConfigState* state) {
    if (!state) return;
    state->loadingControls = true;
    for (auto& field : state->fields) {
        SetFieldControlValue(field.control, *field.def, field.def->fallback);
    }
    state->loadingControls = false;
    SaveConfigControls(state);
}
