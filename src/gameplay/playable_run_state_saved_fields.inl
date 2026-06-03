    static SavedRunRestoreState ReadSavedRunFields(const SavedRunKeyValues& values) {
        SavedRunRestoreState saved{};
        saved.layer = SavedRunInt(values, L"Layer", 1);
        saved.levelInLayer = SavedRunInt(values, L"Level", 1);
        saved.completedLevels = SavedRunInt(values, L"CompletedLevels", 0);
        saved.darkLayerOne = SavedRunInt(values, L"DarkLayerOne", 0) != 0;
        saved.customGame = SavedRunInt(values, L"CustomGame", 0) != 0;
        saved.customSpec.brokenLampScares = SavedRunInt(values, L"CustomBrokenLampScares", 1) != 0;
        saved.customSpec.airVentScares = SavedRunInt(values, L"CustomAirVentScares", 1) != 0;
        saved.customSpec.waterScares = SavedRunInt(values, L"CustomWaterScares", 1) != 0;
        saved.customSpec.bloodWorldScares = SavedRunInt(values, L"CustomBloodWorldScares", 1) != 0;
        saved.customSpec.fleshWorldScares = SavedRunInt(values, L"CustomFleshWorldScares", 1) != 0;
        saved.customSpec.omukadeBoss = SavedRunInt(values, L"CustomOmukadeBoss", 1) != 0;
        saved.customSpec.eightPages = SavedRunInt(values, L"CustomEightPages", 1) != 0;
        saved.customSpec.roomCount = SavedRunInt(values, L"CustomRoomCount", saved.customSpec.roomCount);
        saved.customSpec.jumpscareChancePercent = SavedRunInt(values, L"CustomJumpscareChancePercent", saved.customSpec.jumpscareChancePercent);
        saved.customSpec.jumpscareStartMinSeconds = SavedRunInt(values, L"CustomJumpscareStartMinSeconds", saved.customSpec.jumpscareStartMinSeconds);
        saved.customSpec.jumpscareStartMaxSeconds = SavedRunInt(values, L"CustomJumpscareStartMaxSeconds", saved.customSpec.jumpscareStartMaxSeconds);
        saved.customScareStartDelaySeconds = SavedRunFloat(values, L"CustomScareStartDelaySeconds", 0.0f);
        for (size_t i = 0; i < CustomGameSpec::kScareTypeCount; ++i) {
            std::wstring prefix = L"CustomScare" + std::to_wstring(i);
            saved.customSpec.scareChancePercent[i] = SavedRunInt(values, (prefix + L"ChancePercent").c_str(), saved.customSpec.scareChancePercent[i]);
            saved.customSpec.scareStartMinSeconds[i] = SavedRunInt(values, (prefix + L"StartMinSeconds").c_str(), saved.customSpec.scareStartMinSeconds[i]);
            saved.customSpec.scareStartMaxSeconds[i] = SavedRunInt(values, (prefix + L"StartMaxSeconds").c_str(), saved.customSpec.scareStartMaxSeconds[i]);
            saved.customScareStartDelayByTypeSeconds[i] = SavedRunFloat(values, (prefix + L"StartDelaySeconds").c_str(), saved.customScareStartDelaySeconds);
        }
        saved.saveItemTarget = SavedRunInt(values, L"SaveItemTarget", 1);
        saved.saveItemsSpawned = SavedRunInt(values, L"SaveItemsSpawned", 0);
        for (size_t i = 0; i < saved.levelPageTargets.size(); ++i) {
            std::wstring key = L"LevelPageTarget" + std::to_wstring(i);
            saved.levelPageTargets[i] = SavedRunInt(values, key.c_str(), saved.levelPageTargets[i]);
        }
        saved.layerPagesCollected = SavedRunInt(values, L"LayerPagesCollected", 0);
        for (size_t i = 0; i < saved.layerPageCollected.size(); ++i) {
            std::wstring key = L"LayerPage" + std::to_wstring(i) + L"Collected";
            saved.layerPageCollected[i] = SavedRunInt(values, key.c_str(), 0) != 0 ? 1 : 0;
        }
        saved.runSeconds = SavedRunFloat(values, L"RunSeconds", 0.0f);
        saved.levelSeconds = SavedRunFloat(values, L"LevelSeconds", 0.0f);
        saved.totalScore = SavedRunInt(values, L"TotalScore", 0);
        saved.currentLevel.layer = saved.layer;
        saved.currentLevel.levelInLayer = saved.levelInLayer;
        saved.currentLevel.mazeWidth = std::clamp(SavedRunInt(values, L"SpecMazeWidth", 25), 3, 151);
        saved.currentLevel.mazeHeight = std::clamp(SavedRunInt(values, L"SpecMazeHeight", 25), 3, 151);
        saved.currentLevel.bossEncounter = SavedRunInt(values, L"SpecBoss", 0) != 0;
        saved.currentLevel.bossEncounterChance = std::clamp(SavedRunFloat(values, L"SpecBossChance", 0.0f), 0.0f, 1.0f);
        saved.currentLevel.scareTier = static_cast<PlayableScareTier>(
            std::clamp(SavedRunInt(values, L"SpecScareTier", 0), 0, 4));
        return saved;
    }
