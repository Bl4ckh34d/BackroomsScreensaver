    PlayableSavePointSpawnPlan BuildSavePointSpawnPlan() const {
        PlayableSavePointSpawnPlan plan{};
        if (!active || !levelRunning) return plan;
        if (levelInLayer < 3) return plan;

        int remainingBudget = saveItemTarget - saveItemsSpawned;
        if (remainingBudget <= 0) return plan;

        int remainingLevels = std::max(1, kLevelsPerLayer - std::max(3, levelInLayer) + 1);
        plan.eligible = true;
        plan.mustSpawn = remainingBudget >= remainingLevels;
        return plan;
    }

    void WriteLevelStartNotice(std::wostream& out) const {
        out << L"Layer " << layer << L" - Level " << levelInLayer;
        if (CurrentLevelHasBossEncounter()) out << L"  Encounter active";
    }

    void WriteSavedRunFields(std::wostream& out) const {
        const PlayableLevelSpec& spec = currentLevel;
        out << L"Layer=" << layer << L"\n";
        out << L"Level=" << levelInLayer << L"\n";
        out << L"CompletedLevels=" << completedLevels << L"\n";
        out << L"DarkLayerOne=" << (darkLayerOne ? 1 : 0) << L"\n";
        out << L"CustomGame=" << (customGame ? 1 : 0) << L"\n";
        out << L"CustomBrokenLampScares=" << (customSpec.brokenLampScares ? 1 : 0) << L"\n";
        out << L"CustomAirVentScares=" << (customSpec.airVentScares ? 1 : 0) << L"\n";
        out << L"CustomWaterScares=" << (customSpec.waterScares ? 1 : 0) << L"\n";
        out << L"CustomBloodWorldScares=" << (customSpec.bloodWorldScares ? 1 : 0) << L"\n";
        out << L"CustomFleshWorldScares=" << (customSpec.fleshWorldScares ? 1 : 0) << L"\n";
        out << L"CustomOmukadeBoss=" << (customSpec.omukadeBoss ? 1 : 0) << L"\n";
        out << L"CustomEightPages=" << (customSpec.eightPages ? 1 : 0) << L"\n";
        out << L"CustomRoomCount=" << customSpec.roomCount << L"\n";
        out << L"CustomJumpscareChancePercent=" << customSpec.jumpscareChancePercent << L"\n";
        out << L"CustomJumpscareStartMinSeconds=" << customSpec.jumpscareStartMinSeconds << L"\n";
        out << L"CustomJumpscareStartMaxSeconds=" << customSpec.jumpscareStartMaxSeconds << L"\n";
        out << L"CustomScareStartDelaySeconds=" << customScareStartDelaySeconds << L"\n";
        for (size_t i = 0; i < CustomGameSpec::kScareTypeCount; ++i) {
            out << L"CustomScare" << i << L"ChancePercent=" << customSpec.scareChancePercent[i] << L"\n";
            out << L"CustomScare" << i << L"StartMinSeconds=" << customSpec.scareStartMinSeconds[i] << L"\n";
            out << L"CustomScare" << i << L"StartMaxSeconds=" << customSpec.scareStartMaxSeconds[i] << L"\n";
            out << L"CustomScare" << i << L"StartDelaySeconds=" << customScareStartDelayByTypeSeconds[i] << L"\n";
        }
        out << L"SaveItemTarget=" << saveItemTarget << L"\n";
        out << L"SaveItemsSpawned=" << saveItemsSpawned << L"\n";
        out << L"LayerPagesCollected=" << layerPagesCollected << L"\n";
        for (size_t i = 0; i < levelPageTargets.size(); ++i) {
            out << L"LevelPageTarget" << i << L"=" << levelPageTargets[i] << L"\n";
        }
        for (size_t i = 0; i < layerPageCollected.size(); ++i) {
            out << L"LayerPage" << i << L"Collected=" << static_cast<int>(layerPageCollected[i]) << L"\n";
        }
        out << L"RunSeconds=" << runSeconds << L"\n";
        out << L"LevelSeconds=" << levelSeconds << L"\n";
        out << L"TotalScore=" << totalScore << L"\n";
        out << L"SpecMazeWidth=" << spec.mazeWidth << L"\n";
        out << L"SpecMazeHeight=" << spec.mazeHeight << L"\n";
        out << L"SpecBoss=" << (spec.bossEncounter ? 1 : 0) << L"\n";
        out << L"SpecBossChance=" << spec.bossEncounterChance << L"\n";
        out << L"SpecScareTier=" << static_cast<int>(spec.scareTier) << L"\n";
    }
