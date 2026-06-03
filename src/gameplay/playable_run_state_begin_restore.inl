    void BeginLayerRun(bool useDarkLayerOne, std::mt19937& rng) {
        Reset();
        active = true;
        levelRunning = false;
        runFinished = false;
        layer = 1;
        levelInLayer = 1;
        darkLayerOne = useDarkLayerOne;
        saveItemTarget = PickSaveItemTarget(rng);
        GenerateLayerPageDistribution(rng);
        completed.reserve(kLevelsPerLayer);
    }

    void BeginCustomRun(const CustomGameSpec& spec, std::mt19937& rng) {
        Reset();
        active = true;
        levelRunning = true;
        runFinished = false;
        customGame = true;
        customSpec = spec;
        layer = spec.layer;
        levelInLayer = 1;
        darkLayerOne = false;
        saveItemTarget = PickSaveItemTarget(rng);
        completed.reserve(1);
        DisableLayerPages();
        if (spec.eightPages) levelPageTargets[0] = kCollectiblePageMaterialCount;
        customScareStartDelaySeconds = RandRange(
            static_cast<float>(spec.jumpscareStartMinSeconds),
            static_cast<float>(spec.jumpscareStartMaxSeconds),
            rng);
        for (size_t i = 0; i < CustomGameSpec::kScareTypeCount; ++i) {
            customScareStartDelayByTypeSeconds[i] = RandRange(
                static_cast<float>(spec.scareStartMinSeconds[i]),
                static_cast<float>(spec.scareStartMaxSeconds[i]),
                rng);
        }
    }

    void BeginLevel(const PlayableLevelSpec& spec) {
        levelInLayer = std::clamp(spec.levelInLayer, 1, kLevelsPerLayer);
        currentLevel = spec;
        levelSeconds = 0.0f;
        levelRunning = true;
        scoreScreenActive = false;
        scoreScreenFinal = false;
    }

    int RestoreSaveItemsSpawned(int spawned) {
        saveItemsSpawned = std::clamp(spawned, 0, saveItemTarget);
        return saveItemsSpawned;
    }

    int RestoreSavedRunState(SavedRunRestoreState saved) {
        Reset();
        active = true;
        levelRunning = true;
        runFinished = false;
        scoreScreenActive = false;
        scoreScreenFinal = false;
        layer = std::max(1, saved.layer);
        levelInLayer = std::clamp(saved.levelInLayer, 1, kLevelsPerLayer);
        completedLevels = std::max(0, saved.completedLevels);
        darkLayerOne = saved.darkLayerOne;
        customGame = saved.customGame;
        customSpec = saved.customSpec;
        customSpec.Normalize();
        customScareStartDelaySeconds = std::clamp(saved.customScareStartDelaySeconds, 0.0f, 600.0f);
        for (size_t i = 0; i < customScareStartDelayByTypeSeconds.size(); ++i) {
            customScareStartDelayByTypeSeconds[i] = std::clamp(
                saved.customScareStartDelayByTypeSeconds[i],
                0.0f,
                600.0f);
        }
        saveItemTarget = std::clamp(saved.saveItemTarget, 1, 3);
        int restoredSaveItemsSpawned = RestoreSaveItemsSpawned(saved.saveItemsSpawned);
        for (size_t i = 0; i < levelPageTargets.size(); ++i) {
            levelPageTargets[i] = std::clamp(saved.levelPageTargets[i], 0, kCollectiblePageMaterialCount);
        }
        EnsureLayerPageDistribution();
        SetLayerPagesCollectedCount(saved.layerPagesCollected);
        for (size_t i = 0; i < layerPageCollected.size(); ++i) {
            RestoreLayerPageCollected(static_cast<int>(i), saved.layerPageCollected[i] != 0);
        }
        if (customGame && !customSpec.eightPages) {
            DisableLayerPages();
        }
        runSeconds = std::max(0.0f, saved.runSeconds);
        levelSeconds = std::max(0.0f, saved.levelSeconds);
        totalScore = std::max(0, saved.totalScore);
        currentLevel = saved.currentLevel;
        currentLevel.layer = layer;
        currentLevel.levelInLayer = levelInLayer;
        if (customGame) {
            customSpec.layer = currentLevel.layer;
            customSpec.mazeWidth = currentLevel.mazeWidth;
            customSpec.mazeHeight = currentLevel.mazeHeight;
        }
        return restoredSaveItemsSpawned;
    }
