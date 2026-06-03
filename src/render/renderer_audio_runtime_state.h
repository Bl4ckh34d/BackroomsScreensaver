#pragma once

struct RendererAudioRuntimeState {
    AudioEngine engine;
    bool ready = false;
    bool samplesLoaded = false;
    AudioRuntimeState game{};
};
