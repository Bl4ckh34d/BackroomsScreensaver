#pragma once

#include "maze.h"
#include "../config/settings.h"

// Adapts the broad app settings model into maze-only generation inputs.

inline MazeGenerationSpec MakeMazeGenerationSpec(const Settings& settings) {
    MazeGenerationSpec spec{};
    spec.roomCount = settings.roomCount;
    spec.roomMinRadius = settings.roomMinRadius;
    spec.roomMaxRadius = settings.roomMaxRadius;
    spec.extraConnectorMinRatio = settings.extraConnectorMinRatio;
    spec.extraConnectorMaxRatio = settings.extraConnectorMaxRatio;
    spec.wallFeatureFrequency = settings.wallFeatureFrequency;
    spec.wallFeatureFrequencySpread = settings.wallFeatureFrequencySpread;
    return spec;
}

inline MazeLayoutSpec MakeMazeLayoutSpec(const Settings& settings) {
    MazeLayoutSpec spec{};
    spec.width = settings.mazeWidth;
    spec.height = settings.mazeHeight;
    spec.tileW = settings.tileWidthMeters;
    spec.tileD = settings.tileLengthMeters;
    return spec;
}
