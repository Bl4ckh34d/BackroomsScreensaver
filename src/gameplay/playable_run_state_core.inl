    struct SavedRunRestoreState {
        int layer = 1;
        int levelInLayer = 1;
        int completedLevels = 0;
        bool darkLayerOne = false;
        bool customGame = false;
        CustomGameSpec customSpec{};
        float customScareStartDelaySeconds = 0.0f;
        std::array<float, CustomGameSpec::kScareTypeCount> customScareStartDelayByTypeSeconds{{0.0f, 0.0f, 0.0f, 0.0f, 0.0f}};
        int saveItemTarget = 1;
        int saveItemsSpawned = 0;
        int layerPagesCollected = 0;
        std::array<int, kLevelsPerLayer> levelPageTargets{{2, 2, 2, 1, 1}};
        std::array<uint8_t, kCollectiblePageMaterialCount> layerPageCollected{};
        float runSeconds = 0.0f;
        float levelSeconds = 0.0f;
        int totalScore = 0;
        PlayableLevelSpec currentLevel{};
    };

    bool active = false;
    bool levelRunning = false;
    bool runFinished = false;
    bool scoreScreenActive = false;
    bool scoreScreenFinal = false;
    bool customGame = false;
    int layer = 1;
    int levelInLayer = 1;
    int completedLevels = 0;
    bool darkLayerOne = false;
    int saveItemTarget = 1;
    int saveItemsSpawned = 0;
    int layerPagesCollected = 0;
    std::array<int, kLevelsPerLayer> levelPageTargets{{2, 2, 2, 1, 1}};
    std::array<uint8_t, kCollectiblePageMaterialCount> layerPageCollected{};
    float runSeconds = 0.0f;
    float levelSeconds = 0.0f;
    float customScareStartDelaySeconds = 0.0f;
    std::array<float, CustomGameSpec::kScareTypeCount> customScareStartDelayByTypeSeconds{{0.0f, 0.0f, 0.0f, 0.0f, 0.0f}};
    int totalScore = 0;
    PlayableLevelSpec currentLevel{};
    CustomGameSpec customSpec{};
    PlayableLevelResult lastResult{};
    std::vector<PlayableLevelResult> completed;

    void Reset() {
        *this = {};
    }

    bool CanSaveActiveRun() const {
        return active && levelRunning;
    }

    bool IsActive() const {
        return active;
    }

    bool IsCustomGame() const {
        return customGame;
    }

    const CustomGameSpec& CustomSpec() const {
        return customSpec;
    }

    const PlayableLevelSpec& CurrentLevel() const {
        return currentLevel;
    }

    bool DarkLayerOneAppliesTo(int targetLayer) const {
        return active && darkLayerOne && targetLayer == 1;
    }

    bool CurrentLevelHasBossEncounter() const {
        return currentLevel.bossEncounter;
    }

    float CurrentLevelSeconds() const {
        return levelSeconds;
    }

    float RunSeconds() const {
        return runSeconds;
    }

    int TotalScore() const {
        return totalScore;
    }
