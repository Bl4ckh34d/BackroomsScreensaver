#pragma once

#include "audio_engine_dependencies.h"

struct AudioEngineSettings {
    bool audioMuted = false;
    float audioMasterVolume = 1.0f;
    float audioMusicVolume = 1.0f;
    float audioEffectsVolume = 1.0f;
    float audioAmbienceVolume = 1.0f;
    float audioMonsterVolume = 1.0f;
};

struct AudioEngineAssets {
    std::filesystem::path moduleDirectory;
    std::filesystem::path currentDirectory;
    std::wstring configuredAssetFolder;
};

enum class AudioBus {
    Effects,
    Ambience,
    Monster,
    Music
};

enum class GameSound {
    CarpetStep,
    SoakedCarpetStep,
    NeonHumQuiet,
    NeonHumLoud,
    NeonHumLoud2,
    MonsterGrowl,
    MonsterSpottedScream,
    ElectricCrackle,
    NeonFlickerStarterClick,
    AirVentDustPuff,
    WetCarpetCeilingDrip,
    DoorOpenCreak,
    DoorCloseCreak,
    DoorCloseLock,
    LightBulbBreak,
    VisionFlash,
    FlashlightStutter,
    PaperFlutter,
    TitleTheme
};

enum class AudioToneProfile {
    Normal,
    MetallicVent
};

struct AudioSample {
    WAVEFORMATEX format{};
    std::vector<uint8_t> formatExtra;
    std::vector<uint8_t> data;
};

struct AudioVoiceInstance {
    IXAudio2SourceVoice* voice = nullptr;
    size_t sampleIndex = 0;
    AudioBus bus = AudioBus::Effects;
    GameSound sound = GameSound::CarpetStep;
    XMFLOAT3 pos{};
    float baseVolume = 1.0f;
    float frequencyRatio = 1.0f;
    float occlusion = 0.0f;
    float occlusionRefresh = 0.0f;
    AudioToneProfile toneProfile = AudioToneProfile::Normal;
    int tag = -1;
    bool loop = false;
    bool spatial = true;
};
