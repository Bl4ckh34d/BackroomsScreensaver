        int maxTriangles = std::max(0, settingsRuntime_.live.monsterSkullMaxTriangles);
        if (maxTriangles <= 0) return false;

        std::ifstream in(path);
        if (!in) return false;
