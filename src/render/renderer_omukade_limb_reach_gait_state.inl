        int limbPairs = debugEffectMonster ? 8 : (monsterDetail >= 2 ? 10 : (monsterDetail == 1 ? 7 : 4));
        int requiredLimbAnchors = std::max(0, limbPairs * 2);
        if (monsterPresentation_.limbAnchors.size() < static_cast<size_t>(requiredLimbAnchors)) {
            monsterPresentation_.limbAnchors.resize(static_cast<size_t>(requiredLimbAnchors));
        }
        float gaitDrive = Clamp01(SmoothStep(0.0f, 1.0f, Clamp01(world.monsterRoamBurstTimer / 1.05f)) * (1.0f - monsterPresentation_.headChaseBlend * 0.65f) +
            SmoothStep(0.18f, 1.0f, monsterPresentation_.headChaseBlend) * 0.64f);
        float gaitStepInterval = Lerp(0.34f, 0.16f, gaitDrive);
        int gaitTick = requiredLimbAnchors > 0
            ? static_cast<int>(std::floor((timeRuntime_.time + static_cast<float>(sessionRuntime_.runtimeSeed & 31) * 0.013f) / std::max(0.035f, gaitStepInterval)))
            : 0;
        int gaitPair = limbPairs > 0 ? gaitTick % limbPairs : 0;
        int gaitWave = limbPairs > 0 ? gaitTick / limbPairs : 0;
        bool gaitRightSide = ((gaitPair + gaitWave) & 1) != 0;
        int gaitSlot = limbPairs > 0 ? gaitPair * 2 + (gaitRightSide ? 1 : 0) : 0;
        XMFLOAT3 trailMotion{0.0f, 0.0f, 0.0f};
        if (monsterPresentation_.trail.size() >= 4) {
            trailMotion = Sub3(monsterPresentation_.trail.front(), monsterPresentation_.trail[std::min<size_t>(3, monsterPresentation_.trail.size() - 1)]);
        }
        float trailMotionLen = Length3(trailMotion);
        XMFLOAT3 trailMotionDir = Normalize3(trailMotion, forward);
        float trailMotionAmount = Clamp01(trailMotionLen / std::max(0.05f, maze.TileMinimum() * 0.22f));
