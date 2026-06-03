    float UnitRandom() {
        return static_cast<float>(sessionRuntime_.rng() & 0xffffu) / 65535.0f;
    }

    bool SavedRunExists() const {
        std::error_code ec;
        return std::filesystem::exists(GameSavePath(), ec);
    }

    void DeleteSavedRun() const {
        std::error_code ec;
        std::filesystem::remove(GameSavePath(), ec);
    }

    void ApplyPlayableLevelSpec(const PlayableLevelSpec& spec) {
        float lampFlickerUnit = spec.NeedsLampFlickerRoll() ? UnitRandom() : 0.0f;
        spec.ApplyRuntimeSettings(
            settingsRuntime_.live,
            sessionRuntime_.gameplaySettings,
            lampFlickerUnit,
            gameWorld_.DarkLayerOneAppliesTo(spec.layer));
        if (!spec.bossEncounter) {
            gameWorld_.monster.ClearPursuitState();
            ResetMonsterPresentationState(true, false);
        }
    }
