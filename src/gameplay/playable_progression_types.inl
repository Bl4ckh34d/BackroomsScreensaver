enum class PlayableScareTier : uint8_t {
    None = 0,
    Harmless = 1,
    Water = 2,
    Blood = 3,
    Flesh = 4
};

struct PlayableLevelSpec {
    int layer = 1;
    int levelInLayer = 1;
    int mazeWidth = 15;
    int mazeHeight = 15;
    bool bossEncounter = false;
    float bossEncounterChance = 0.0f;
    PlayableScareTier scareTier = PlayableScareTier::None;
};

struct PlayableLevelResult {
    int layer = 1;
    int levelInLayer = 1;
    float levelSeconds = 0.0f;
    float runSeconds = 0.0f;
    int score = 0;
    bool bossEncounter = false;
};

struct PlayableRunState {
    static constexpr int kLevelsPerLayer = 5;

    bool active = false;
    bool levelRunning = false;
    bool runFinished = false;
    bool scoreScreenActive = false;
    bool scoreScreenFinal = false;
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
    int totalScore = 0;
    PlayableLevelSpec currentLevel{};
    PlayableLevelResult lastResult{};
    std::vector<PlayableLevelResult> completed;
};
