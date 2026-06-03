#pragma once

#include <windows.h>
#include <xaudio2.h>
#include <x3daudio.h>
#include <wrl/client.h>
#include <DirectXMath.h>

#include <algorithm>
#include <array>
#include <cmath>
#include <cstddef>
#include <cstdint>
#include <cstring>
#include <filesystem>
#include <fstream>
#include <random>
#include <string>
#include <vector>

using DirectX::XMFLOAT3;

struct AudioEngineSettings {
    bool audioMuted = false;
    float audioMasterVolume = 1.0f;
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
    Monster
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
    PaperFlutter
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

class AudioEngine {
public:
    bool Initialize(const AudioEngineSettings& settings);
    void Shutdown();
    void ApplySettings(const AudioEngineSettings& settings);
    void LoadAll(const AudioEngineAssets& assets);
    void SetListener(XMFLOAT3 pos, XMFLOAT3 forward);

    template <typename OcclusionFn>
    void Update(float dt, OcclusionFn occlusionForPosition) {
        if (!initialized_) return;
        dt = std::max(0.0f, dt);
        for (size_t i = 0; i < voices_.size();) {
            AudioVoiceInstance& instance = voices_[i];
            XAUDIO2_VOICE_STATE state{};
            instance.voice->GetState(&state);
            if (!instance.loop && state.BuffersQueued == 0) {
                instance.voice->DestroyVoice();
                voices_.erase(voices_.begin() + static_cast<std::ptrdiff_t>(i));
                continue;
            }
            float distanceGain = SpatialDistanceGain(instance);
            if (instance.spatial && distanceGain <= 0.0001f) {
                instance.voice->SetVolume(0.0f);
                instance.occlusionRefresh = std::min(instance.occlusionRefresh, 0.10f);
                ++i;
                continue;
            }
            instance.occlusionRefresh -= dt;
            if (instance.occlusionRefresh <= 0.0f) {
                instance.occlusion = std::clamp(occlusionForPosition(instance.pos), 0.0f, 8.0f);
                instance.occlusionRefresh = instance.bus == AudioBus::Ambience ? 0.32f : 0.14f;
            }
            Apply3D(instance);
            ++i;
        }
    }

    template <typename OcclusionFn>
    void Update(OcclusionFn occlusionForPosition) {
        Update(0.0f, occlusionForPosition);
    }

    void Update() {
        Update(0.0f, [](XMFLOAT3) { return 0.0f; });
    }

    void StopAll();
    void PlayRandom(GameSound sound, AudioBus bus, XMFLOAT3 pos, float volume = 1.0f, bool spatial = true,
                    float initialOcclusion = 0.0f);
    void StartLoop(GameSound sound, AudioBus bus, XMFLOAT3 pos, float volume = 1.0f, bool spatial = true,
                   float initialOcclusion = 0.0f);
    void StartLoopTagged(GameSound sound, AudioBus bus, XMFLOAT3 pos, float volume, bool spatial, int tag,
                         float initialOcclusion = 0.0f);
    void StartLoopTaggedSample(GameSound sound, size_t sampleIndex, AudioBus bus, XMFLOAT3 pos, float volume,
                               bool spatial, int tag, float initialOcclusion = 0.0f);
    bool HasTaggedVoice(int tag) const;
    void StopTaggedVoice(int tag);
    size_t PickRandomSample(GameSound sound);
    size_t PickStableSample(GameSound sound, uint32_t stableId) const;
    size_t PickRandomSampleExcept(GameSound sound, size_t excludedSampleIndex);
    void PlaySample(size_t sampleIndex, AudioBus bus, XMFLOAT3 pos, float volume = 1.0f, bool spatial = true,
                    float frequencyRatio = 1.0f, GameSound sound = GameSound::MonsterGrowl,
                    float initialOcclusion = 0.0f, AudioToneProfile toneProfile = AudioToneProfile::Normal);

private:
    static constexpr size_t kInvalidSample = static_cast<size_t>(-1);
    static constexpr uint32_t FourCC(char a, char b, char c, char d) {
        return static_cast<uint32_t>(static_cast<uint8_t>(a)) |
            (static_cast<uint32_t>(static_cast<uint8_t>(b)) << 8) |
            (static_cast<uint32_t>(static_cast<uint8_t>(c)) << 16) |
            (static_cast<uint32_t>(static_cast<uint8_t>(d)) << 24);
    }

    std::filesystem::path ResolveSoundFolder(const AudioEngineAssets& assets, const std::wstring& folder) const;
    void AddFolder(const AudioEngineAssets& assets, GameSound sound, const std::wstring& folder,
                   const std::wstring& pattern = L"*.wav");
    bool LoadWav(const std::filesystem::path& path, AudioSample& out) const;
    size_t PickSample(GameSound sound);
    float BusVolume(AudioBus bus) const;
    void StartVoice(GameSound sound, size_t sampleIndex, AudioBus bus, XMFLOAT3 pos, float volume, bool loop, bool spatial,
                    float frequencyRatio = 1.0f, int tag = -1, float initialOcclusion = 0.0f,
                    AudioToneProfile toneProfile = AudioToneProfile::Normal);
    void Apply3D(AudioVoiceInstance& instance);
    float SpatialDistanceGain(const AudioVoiceInstance& instance) const;
    float OcclusionGain(AudioBus bus, float occlusion) const;

    bool initialized_ = false;
    bool comInitialized_ = false;
    HRESULT comHr_ = E_FAIL;
    Microsoft::WRL::ComPtr<IXAudio2> xaudio_;
    IXAudio2MasteringVoice* masterVoice_ = nullptr;
    UINT32 outputChannels_ = 2;
    X3DAUDIO_HANDLE x3dHandle_{};
    X3DAUDIO_LISTENER listener_{};
    std::vector<AudioSample> samples_;
    std::vector<std::vector<size_t>> groups_;
    std::vector<AudioVoiceInstance> voices_;
    std::mt19937 rng_{0xA7710u};
    bool muted_ = false;
    float masterVolume_ = 1.0f;
    float effectsVolume_ = 1.0f;
    float ambienceVolume_ = 1.0f;
    float monsterVolume_ = 1.0f;
};
