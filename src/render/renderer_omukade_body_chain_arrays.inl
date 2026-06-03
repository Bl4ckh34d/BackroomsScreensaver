        constexpr int maxBodyCount = 48;
        std::array<XMFLOAT3, maxBodyCount> bodyPoints{};
        std::array<XMFLOAT3, maxBodyCount> bodyCenterlinePoints{};
        std::array<float, maxBodyCount> bodyRadii{};
        std::array<XMFLOAT3, maxBodyCount> bodySides{};
        std::array<XMFLOAT3, maxBodyCount> bodyUps{};
        std::array<XMFLOAT3, maxBodyCount> bodyTangents{};
        std::array<float, maxBodyCount> bodyUvV{};
        std::array<float, maxBodyCount> bodyUvShift{};
        float bodySpacing = maze.TileMinimum() * (debugEffectMonster ? 0.21f : (monsterDetail >= 2 ? 0.19f : (monsterDetail == 1 ? 0.24f : 0.32f)));
        float visualBodySpacing = maze.TileMinimum() * (debugEffectMonster ? 0.19f : (monsterDetail >= 2 ? 0.17f : (monsterDetail == 1 ? 0.22f : 0.29f)));
        int bodyCount = debugEffectMonster
            ? 22
            : std::clamp(static_cast<int>(std::ceil(bodyLengthMeters / std::max(0.12f, bodySpacing))) + 1,
                monsterDetail >= 2 ? 18 : (monsterDetail == 1 ? 13 : 8),
                monsterDetail >= 2 ? 34 : (monsterDetail == 1 ? 22 : 14));
        float curiosityPose = MonsterCuriosityAmount();
        XMFLOAT3 monsterForward{std::sin(monsterYaw), 0.0f, std::cos(monsterYaw)};
        XMFLOAT3 monsterRight{std::cos(monsterYaw), 0.0f, -std::sin(monsterYaw)};
