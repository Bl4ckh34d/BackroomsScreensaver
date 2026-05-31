    float UnitRandom() {
        return static_cast<float>(rng_() & 0xffffu) / 65535.0f;
    }

    int PickLayerSaveItemTarget() {
        return 1 + static_cast<int>(rng_() % 3u);
    }

    void GenerateLayerPageDistribution() {
        playableRun_.levelPageTargets.fill(1);
        std::array<int, PlayableRunState::kLevelsPerLayer> order{{0, 1, 2, 3, 4}};
        std::shuffle(order.begin(), order.end(), rng_);
        for (int i = 0; i < 3; ++i) {
            playableRun_.levelPageTargets[static_cast<size_t>(order[static_cast<size_t>(i)])] = 2;
        }
        playableRun_.layerPageCollected.fill(0);
        playableRun_.layerPagesCollected = 0;
    }

    void EnsureLayerPageDistribution() {
        int total = 0;
        for (int count : playableRun_.levelPageTargets) total += count;
        if (total == kCollectiblePageMaterialCount) return;
        playableRun_.levelPageTargets = {{2, 2, 2, 1, 1}};
    }

    int LayerPageStartForLevel(int levelInLayer) const {
        int clamped = std::clamp(levelInLayer, 1, PlayableRunState::kLevelsPerLayer);
        int start = 0;
        for (int i = 1; i < clamped; ++i) {
            start += playableRun_.levelPageTargets[static_cast<size_t>(i - 1)];
        }
        return std::clamp(start, 0, kCollectiblePageMaterialCount);
    }

    int LayerPageCountForLevel(int levelInLayer) const {
        int index = std::clamp(levelInLayer, 1, PlayableRunState::kLevelsPerLayer) - 1;
        int start = LayerPageStartForLevel(levelInLayer);
        return std::clamp(playableRun_.levelPageTargets[static_cast<size_t>(index)], 0, kCollectiblePageMaterialCount - start);
    }

    bool PickChance(float chance) {
        return chance > 0.0f && UnitRandom() < chance;
    }

    bool SavedRunExists() const {
        std::error_code ec;
        return std::filesystem::exists(GameSavePath(), ec);
    }

    void DeleteSavedRun() const {
        std::error_code ec;
        std::filesystem::remove(GameSavePath(), ec);
    }

    PlayableLevelSpec BuildLayerOneLevelSpec(int levelInLayer) {
        PlayableLevelSpec spec{};
        spec.layer = playableRun_.layer;
        spec.levelInLayer = std::clamp(levelInLayer, 1, PlayableRunState::kLevelsPerLayer);

        switch (spec.levelInLayer) {
        case 1:
            spec.mazeWidth = 15;
            spec.mazeHeight = 15;
            spec.scareTier = PlayableScareTier::None;
            spec.bossEncounterChance = 0.0f;
            break;
        case 2:
            if ((rng_() & 1u) == 0) {
                spec.mazeWidth = 20;
                spec.mazeHeight = 10;
            } else {
                spec.mazeWidth = 10;
                spec.mazeHeight = 20;
            }
            spec.scareTier = PlayableScareTier::Harmless;
            spec.bossEncounterChance = 0.05f;
            break;
        case 3:
            spec.mazeWidth = 25;
            spec.mazeHeight = 25;
            spec.scareTier = PlayableScareTier::Water;
            spec.bossEncounterChance = 0.15f;
            break;
        case 4:
            if ((rng_() & 1u) == 0) {
                spec.mazeWidth = 30;
                spec.mazeHeight = 15;
            } else {
                spec.mazeWidth = 15;
                spec.mazeHeight = 30;
            }
            spec.scareTier = PlayableScareTier::Blood;
            spec.bossEncounterChance = 0.25f;
            break;
        default:
            {
                const std::array<Tile, 5> choices = {{{35, 20}, {40, 15}, {15, 40}, {35, 20}, {27, 27}}};
                Tile size = choices[static_cast<size_t>(rng_() % choices.size())];
                spec.mazeWidth = size.x;
                spec.mazeHeight = size.y;
            }
            spec.scareTier = PlayableScareTier::Flesh;
            spec.bossEncounter = true;
            spec.bossEncounterChance = 1.0f;
            break;
        }

        if (!spec.bossEncounter) spec.bossEncounter = PickChance(spec.bossEncounterChance);
        return spec;
    }

    void ApplyPlayableLevelSpec(const PlayableLevelSpec& spec) {
        const int scareTier = static_cast<int>(spec.scareTier);
        settings_ = gameplaySettings_;
        settings_.mazeWidth = spec.mazeWidth;
        settings_.mazeHeight = spec.mazeHeight;
        settings_.roomCount = spec.levelInLayer <= 2 ? 0 : std::min(3, spec.levelInLayer - 2);
        const float loosePaperScaleByLevel[] = {0.0f, 0.055f, 0.24f, 0.58f, 1.0f};
        const float hallwayPaperScaleByLevel[] = {0.0f, 0.035f, 0.18f, 0.50f, 1.0f};
        const float clutterScaleByLevel[] = {0.0f, 0.035f, 0.18f, 0.48f, 1.0f};
        int levelIndex = std::clamp(spec.levelInLayer, 1, PlayableRunState::kLevelsPerLayer) - 1;
        settings_.paperDensity *= loosePaperScaleByLevel[levelIndex];
        settings_.hallwayPaperRunDensity *= hallwayPaperScaleByLevel[levelIndex];
        settings_.chairDensity *= clutterScaleByLevel[levelIndex];
        settings_.lampOnRatio = 1.0f;
        settings_.brokenZoneRatio = spec.scareTier == PlayableScareTier::None ? 0.0f : 0.05f;
        settings_.lampFlickerRatio = spec.scareTier == PlayableScareTier::None ? 0.0f :
            Lerp(0.04f, 0.10f, UnitRandom());
        settings_.sparkParticles = spec.scareTier != PlayableScareTier::None && settings_.sparkParticles;
        if (playableRun_.active && playableRun_.darkLayerOne && spec.layer == 1) {
            settings_.lampOnRatio = 0.0f;
            settings_.lampFlickerRatio = 0.0f;
            settings_.ambientLight = std::min(settings_.ambientLight, 0.002f);
        }

        settings_.jumpscareFrequency = spec.scareTier == PlayableScareTier::None ? 0.0f :
            std::max(settings_.jumpscareFrequency, 0.12f);
        settings_.waterDamageEnabled = scareTier >= static_cast<int>(PlayableScareTier::Water);
        settings_.waterDamageDensity = scareTier >= static_cast<int>(PlayableScareTier::Water) ? std::max(settings_.waterDamageDensity, 0.70f) : 0.0f;
        settings_.bloodSplatterDensity = scareTier >= static_cast<int>(PlayableScareTier::Blood) ? std::max(settings_.bloodSplatterDensity, 0.70f) : 0.0f;
        settings_.bloodWorldFlicker = scareTier >= static_cast<int>(PlayableScareTier::Blood);
        settings_.bloodWorldCoverage = scareTier >= static_cast<int>(PlayableScareTier::Blood) ? std::max(settings_.bloodWorldCoverage, 0.75f) : 0.0f;
        settings_.fleshFlicker = scareTier >= static_cast<int>(PlayableScareTier::Flesh);
        settings_.fleshAlwaysOn = false;
        settings_.monsterIgnorePlayer = !spec.bossEncounter;
        if (spec.bossEncounter) {
            settings_.monsterSpeed = std::max(settings_.monsterSpeed, spec.levelInLayer >= 5 ? 0.78f : 0.62f);
            settings_.monsterSprintSpeed = std::max(settings_.monsterSprintSpeed, spec.levelInLayer >= 5 ? 1.02f : 0.86f);
            settings_.monsterVisibleDistance = std::max(settings_.monsterVisibleDistance, spec.levelInLayer >= 5 ? 15.0f : 10.0f);
        } else {
            monsterPath_.clear();
            monsterTrail_.clear();
            monsterLimbAnchors_.clear();
            monsterSmoothedBodyPoints_.clear();
            monsterSmoothedBodyUps_.clear();
            monsterBodySmoothTime_ = -1000.0f;
            monsterGoal_ = {-1000, -1000};
            monsterSoundTile_ = {-1000, -1000};
            monsterLastKnownTile_ = {-1000, -1000};
            monsterRoamTile_ = {-1000, -1000};
            monsterHasSound_ = false;
            monsterHasLastKnown_ = false;
            monsterChasingVisible_ = false;
            monsterRecognizedForChase_ = false;
            monsterHeadChaseBlend_ = 0.0f;
            monsterHeadLockAmount_ = 0.0f;
        }
    }

    std::wstring EncodeMazeBytes(const std::vector<uint8_t>& bytes) const {
        std::wstring out;
        out.reserve(bytes.size());
        for (uint8_t v : bytes) out.push_back(static_cast<wchar_t>(L'0' + std::clamp<int>(v, 0, 9)));
        return out;
    }

    void DecodeMazeBytes(const std::wstring& text, std::vector<uint8_t>& out, size_t expectedSize) const {
        out.assign(expectedSize, 0);
        size_t count = std::min(expectedSize, text.size());
        for (size_t i = 0; i < count; ++i) {
            wchar_t ch = text[i];
            out[i] = (ch >= L'0' && ch <= L'9') ? static_cast<uint8_t>(ch - L'0') : 0;
        }
    }

    static std::unordered_map<std::wstring, std::wstring> ParseSaveKeyValues(const std::wstring& text) {
        std::unordered_map<std::wstring, std::wstring> values;
        std::wistringstream in(text);
        std::wstring line;
        while (std::getline(in, line)) {
            if (!line.empty() && line.back() == L'\r') line.pop_back();
            size_t eq = line.find(L'=');
            if (eq == std::wstring::npos) continue;
            values[line.substr(0, eq)] = line.substr(eq + 1);
        }
        return values;
    }

    static int SaveInt(const std::unordered_map<std::wstring, std::wstring>& values, const wchar_t* key, int fallback) {
        auto it = values.find(key);
        if (it == values.end()) return fallback;
        wchar_t* end = nullptr;
        long parsed = std::wcstol(it->second.c_str(), &end, 10);
        return end != it->second.c_str() ? static_cast<int>(parsed) : fallback;
    }

    static float SaveFloat(const std::unordered_map<std::wstring, std::wstring>& values, const wchar_t* key, float fallback) {
        auto it = values.find(key);
        if (it == values.end()) return fallback;
        wchar_t* end = nullptr;
        float parsed = std::wcstof(it->second.c_str(), &end);
        return end != it->second.c_str() ? parsed : fallback;
    }

    static std::wstring SaveString(const std::unordered_map<std::wstring, std::wstring>& values, const wchar_t* key) {
        auto it = values.find(key);
        return it == values.end() ? std::wstring{} : it->second;
    }

    bool SaveCurrentRunToFile() {
        if (runtimeMode_ != RendererRuntimeMode::PlayableGame || !playableRun_.active || !playableRun_.levelRunning) return false;
        std::wostringstream out;
        const PlayableLevelSpec& spec = playableRun_.currentLevel;
        out << L"Version=1\n";
        out << L"Layer=" << playableRun_.layer << L"\n";
        out << L"Level=" << playableRun_.levelInLayer << L"\n";
        out << L"CompletedLevels=" << playableRun_.completedLevels << L"\n";
        out << L"DarkLayerOne=" << (playableRun_.darkLayerOne ? 1 : 0) << L"\n";
        out << L"SaveItemTarget=" << playableRun_.saveItemTarget << L"\n";
        out << L"SaveItemsSpawned=" << playableRun_.saveItemsSpawned << L"\n";
        out << L"LayerPagesCollected=" << playableRun_.layerPagesCollected << L"\n";
        for (size_t i = 0; i < playableRun_.levelPageTargets.size(); ++i) {
            out << L"LevelPageTarget" << i << L"=" << playableRun_.levelPageTargets[i] << L"\n";
        }
        for (size_t i = 0; i < playableRun_.layerPageCollected.size(); ++i) {
            out << L"LayerPage" << i << L"Collected=" << static_cast<int>(playableRun_.layerPageCollected[i]) << L"\n";
        }
        out << L"RunSeconds=" << playableRun_.runSeconds << L"\n";
        out << L"LevelSeconds=" << playableRun_.levelSeconds << L"\n";
        out << L"TotalScore=" << playableRun_.totalScore << L"\n";
        out << L"SpecMazeWidth=" << spec.mazeWidth << L"\n";
        out << L"SpecMazeHeight=" << spec.mazeHeight << L"\n";
        out << L"SpecBoss=" << (spec.bossEncounter ? 1 : 0) << L"\n";
        out << L"SpecBossChance=" << spec.bossEncounterChance << L"\n";
        out << L"SpecScareTier=" << static_cast<int>(spec.scareTier) << L"\n";
        out << L"MazeW=" << maze_.w << L"\n";
        out << L"MazeH=" << maze_.h << L"\n";
        out << L"MazeTileW=" << maze_.tileW << L"\n";
        out << L"MazeTileD=" << maze_.tileD << L"\n";
        out << L"StartX=" << maze_.start.x << L"\n";
        out << L"StartY=" << maze_.start.y << L"\n";
        out << L"ExitX=" << maze_.exit.x << L"\n";
        out << L"ExitY=" << maze_.exit.y << L"\n";
        out << L"Open=" << EncodeMazeBytes(maze_.open) << L"\n";
        out << L"WallFeatures=" << EncodeMazeBytes(maze_.wallFeatures) << L"\n";
        out << L"CameraX=" << camera_.x << L"\n";
        out << L"CameraY=" << camera_.y << L"\n";
        out << L"CameraZ=" << camera_.z << L"\n";
        out << L"Yaw=" << yaw_ << L"\n";
        out << L"BodyYaw=" << bodyYaw_ << L"\n";
        out << L"LookPitch=" << lookPitch_ << L"\n";
        out << L"PlayerHealth=" << playerHealth_ << L"\n";
        out << L"PlayerStamina=" << playerStamina_ << L"\n";
        out << L"CollectedPages=" << collectiblePagesCollected_ << L"\n";
        out << L"SavePointActive=" << (savePoint_.active ? 1 : 0) << L"\n";
        out << L"SavePointX=" << savePoint_.pos.x << L"\n";
        out << L"SavePointY=" << savePoint_.pos.y << L"\n";
        out << L"SavePointZ=" << savePoint_.pos.z << L"\n";
        out << L"SavePointYaw=" << savePoint_.yaw << L"\n";
        for (size_t i = 0; i < collectiblePages_.size(); ++i) {
            out << L"Page" << i << L"Collected=" << (collectiblePages_[i].collected ? 1 : 0) << L"\n";
        }
        return WriteTextFile(GameSavePath(), out.str());
    }

    bool LoadSavedRunFromFile() {
        std::wstring text = ReadTextFile(GameSavePath());
        if (text.empty()) return false;
        auto values = ParseSaveKeyValues(text);
        if (SaveInt(values, L"Version", 0) != 1) return false;

        playableRun_ = {};
        playableRun_.active = true;
        playableRun_.levelRunning = true;
        playableRun_.runFinished = false;
        playableRun_.layer = std::max(1, SaveInt(values, L"Layer", 1));
        playableRun_.levelInLayer = std::clamp(SaveInt(values, L"Level", 1), 1, PlayableRunState::kLevelsPerLayer);
        playableRun_.completedLevels = std::max(0, SaveInt(values, L"CompletedLevels", 0));
        playableRun_.darkLayerOne = SaveInt(values, L"DarkLayerOne", 0) != 0;
        playableRun_.saveItemTarget = std::clamp(SaveInt(values, L"SaveItemTarget", 1), 1, 3);
        int savedSaveItemsSpawned = std::clamp(SaveInt(values, L"SaveItemsSpawned", 0), 0, playableRun_.saveItemTarget);
        playableRun_.saveItemsSpawned = savedSaveItemsSpawned;
        for (size_t i = 0; i < playableRun_.levelPageTargets.size(); ++i) {
            std::wstring key = L"LevelPageTarget" + std::to_wstring(i);
            playableRun_.levelPageTargets[i] = std::clamp(SaveInt(values, key.c_str(), playableRun_.levelPageTargets[i]), 0, kCollectiblePageMaterialCount);
        }
        EnsureLayerPageDistribution();
        playableRun_.layerPagesCollected = std::clamp(SaveInt(values, L"LayerPagesCollected", 0), 0, kCollectiblePageMaterialCount);
        for (size_t i = 0; i < playableRun_.layerPageCollected.size(); ++i) {
            std::wstring key = L"LayerPage" + std::to_wstring(i) + L"Collected";
            playableRun_.layerPageCollected[i] = SaveInt(values, key.c_str(), 0) != 0 ? 1 : 0;
        }
        playableRun_.runSeconds = std::max(0.0f, SaveFloat(values, L"RunSeconds", 0.0f));
        playableRun_.levelSeconds = std::max(0.0f, SaveFloat(values, L"LevelSeconds", 0.0f));
        playableRun_.totalScore = std::max(0, SaveInt(values, L"TotalScore", 0));
        PlayableLevelSpec spec{};
        spec.layer = playableRun_.layer;
        spec.levelInLayer = playableRun_.levelInLayer;
        spec.mazeWidth = std::clamp(SaveInt(values, L"SpecMazeWidth", 25), 3, 151);
        spec.mazeHeight = std::clamp(SaveInt(values, L"SpecMazeHeight", 25), 3, 151);
        spec.bossEncounter = SaveInt(values, L"SpecBoss", 0) != 0;
        spec.bossEncounterChance = std::clamp(SaveFloat(values, L"SpecBossChance", 0.0f), 0.0f, 1.0f);
        spec.scareTier = static_cast<PlayableScareTier>(std::clamp(SaveInt(values, L"SpecScareTier", 0), 0, 4));
        playableRun_.currentLevel = spec;
        ApplyPlayableLevelSpec(spec);

        maze_.w = std::clamp(SaveInt(values, L"MazeW", spec.mazeWidth), 3, 151);
        maze_.h = std::clamp(SaveInt(values, L"MazeH", spec.mazeHeight), 3, 151);
        maze_.tileW = std::clamp(SaveFloat(values, L"MazeTileW", settings_.tileWidthMeters), 1.2f, 8.0f);
        maze_.tileD = std::clamp(SaveFloat(values, L"MazeTileD", settings_.tileLengthMeters), 1.2f, 8.0f);
        maze_.start = {std::clamp(SaveInt(values, L"StartX", 1), 0, maze_.w - 1), std::clamp(SaveInt(values, L"StartY", 1), 0, maze_.h - 1)};
        maze_.exit = {std::clamp(SaveInt(values, L"ExitX", maze_.w - 2), 0, maze_.w - 1), std::clamp(SaveInt(values, L"ExitY", maze_.h - 2), 0, maze_.h - 1)};
        const size_t cellCount = static_cast<size_t>(maze_.w * maze_.h);
        DecodeMazeBytes(SaveString(values, L"Open"), maze_.open, cellCount);
        DecodeMazeBytes(SaveString(values, L"WallFeatures"), maze_.wallFeatures, cellCount);
        if (!maze_.IsOpen(maze_.start.x, maze_.start.y)) maze_.SetOpen(maze_.start.x, maze_.start.y);
        if (!maze_.IsOpen(maze_.exit.x, maze_.exit.y)) maze_.SetOpen(maze_.exit.x, maze_.exit.y);

        CreateMazeMaskTexture();
        ResetSimulation();
        playableRun_.saveItemsSpawned = savedSaveItemsSpawned;
        CreateMazeMesh();
        SetupPersistentAudioEmitters();

        camera_ = {
            SaveFloat(values, L"CameraX", maze_.WorldCenter(maze_.start, 1.45f).x),
            SaveFloat(values, L"CameraY", 1.45f),
            SaveFloat(values, L"CameraZ", maze_.WorldCenter(maze_.start, 1.45f).z)
        };
        yaw_ = SaveFloat(values, L"Yaw", 0.0f);
        bodyYaw_ = SaveFloat(values, L"BodyYaw", yaw_);
        lookPitch_ = SaveFloat(values, L"LookPitch", -0.055f);
        playerHealth_ = std::clamp(SaveFloat(values, L"PlayerHealth", 100.0f), 1.0f, 100.0f);
        playerStamina_ = std::clamp(SaveFloat(values, L"PlayerStamina", 100.0f), 0.0f, 100.0f);
        collectiblePagesCollected_ = std::clamp(SaveInt(values, L"CollectedPages", 0), 0, kCollectiblePageMaterialCount);
        savePoint_.active = SaveInt(values, L"SavePointActive", savePoint_.active ? 1 : 0) != 0;
        savePoint_.pos = {
            SaveFloat(values, L"SavePointX", savePoint_.pos.x),
            SaveFloat(values, L"SavePointY", savePoint_.pos.y),
            SaveFloat(values, L"SavePointZ", savePoint_.pos.z)
        };
        savePoint_.yaw = SaveFloat(values, L"SavePointYaw", savePoint_.yaw);
        for (size_t i = 0; i < collectiblePages_.size(); ++i) {
            std::wstring key = L"Page" + std::to_wstring(i) + L"Collected";
            collectiblePages_[i].collected = SaveInt(values, key.c_str(), collectiblePages_[i].collected ? 1 : 0) != 0;
            if (collectiblePages_[i].pageIndex >= 0 &&
                collectiblePages_[i].pageIndex < kCollectiblePageMaterialCount &&
                collectiblePages_[i].collected) {
                playableRun_.layerPageCollected[static_cast<size_t>(collectiblePages_[i].pageIndex)] = 1;
            }
        }
        int countedLayerPages = 0;
        for (uint8_t collected : playableRun_.layerPageCollected) {
            if (collected) ++countedLayerPages;
        }
        playableRun_.layerPagesCollected = std::max(playableRun_.layerPagesCollected, countedLayerPages);
        ShowGameNotification(L"Saved run loaded", 3.8f);
        return true;
    }

    int ScoreCompletedPlayableLevel(float levelSeconds, bool bossEncounter) const {
        int base = 1000 + playableRun_.currentLevel.levelInLayer * 250;
        int speedBonus = std::max(0, 900 - static_cast<int>(std::round(levelSeconds * 6.0f)));
        int healthBonus = static_cast<int>(std::round(std::clamp(playerHealth_, 0.0f, 100.0f) * 3.0f));
        int pageBonus = collectiblePagesCollected_ * 150;
        int bossBonus = bossEncounter ? 750 : 0;
        return std::max(0, base + speedBonus + healthBonus + pageBonus + bossBonus);
    }

    std::wstring FormatRunSeconds(float seconds) const {
        int whole = std::max(0, static_cast<int>(std::round(seconds)));
        int minutes = whole / 60;
        int secs = whole % 60;
        std::wostringstream s;
        s << minutes << L":";
        if (secs < 10) s << L"0";
        s << secs;
        return s.str();
    }

    void BeginPlayableLevel(int levelInLayer, bool showStartNotification = true) {
        playableRun_.levelInLayer = std::clamp(levelInLayer, 1, PlayableRunState::kLevelsPerLayer);
        playableRun_.currentLevel = BuildLayerOneLevelSpec(playableRun_.levelInLayer);
        playableRun_.levelSeconds = 0.0f;
        playableRun_.levelRunning = true;
        playableRun_.scoreScreenActive = false;
        playableRun_.scoreScreenFinal = false;
        ApplyPlayableLevelSpec(playableRun_.currentLevel);
        maze_.w = settings_.mazeWidth;
        maze_.h = settings_.mazeHeight;
        maze_.tileW = settings_.tileWidthMeters;
        maze_.tileD = settings_.tileLengthMeters;
        maze_.exit = {maze_.w - 2, maze_.h - 2};
        RestartMaze();

        if (showStartNotification) {
            std::wostringstream notice;
            notice << L"Layer " << playableRun_.layer << L" - Level " << playableRun_.levelInLayer;
            if (playableRun_.currentLevel.bossEncounter) notice << L"  Encounter active";
            ShowGameNotification(notice.str(), 3.6f);
        }
    }

    void BeginPlayableRun() {
        playableRun_ = {};
        playableRun_.active = true;
        playableRun_.levelRunning = false;
        playableRun_.runFinished = false;
        playableRun_.layer = 1;
        playableRun_.levelInLayer = 1;
        playableRun_.darkLayerOne = menuDarkLayerOneRun_;
        playableRun_.saveItemTarget = PickLayerSaveItemTarget();
        GenerateLayerPageDistribution();
        playableRun_.completed.reserve(PlayableRunState::kLevelsPerLayer);
        monsterKillCount_ = 0;
        BeginPlayableLevel(1);
    }

    void CompletePlayableLevel() {
        if (!playableRun_.active || !playableRun_.levelRunning) {
            exitTransitionActive_ = false;
            return;
        }

        playableRun_.levelRunning = false;
        DeleteSavedRun();
        PlayableLevelResult result{};
        result.layer = playableRun_.layer;
        result.levelInLayer = playableRun_.levelInLayer;
        result.levelSeconds = playableRun_.levelSeconds;
        result.runSeconds = playableRun_.runSeconds;
        result.bossEncounter = playableRun_.currentLevel.bossEncounter;
        result.score = ScoreCompletedPlayableLevel(result.levelSeconds, result.bossEncounter);
        playableRun_.totalScore += result.score;
        playableRun_.lastResult = result;
        playableRun_.completed.push_back(result);
        playableRun_.completedLevels = static_cast<int>(playableRun_.completed.size());
        playableRun_.scoreScreenActive = true;

        if (playableRun_.levelInLayer >= PlayableRunState::kLevelsPerLayer) {
            playableRun_.runFinished = true;
            playableRun_.active = false;
            playableRun_.scoreScreenFinal = true;
            exitTransitionActive_ = false;
            settings_.monsterIgnorePlayer = true;
            path_.clear();
            pathIndex_ = 0;
            std::wostringstream notice;
            notice << L"Layer complete\nTime " << FormatRunSeconds(playableRun_.runSeconds)
                   << L"   Score " << playableRun_.totalScore
                   << L"\nPress Esc for menu";
            ShowGameNotification(notice.str(), 3600.0f);
            return;
        }

        std::wostringstream notice;
        notice << L"Level " << result.levelInLayer << L" clear\n"
               << L"Time " << FormatRunSeconds(result.levelSeconds) << L"   Score +" << result.score
               << L"   Total " << playableRun_.totalScore
               << L"\nPress Interact to continue";
        ShowGameNotification(notice.str(), 3600.0f);
        exitTransitionActive_ = false;
    }

    void ContinueAfterScoreScreen() {
        if (!playableRun_.scoreScreenActive || playableRun_.scoreScreenFinal) return;
        int nextLevel = std::clamp(playableRun_.levelInLayer + 1, 1, PlayableRunState::kLevelsPerLayer);
        BeginPlayableLevel(nextLevel, true);
    }

    void UpdatePlayableProgressionTimers(float dt) {
        if (runtimeMode_ != RendererRuntimeMode::PlayableGame || !playableRun_.active || !playableRun_.levelRunning) return;
        if (deathActive_ || exitTransitionActive_) return;
        float step = std::max(0.0f, dt);
        playableRun_.runSeconds += step;
        playableRun_.levelSeconds += step;
    }
