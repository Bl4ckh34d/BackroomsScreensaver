        XMFLOAT3 toMonster = Sub3(monsterPosition, playerPosition);
        float planarMonsterDist = std::sqrt(toMonster.x * toMonster.x + toMonster.z * toMonster.z);
        XMFLOAT3 cameraForward = Forward();
        float forwardDot = planarMonsterDist > 0.001f
            ? (toMonster.x * cameraForward.x + toMonster.z * cameraForward.z) / planarMonsterDist
            : 1.0f;
        bool monsterInFront = forwardDot > -0.10f;
        bool canTrackPlayer = false;
        bool specialMonsterView = monsterPreview_.active || debugEffectMonster || world.deathActive;
        bool monsterTileVisible = specialMonsterView;
        bool monsterAnyPartVisible = specialMonsterView;
        bool monsterViewRelevant = specialMonsterView;
        bool monsterOccluded = false;
