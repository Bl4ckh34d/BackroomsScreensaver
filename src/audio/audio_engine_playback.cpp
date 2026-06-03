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

void AudioEngine::PlayRandom(GameSound sound, AudioBus bus, XMFLOAT3 pos, float volume, bool spatial,
                             float initialOcclusion) {
    size_t sampleIndex = PickRandomSample(sound);
    if (sampleIndex == kInvalidSample) return;
    StartVoice(sound, sampleIndex, bus, pos, volume, false, spatial, 1.0f, -1, initialOcclusion);
}

void AudioEngine::PlayTagged(GameSound sound, AudioBus bus, XMFLOAT3 pos, float volume, bool spatial, int tag,
                             float initialOcclusion) {
    if (tag >= 0 && HasTaggedVoice(tag)) return;
    size_t sampleIndex = PickRandomSample(sound);
    if (sampleIndex == kInvalidSample) return;
    StartVoice(sound, sampleIndex, bus, pos, volume, false, spatial, 1.0f, tag, initialOcclusion);
}

void AudioEngine::StartLoop(GameSound sound, AudioBus bus, XMFLOAT3 pos, float volume, bool spatial,
                            float initialOcclusion) {
    size_t sampleIndex = PickRandomSample(sound);
    if (sampleIndex == kInvalidSample) return;
    StartVoice(sound, sampleIndex, bus, pos, volume, true, spatial, 1.0f, -1, initialOcclusion);
}

void AudioEngine::StartLoopTagged(GameSound sound, AudioBus bus, XMFLOAT3 pos, float volume, bool spatial, int tag,
                                  float initialOcclusion) {
    if (tag >= 0 && HasTaggedVoice(tag)) return;
    size_t sampleIndex = PickRandomSample(sound);
    if (sampleIndex == kInvalidSample) return;
    StartVoice(sound, sampleIndex, bus, pos, volume, true, spatial, 1.0f, tag, initialOcclusion);
}

void AudioEngine::StartLoopTaggedSample(GameSound sound, size_t sampleIndex, AudioBus bus, XMFLOAT3 pos, float volume,
                                        bool spatial, int tag, float initialOcclusion) {
    if (tag >= 0 && HasTaggedVoice(tag)) return;
    if (sampleIndex == kInvalidSample) return;
    StartVoice(sound, sampleIndex, bus, pos, volume, true, spatial, 1.0f, tag, initialOcclusion);
}

bool AudioEngine::HasTaggedVoice(int tag) const {
    if (tag < 0) return false;
    return std::any_of(voices_.begin(), voices_.end(), [tag](const AudioVoiceInstance& instance) {
        return instance.tag == tag;
    });
}

void AudioEngine::StopTaggedVoice(int tag) {
    if (tag < 0) return;
    for (size_t i = 0; i < voices_.size();) {
        if (voices_[i].tag == tag) {
            if (voices_[i].voice) voices_[i].voice->DestroyVoice();
            voices_.erase(voices_.begin() + static_cast<std::ptrdiff_t>(i));
            continue;
        }
        ++i;
    }
}

size_t AudioEngine::PickRandomSample(GameSound sound) {
    return PickSample(sound);
}

size_t AudioEngine::PickStableSample(GameSound sound, uint32_t stableId) const {
    if (!initialized_) return kInvalidSample;
    const std::vector<size_t>& group = groups_[static_cast<size_t>(sound)];
    if (group.empty()) return kInvalidSample;
    return group[static_cast<size_t>(stableId) % group.size()];
}

size_t AudioEngine::PickRandomSampleExcept(GameSound sound, size_t excludedSampleIndex) {
    if (!initialized_) return kInvalidSample;
    const std::vector<size_t>& group = groups_[static_cast<size_t>(sound)];
    if (group.empty()) return kInvalidSample;
    if (group.size() == 1) return group.front() == excludedSampleIndex ? kInvalidSample : group.front();
    for (size_t attempt = 0; attempt < 6; ++attempt) {
        size_t index = group[static_cast<size_t>(rng_() % group.size())];
        if (index != excludedSampleIndex) return index;
    }
    for (size_t index : group) {
        if (index != excludedSampleIndex) return index;
    }
    return kInvalidSample;
}

void AudioEngine::PlaySample(size_t sampleIndex, AudioBus bus, XMFLOAT3 pos, float volume, bool spatial,
                             float frequencyRatio, GameSound sound, float initialOcclusion,
                             AudioToneProfile toneProfile) {
    if (sampleIndex == kInvalidSample) return;
    StartVoice(sound, sampleIndex, bus, pos, volume, false, spatial, frequencyRatio, -1, initialOcclusion, toneProfile);
}

std::filesystem::path AudioEngine::ResolveSoundFolder(const AudioEngineAssets& assets, const std::wstring& folder) const {
    std::vector<std::filesystem::path> roots;
    auto add = [&](std::filesystem::path p) {
        if (p.empty()) return;
        p = p.lexically_normal();
        if (std::find(roots.begin(), roots.end(), p) == roots.end()) roots.push_back(std::move(p));
    };
    add(assets.moduleDirectory / folder);
    add(assets.moduleDirectory.parent_path() / folder);
    add(assets.moduleDirectory.parent_path().parent_path() / folder);
    add(assets.currentDirectory / folder);
    if (!assets.configuredAssetFolder.empty()) {
        std::filesystem::path configured(assets.configuredAssetFolder);
        std::filesystem::path soundRelative = folder;
        if (soundRelative.is_relative()) {
            add((configured.is_absolute() ? configured : assets.moduleDirectory / configured) / soundRelative);
        }
    }
    for (const auto& root : roots) {
        std::error_code ec;
        if (std::filesystem::exists(root, ec)) return root;
    }
    return {};
}
