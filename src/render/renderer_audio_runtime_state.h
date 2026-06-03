#pragma once

constexpr int kTitleThemeAudioTag = 7100;
constexpr int kVisionFlashAudioTag = 7101;

struct RendererAudioRuntimeState {
    AudioEngine engine;
    bool ready = false;
    bool samplesLoaded = false;
    bool titleThemeStarted = false;
    AudioRuntimeState game{};
};
