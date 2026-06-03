#pragma once

struct PlaybackMonitorRect {
    RECT rc{};
    bool primary = false;
};

std::vector<PlaybackMonitorRect> EnumeratePlaybackMonitors();
