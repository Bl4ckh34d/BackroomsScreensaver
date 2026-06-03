    Maze maze;
    PlayerState player;
    MonsterState monster;
    PlayableRunState playableRun;

    std::array<CollectiblePage, kCollectiblePageMaterialCount> collectiblePages{};
    int collectiblePagesCollected = 0;
    SavePoint savePoint;
    std::vector<PlayerAudibleSoundPulse> playerSoundPulses;
    std::vector<GameAudioEvent> audioEvents;

    bool progressionEnabled = false;
    bool exitTransitionActive = false;
    float exitTransitionTimer = 0.0f;
    bool deathActive = false;
    float deathTimer = 0.0f;

