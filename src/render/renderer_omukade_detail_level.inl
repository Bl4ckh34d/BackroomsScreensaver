        bool highDetailMonster = monsterPreview_.active || world.deathActive || canTrackPlayer ||
            monsterPresentation_.headChaseBlend > 0.62f || (monsterAnyPartVisible && dist < tileScale * 4.2f);
        bool mediumDetailMonster = highDetailMonster || (!monsterOccluded && (world.monsterHasLastKnownTarget || world.monsterHasSoundTarget)) ||
            monsterPresentation_.headChaseBlend > 0.18f || (monsterInFront && dist < tileScale * 8.5f) ||
            monsterAnyPartVisible || monsterViewRelevant;
        int monsterDetail = debugEffectMonster ? 1 : (highDetailMonster ? 2 : (mediumDetailMonster ? 1 : 0));
        if (IsPlayableSimulationMode(sessionRuntime_.mode) && !specialMonsterView && !monsterAnyPartVisible) {
            monsterDetail = std::min(monsterDetail, 1);
        }
        float faceYaw = monsterYaw;
        if (canTrackPlayer) {
            float cameraYaw = std::atan2(playerPosition.x - monsterPosition.x, playerPosition.z - monsterPosition.z);
            float turnIn = 0.035f + SmoothStep(0.0f, 1.0f, monsterPresentation_.headLockAmount) * 0.18f;
            faceYaw += AngleWrap(cameraYaw - faceYaw) * turnIn;
        }
