#include "audio_engine.h"

void AudioEngine::StartVoice(GameSound sound, size_t sampleIndex, AudioBus bus, XMFLOAT3 pos, float volume, bool loop, bool spatial,
                             float frequencyRatio, int tag, float initialOcclusion, AudioToneProfile toneProfile) {
    if (!initialized_ || sampleIndex >= samples_.size()) return;
    AudioSample& sample = samples_[sampleIndex];
    WAVEFORMATEX format = sample.format;
    std::vector<uint8_t> formatStorage;
    WAVEFORMATEX* formatPtr = &format;
    if (!sample.formatExtra.empty()) {
        formatStorage.resize(sizeof(WAVEFORMATEX) + sample.formatExtra.size());
        std::memcpy(formatStorage.data(), &sample.format, sizeof(WAVEFORMATEX));
        std::memcpy(formatStorage.data() + sizeof(WAVEFORMATEX), sample.formatExtra.data(), sample.formatExtra.size());
        formatPtr = reinterpret_cast<WAVEFORMATEX*>(formatStorage.data());
    }

    IXAudio2SourceVoice* voice = nullptr;
    HRESULT hr = xaudio_->CreateSourceVoice(&voice, formatPtr, XAUDIO2_VOICE_USEFILTER, XAUDIO2_DEFAULT_FREQ_RATIO, nullptr, nullptr, nullptr);
    if (FAILED(hr) || !voice) return;

    XAUDIO2_BUFFER buffer{};
    buffer.AudioBytes = static_cast<UINT32>(sample.data.size());
    buffer.pAudioData = sample.data.data();
    buffer.Flags = loop ? 0 : XAUDIO2_END_OF_STREAM;
    buffer.PlayBegin = 0;
    buffer.PlayLength = 0;
    buffer.LoopBegin = 0;
    buffer.LoopLength = 0;
    buffer.LoopCount = loop ? XAUDIO2_LOOP_INFINITE : 0;
    hr = voice->SubmitSourceBuffer(&buffer);
    if (FAILED(hr)) {
        voice->DestroyVoice();
        return;
    }

    AudioVoiceInstance instance{};
    instance.voice = voice;
    instance.sampleIndex = sampleIndex;
    instance.bus = bus;
    instance.sound = sound;
    instance.pos = pos;
    instance.baseVolume = std::clamp(volume, 0.0f, 6.0f);
    instance.frequencyRatio = std::clamp(frequencyRatio, 0.50f, 2.0f);
    instance.occlusion = spatial ? std::clamp(initialOcclusion, 0.0f, 8.0f) : 0.0f;
    instance.occlusionRefresh = spatial ? (bus == AudioBus::Ambience ? 0.32f : 0.14f) : 0.0f;
    instance.toneProfile = toneProfile;
    instance.tag = tag;
    instance.loop = loop;
    instance.spatial = spatial;
    if (instance.spatial && SpatialDistanceGain(instance) <= 0.0001f) {
        voice->DestroyVoice();
        return;
    }
    Apply3D(instance);
    voice->Start(0);
    voices_.push_back(instance);
}
