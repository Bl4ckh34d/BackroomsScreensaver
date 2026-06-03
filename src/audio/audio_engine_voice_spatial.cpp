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
}
float AudioEngine::SpatialDistanceGain(const AudioVoiceInstance& instance) const {
    if (!instance.spatial) return 1.0f;
    float dx = instance.pos.x - listener_.Position.x;
    float dy = instance.pos.y - listener_.Position.y;
    float dz = instance.pos.z - listener_.Position.z;
    float dist = std::sqrt(dx * dx + dy * dy + dz * dz);

    float inner = 0.65f;
    float maxDist = 18.0f;
    float power = 1.18f;
    if (instance.bus == AudioBus::Effects && instance.sampleIndex < samples_.size()) {
        const AudioSample& sample = samples_[instance.sampleIndex];
        if (!sample.data.empty() && sample.data.size() < 65536 && sample.format.nSamplesPerSec >= 22050) {
            inner = 1.15f;
            maxDist = 24.0f;
            power = 1.08f;
        }
        if (instance.sound == GameSound::LightBulbBreak) {
            inner = 1.35f;
            maxDist = 30.0f;
            power = 1.02f;
        }
    }
    if (instance.bus == AudioBus::Ambience) {
        inner = 0.85f;
        maxDist = 16.0f;
        power = 1.25f;
    } else if (instance.bus == AudioBus::Monster) {
        inner = 1.15f;
        maxDist = 22.0f;
        power = 1.35f;
    }

    if (dist <= inner) return 1.0f;
    float t = Clamp01((dist - inner) / std::max(0.1f, maxDist - inner));
    float shaped = std::pow(1.0f - t, power);
    float tail = 1.0f - SmoothStep(0.88f, 1.0f, t);
    return shaped * tail;
}
