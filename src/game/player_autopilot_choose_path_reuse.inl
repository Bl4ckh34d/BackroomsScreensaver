        bool monsterActive = MonsterActiveForCurrentMode();
        Tile monsterTile = monsterActive ? MonsterTile() : Tile{-10000, -10000};
        if (!force && cameraRuntime_.pathIndex < cameraRuntime_.path.size()) {
            bool panicState = monsterActive && ChasePanicActive();
            bool freeRunPath = panicState && OpenAreaAllowsFreeRun(cur);
            bool panicContext = monsterActive && (IsThreatVisible() || panicState);
            if (ActivePathValidForMode(cur, freeRunPath) &&
                (!panicContext || !ActiveThreatPathShouldRepath(cur, monsterTile))) {
                return;
            }
            cameraRuntime_.path.clear();
            cameraRuntime_.pathIndex = 0;
        }
        if (VisibleInFront(maze.exit)) viewRuntime_.exitSpotted = true;
        cameraRuntime_.path.clear();
        cameraRuntime_.pathIndex = 0;
