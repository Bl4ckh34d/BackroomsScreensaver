    void ResetMonsterPresentationState(bool clearTrail, bool clearHandprints, bool resetHead = true) {
        if (clearTrail) monsterPresentation_.trail.clear();
        monsterPresentation_.limbAnchors.clear();
        monsterPresentation_.smoothedBodyPoints.clear();
        monsterPresentation_.smoothedBodyUps.clear();
        monsterPresentation_.bodySmoothTime = -1000.0f;
        monsterPresentation_.renderVisibleUntil = -1000.0f;
        if (clearHandprints) monsterPresentation_.handprints.clear();
        if (resetHead) {
            monsterPresentation_.headChaseBlend = 0.0f;
            monsterPresentation_.headLockAmount = 0.0f;
        }
    }
