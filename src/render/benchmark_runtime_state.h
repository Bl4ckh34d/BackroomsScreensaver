#pragma once

struct BenchmarkRuntimeState {
    bool active = false;
    float timer = 0.0f;
    std::vector<Tile> flightPath;
    bool autoplayActive = false;
    float autoplayTimer = 0.0f;
    bool autoplayClosePosted = false;
    double previousProfileFrameStartMs = 0.0;
};
