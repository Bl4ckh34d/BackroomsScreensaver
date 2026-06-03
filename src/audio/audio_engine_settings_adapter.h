#pragma once

#include "audio_engine.h"
#include "../config/settings.h"

#include <filesystem>

// Renderer-side adapter from full game settings to audio engine inputs.

inline AudioEngineSettings MakeAudioEngineSettings(const Settings& settings) {
    AudioEngineSettings audio{};
    audio.audioMuted = settings.audioMuted;
    audio.audioMasterVolume = settings.audioMasterVolume;
    audio.audioMusicVolume = settings.audioMusicVolume;
    audio.audioEffectsVolume = settings.audioEffectsVolume;
    audio.audioAmbienceVolume = settings.audioAmbienceVolume;
    audio.audioMonsterVolume = settings.audioMonsterVolume;
    return audio;
}

inline AudioEngineAssets MakeAudioEngineAssets(const Settings& settings) {
    AudioEngineAssets assets{};
    assets.moduleDirectory = ModuleDirectory();
    assets.currentDirectory = std::filesystem::current_path();
    assets.configuredAssetFolder = settings.assetFolder;
    return assets;
}
