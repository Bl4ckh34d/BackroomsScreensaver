        if (!monsterViewRelevant) {
            if (!specialMonsterView && timeRuntime_.time > monsterPresentation_.renderVisibleUntil + 0.18f) {
                monsterPresentation_.bodySmoothTime = -1000.0f;
                monsterPresentation_.smoothedBodyPoints.clear();
                monsterPresentation_.smoothedBodyUps.clear();
                monsterPresentation_.limbAnchors.clear();
            }
            return;
        }
