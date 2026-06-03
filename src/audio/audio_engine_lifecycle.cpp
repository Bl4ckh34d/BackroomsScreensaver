#include "audio_engine.h"

namespace {
float Clamp01(float value) {
    return std::clamp(value, 0.0f, 1.0f);
}

float SmoothStep(float edge0, float edge1, float x) {
    if (edge1 <= edge0) return x >= edge1 ? 1.0f : 0.0f;
    float t = Clamp01((x - edge0) / (edge1 - edge0));
    return t * t * (3.0f - 2.0f * t);
}

float Lerp(float a, float b, float t) {
    return a + (b - a) * t;
}
}

bool AudioEngine::Initialize(const AudioEngineSettings& settings) {
    if (initialized_) {
        ApplySettings(settings);
        return true;
    }
    if (!comInitialized_) {
        comHr_ = CoInitializeEx(nullptr, COINIT_MULTITHREADED);
        comInitialized_ = SUCCEEDED(comHr_);
    }
    HRESULT hr = XAudio2Create(&xaudio_, 0, XAUDIO2_DEFAULT_PROCESSOR);
    if (FAILED(hr) || !xaudio_) {
        OutputDebugStringW(L"Backrooms audio: XAudio2Create failed.\n");
        return false;
    }
    hr = MFStartup(MF_VERSION);
    if (SUCCEEDED(hr)) {
        mediaFoundationStarted_ = true;
    } else {
        OutputDebugStringW(L"Backrooms audio: Media Foundation startup failed; MP3 samples will not load.\n");
    }
    hr = xaudio_->CreateMasteringVoice(&masterVoice_);
    if (FAILED(hr)) {
        OutputDebugStringW(L"Backrooms audio: CreateMasteringVoice failed.\n");
        xaudio_.Reset();
        if (mediaFoundationStarted_) {
            MFShutdown();
            mediaFoundationStarted_ = false;
        }
        return false;
    }
    XAUDIO2_VOICE_DETAILS details{};
    masterVoice_->GetVoiceDetails(&details);
    outputChannels_ = std::max<UINT32>(1, details.InputChannels);
    DWORD channelMask = 0;
    masterVoice_->GetChannelMask(&channelMask);
    if (channelMask == 0) {
        channelMask = outputChannels_ == 1 ? SPEAKER_FRONT_CENTER :
            (outputChannels_ == 2 ? SPEAKER_STEREO : SPEAKER_5POINT1);
    }
    X3DAudioInitialize(channelMask, X3DAUDIO_SPEED_OF_SOUND, x3dHandle_);
    initialized_ = true;
    ApplySettings(settings);
    return true;
}

void AudioEngine::Shutdown() {
    StopAll();
    if (masterVoice_) {
        masterVoice_->DestroyVoice();
        masterVoice_ = nullptr;
    }
    xaudio_.Reset();
    initialized_ = false;
    if (mediaFoundationStarted_) {
        MFShutdown();
        mediaFoundationStarted_ = false;
    }
    if (comInitialized_) {
        CoUninitialize();
        comInitialized_ = false;
        comHr_ = E_FAIL;
    }
}

void AudioEngine::ApplySettings(const AudioEngineSettings& settings) {
    muted_ = settings.audioMuted;
    masterVolume_ = std::clamp(settings.audioMasterVolume, 0.0f, 1.0f);
    musicVolume_ = std::clamp(settings.audioMusicVolume, 0.0f, 1.0f);
    effectsVolume_ = std::clamp(settings.audioEffectsVolume, 0.0f, 1.0f);
    ambienceVolume_ = std::clamp(settings.audioAmbienceVolume, 0.0f, 1.0f);
    monsterVolume_ = std::clamp(settings.audioMonsterVolume, 0.0f, 1.0f);
}

void AudioEngine::LoadAll(const AudioEngineAssets& assets) {
    if (!initialized_) return;
    samples_.clear();
    groups_.clear();
    groups_.resize(static_cast<size_t>(GameSound::TitleTheme) + 1);
    AddFolder(assets, GameSound::CarpetStep, L"assets\\sounds\\carpet_steps", L"carpet_step_*.wav");
    AddFolder(assets, GameSound::SoakedCarpetStep, L"assets\\sounds\\soaked_carpet_steps", L"soaked_step_*.wav");
    AddFolder(assets, GameSound::NeonHumQuiet, L"assets\\sounds\\neon_light_hum", L"*quiet*.wav");
    AddFolder(assets, GameSound::NeonHumLoud, L"assets\\sounds\\neon_light_hum", L"neon_light_hum_loud.wav");
    AddFolder(assets, GameSound::NeonHumLoud2, L"assets\\sounds\\neon_light_hum", L"neon_light_hum_loud_2.wav");
    AddFolder(assets, GameSound::MonsterGrowl, L"assets\\sounds\\monster_growls");
    AddFolder(assets, GameSound::MonsterSpottedScream, L"assets\\sounds\\monster_spotted_scream");
    AddFolder(assets, GameSound::ElectricCrackle, L"assets\\sounds\\electric_crackling");
    AddFolder(assets, GameSound::NeonFlickerStarterClick, L"assets\\sounds\\neon_light_flicker_start");
    AddFolder(assets, GameSound::AirVentDustPuff, L"assets\\sounds\\air_vent_dust_air_puff");
    AddFolder(assets, GameSound::WetCarpetCeilingDrip, L"assets\\sounds\\wet_carpet_ceiling_drips", L"muted_cardboard_drip_tap_*.wav");
    AddFolder(assets, GameSound::DoorOpenCreak, L"assets\\sounds\\door_open_creak");
    AddFolder(assets, GameSound::DoorCloseCreak, L"assets\\sounds\\door_close_creak");
    AddFolder(assets, GameSound::DoorCloseLock, L"assets\\sounds\\door_close_lock");
    AddFolder(assets, GameSound::LightBulbBreak, L"assets\\sounds\\light_bulb_break", L"light_bulb_break_1.wav");
    AddFolder(assets, GameSound::VisionFlash, L"assets\\sounds\\vision_flash", L"vision_flash_*.wav");
    AddFolder(assets, GameSound::FlashlightStutter, L"assets\\sounds\\flashlight_contact_click", L"flashlight_stutter.wav");
    AddFolder(assets, GameSound::PaperFlutter, L"assets\\sounds\\paper_flutter", L"paper_flutter.wav");
    AddFolder(assets, GameSound::TitleTheme, L"assets\\music", L"title.mp3");
    if (samples_.empty()) {
        OutputDebugStringW(L"Backrooms audio: no WAV samples were loaded.\n");
    }
}

void AudioEngine::SetListener(XMFLOAT3 pos, XMFLOAT3 forward) {
    listener_.Position = {pos.x, pos.y, pos.z};
    float len = std::sqrt(forward.x * forward.x + forward.y * forward.y + forward.z * forward.z);
    XMFLOAT3 f = len > 0.0001f
        ? XMFLOAT3{forward.x / len, forward.y / len, forward.z / len}
        : XMFLOAT3{0.0f, 0.0f, 1.0f};
    listener_.OrientFront = {f.x, f.y, f.z};
    listener_.OrientTop = {0.0f, 1.0f, 0.0f};
}

void AudioEngine::StopAll() {
    for (AudioVoiceInstance& instance : voices_) {
        if (instance.voice) instance.voice->DestroyVoice();
    }
    voices_.clear();
}

void AudioEngine::StopAllExceptTag(int preservedTag) {
    if (preservedTag < 0) {
        StopAll();
        return;
    }
    for (size_t i = 0; i < voices_.size();) {
        if (voices_[i].tag == preservedTag) {
            ++i;
            continue;
        }
        if (voices_[i].voice) voices_[i].voice->DestroyVoice();
        voices_.erase(voices_.begin() + static_cast<std::ptrdiff_t>(i));
    }
}
