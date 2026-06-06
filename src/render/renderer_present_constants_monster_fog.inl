        float monsterFogRadius = std::max(tileAverage * 2.60f, 5.80f);
        float monsterFogStrength = (!MonsterActiveForCurrentMode() || monsterPreview_.active || gEffectDebugViewer || gBloodDebugEveryWall || settingsRuntime_.live.bloodStudyView)
            ? 0.0f
            : 0.72f;
        cb.monsterFog0 = {
            world.monsterPosition.x,
            world.monsterPosition.z,
            monsterFogRadius,
            monsterFogStrength
        };
