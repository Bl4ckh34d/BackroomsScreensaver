    static int PickSaveItemTarget(std::mt19937& rng) {
        return 1 + static_cast<int>(rng() % 3u);
    }

    static float RandRange(float a, float b, std::mt19937& rng) {
        std::uniform_real_distribution<float> dist(a, b);
        return dist(rng);
    }

    PlayableLevelSpec BuildLayerOneLevelSpec(int targetLevelInLayer, std::mt19937& rng) const {
        PlayableLevelSpec spec{};
        spec.layer = layer;
        spec.levelInLayer = std::clamp(targetLevelInLayer, 1, kLevelsPerLayer);

        switch (spec.levelInLayer) {
        case 1:
            spec.mazeWidth = 15;
            spec.mazeHeight = 15;
            spec.scareTier = PlayableScareTier::None;
            spec.bossEncounterChance = 0.0f;
            break;
        case 2:
            if ((rng() & 1u) == 0) {
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
            if ((rng() & 1u) == 0) {
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
                Tile size = choices[static_cast<size_t>(rng() % choices.size())];
                spec.mazeWidth = size.x;
                spec.mazeHeight = size.y;
            }
            spec.scareTier = PlayableScareTier::Flesh;
            spec.bossEncounter = true;
            spec.bossEncounterChance = 1.0f;
            break;
        }

        if (!spec.bossEncounter && spec.bossEncounterChance > 0.0f) {
            float roll = static_cast<float>(rng() & 0xffffu) / 65535.0f;
            spec.bossEncounter = roll < spec.bossEncounterChance;
        }
        return spec;
    }

    float MapDirtProgression() const {
        if (!active) return 0.48f;
        if (customGame) {
            return Clamp01(static_cast<float>(customSpec.mapDirtPercent) / 100.0f);
        }

        float levelProgress = Clamp01((static_cast<float>(levelInLayer) - 1.0f) /
            static_cast<float>(std::max(1, kLevelsPerLayer - 1)));
        float layerProgress = Clamp01(static_cast<float>(std::max(0, layer - 1)) * 0.18f);
        return Clamp01(levelProgress + layerProgress);
    }

    float AirParticleDensityScale() const {
        if (!active) return 1.0f;
        int clampedLevel = std::clamp(levelInLayer, 1, kLevelsPerLayer);
        constexpr float kLevelScale[kLevelsPerLayer] = {
            0.16f, 0.34f, 0.56f, 0.78f, 1.0f
        };
        return kLevelScale[static_cast<size_t>(clampedLevel - 1)];
    }

    PlayableCustomScareGate CustomScareGateFor(int index) const {
        PlayableCustomScareGate gate{};
        if (!customGame) return gate;

        index = std::clamp(index, 0, CustomGameSpec::kScareTypeCount - 1);
        size_t slot = static_cast<size_t>(index);
        if (levelSeconds < customScareStartDelayByTypeSeconds[slot]) {
            gate.allowed = false;
            gate.requiresRoll = false;
            gate.chance = 0.0f;
            return gate;
        }

        gate.allowed = true;
        gate.requiresRoll = true;
        gate.chance = Clamp01(static_cast<float>(
            std::clamp(customSpec.scareChancePercent[slot], 0, 100)) / 100.0f);
        return gate;
    }
