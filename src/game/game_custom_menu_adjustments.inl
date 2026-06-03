// Custom-game menu adjust.

void AdjustCustomGameSize(int& value, int delta) {
    value = std::clamp((value + delta) | 1, 3, 151);
}

void AdjustCustomGameRoomCount(int& value, int delta) {
    value = std::clamp(value + delta, 0, 80);
}

void AdjustCustomGameScareChance(int& value, int delta) {
    value = std::clamp(value + delta, 0, 100);
}

void AdjustCustomGamePercent(int& value, int delta, int maxValue = 100) {
    value = std::clamp(value + delta, 0, maxValue);
}

void AdjustCustomGameMeters(int& value, int delta, int minValue, int maxValue) {
    value = std::clamp(value + delta, minValue, maxValue);
}

void AdjustCustomGameScareStartWindow(CustomGameSpec& spec, bool minValue, int delta) {
    if (minValue) {
        spec.jumpscareStartMinSeconds = std::clamp(spec.jumpscareStartMinSeconds + delta, 0, 600);
        spec.jumpscareStartMaxSeconds = std::max(spec.jumpscareStartMaxSeconds, spec.jumpscareStartMinSeconds);
    } else {
        spec.jumpscareStartMaxSeconds = std::clamp(spec.jumpscareStartMaxSeconds + delta, 0, 600);
        spec.jumpscareStartMinSeconds = std::min(spec.jumpscareStartMinSeconds, spec.jumpscareStartMaxSeconds);
    }
}

int CustomGameScareIndexFromControl(CustomGameMenuControl control) {
    switch (control) {
    case CustomGameMenuControl::BrokenLampScareDetails: return 0;
    case CustomGameMenuControl::AirVentScareDetails: return 1;
    case CustomGameMenuControl::WaterScareDetails: return 2;
    case CustomGameMenuControl::BloodWorldScareDetails: return 3;
    case CustomGameMenuControl::FleshWorldScareDetails: return 4;
    default: return -1;
    }
}

void AdjustSelectedCustomScareChance(int delta) {
    if (!gApp) return;
    int index = std::clamp(gApp->gameCustomSelectedScare, 0, CustomGameSpec::kScareTypeCount - 1);
    gApp->gameCustomSpec.scareChancePercent[static_cast<size_t>(index)] =
        std::clamp(gApp->gameCustomSpec.scareChancePercent[static_cast<size_t>(index)] + delta, 0, 100);
}

void AdjustSelectedCustomScareStartWindow(bool minValue, int delta) {
    if (!gApp) return;
    int index = std::clamp(gApp->gameCustomSelectedScare, 0, CustomGameSpec::kScareTypeCount - 1);
    size_t i = static_cast<size_t>(index);
    if (minValue) {
        gApp->gameCustomSpec.scareStartMinSeconds[i] = std::clamp(gApp->gameCustomSpec.scareStartMinSeconds[i] + delta, 0, 600);
        gApp->gameCustomSpec.scareStartMaxSeconds[i] = std::max(gApp->gameCustomSpec.scareStartMaxSeconds[i], gApp->gameCustomSpec.scareStartMinSeconds[i]);
    } else {
        gApp->gameCustomSpec.scareStartMaxSeconds[i] = std::clamp(gApp->gameCustomSpec.scareStartMaxSeconds[i] + delta, 0, 600);
        gApp->gameCustomSpec.scareStartMinSeconds[i] = std::min(gApp->gameCustomSpec.scareStartMinSeconds[i], gApp->gameCustomSpec.scareStartMaxSeconds[i]);
    }
}
