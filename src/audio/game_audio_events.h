#pragma once

enum class GameAudioEventKind {
    PlayOneShot,
    PlayerNoise
};

enum class GameAudioEventCategory {
    Generic,
    Footstep,
    Door,
    Lamp,
    Vent,
    MonsterVocal,
    WetDrip,
    Scare,
    PlayerInteraction,
    Collectible
};

struct PlayerAudibleSoundPulse {
    XMFLOAT3 pos{};
    float radius = 0.0f;
    float age = 0.0f;
    float life = 0.90f;
    bool processedByMonster = false;
    bool heardByMonster = false;
};

struct GameAudioEvent {
    GameAudioEventKind kind = GameAudioEventKind::PlayOneShot;
    GameAudioEventCategory category = GameAudioEventCategory::Generic;
    GameSound sound = GameSound::CarpetStep;
    AudioBus bus = AudioBus::Effects;
    XMFLOAT3 pos{};
    float volume = 1.0f;
    bool spatial = true;
    bool useOcclusion = false;
    float occlusionLimit = 1000000.0f;
    float hearingRadius = 0.0f;
    float hearingLife = 0.90f;

    GameAudioEvent WithCategory(GameAudioEventCategory value) const;

    static GameAudioEvent OneShot(
        GameSound sound,
        AudioBus bus,
        const XMFLOAT3& pos,
        float volume,
        bool spatial,
        bool useOcclusion = false,
        float occlusionLimit = 1000000.0f);

    static GameAudioEvent OneShotWithPlayerNoise(
        GameSound sound,
        AudioBus bus,
        const XMFLOAT3& pos,
        float volume,
        bool spatial,
        float hearingRadius,
        float hearingLife,
        bool useOcclusion = false,
        float occlusionLimit = 1000000.0f);

    static GameAudioEvent PlayerNoise(
        const XMFLOAT3& pos,
        float hearingRadius,
        float hearingLife,
        GameAudioEventCategory category = GameAudioEventCategory::Generic);
};
