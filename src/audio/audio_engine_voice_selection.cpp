#include "audio_engine.h"

size_t AudioEngine::PickSample(GameSound sound) {
    if (!initialized_) return kInvalidSample;
    const std::vector<size_t>& group = groups_[static_cast<size_t>(sound)];
    if (group.empty()) return kInvalidSample;
    size_t index = static_cast<size_t>(rng_() % group.size());
    return group[index];
}

float AudioEngine::BusVolume(AudioBus bus) const {
    if (muted_) return 0.0f;
    float busVolume = effectsVolume_;
    if (bus == AudioBus::Ambience) busVolume = ambienceVolume_;
    if (bus == AudioBus::Monster) busVolume = monsterVolume_;
    if (bus == AudioBus::Music) busVolume = musicVolume_;
    return masterVolume_ * busVolume;
}

float AudioEngine::OcclusionGain(AudioBus bus, float occlusion) const {
    occlusion = std::clamp(occlusion, 0.0f, 8.0f);
    float wallGain = std::pow(0.25f, occlusion);
    float floor = bus == AudioBus::Monster ? 0.0010f : (bus == AudioBus::Ambience ? 0.006f : 0.008f);
    return std::max(floor, wallGain);
}
