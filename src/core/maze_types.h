#pragma once

constexpr int kMazeW = 25;
constexpr int kMazeH = 25;
constexpr float kTile = 1.6f;

struct Tile {
    int x = 0;
    int y = 0;
};

inline bool operator==(Tile a, Tile b) {
    return a.x == b.x && a.y == b.y;
}

inline bool operator!=(Tile a, Tile b) {
    return !(a == b);
}
