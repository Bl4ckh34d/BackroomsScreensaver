#pragma once

struct MapOverlayRuntimeState {
    std::vector<OverlayVertex> cachedVerts;
    float nextUpdateTime = 0.0f;
    LONG cacheWidth = 0;
    LONG cacheHeight = 0;
    int cacheMazeW = 0;
    int cacheMazeH = 0;
    GameSessionMapOverlayStyle cacheStyle = GameSessionMapOverlayStyle::AiDebug;
};
