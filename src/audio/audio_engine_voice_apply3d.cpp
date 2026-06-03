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
void AudioEngine::Apply3D(AudioVoiceInstance& instance) {
    if (!instance.voice || instance.sampleIndex >= samples_.size()) return;
    AudioSample& sample = samples_[instance.sampleIndex];
    if (instance.bus == AudioBus::Music) {
        instance.voice->SetVolume(instance.baseVolume * BusVolume(instance.bus));
        XAUDIO2_FILTER_PARAMETERS filter{};
        filter.Type = LowPassFilter;
        filter.Frequency = 1.0f;
        filter.OneOverQ = 1.0f;
        instance.voice->SetFilterParameters(&filter);
        instance.voice->SetFrequencyRatio(std::clamp(instance.frequencyRatio, XAUDIO2_MIN_FREQ_RATIO, XAUDIO2_MAX_FREQ_RATIO));
        return;
    }
    float occlusion = (!instance.spatial || instance.bus == AudioBus::Music) ? 0.0f : std::clamp(instance.occlusion, 0.0f, 8.0f);
    float occlusionGain = OcclusionGain(instance.bus, occlusion);
    float distanceGain = SpatialDistanceGain(instance);
    float toneGain = instance.toneProfile == AudioToneProfile::MetallicVent ? 0.42f : 1.0f;
    float volume = instance.baseVolume * BusVolume(instance.bus) * occlusionGain * distanceGain * toneGain;
    instance.voice->SetVolume(volume);
    if (instance.spatial && distanceGain <= 0.0001f) return;
    XAUDIO2_FILTER_PARAMETERS filter{};
    float cutoffHz = 18000.0f;
    if (instance.toneProfile == AudioToneProfile::MetallicVent) {
        filter.Type = BandPassFilter;
        cutoffHz = Lerp(1450.0f, 780.0f, SmoothStep(0.0f, 4.0f, occlusion));
        filter.OneOverQ = 0.22f;
    } else {
        filter.Type = LowPassFilter;
        float occludedCutoff = instance.bus == AudioBus::Monster ? 220.0f :
            (instance.bus == AudioBus::Ambience ? 420.0f : 520.0f);
        cutoffHz = Lerp(18000.0f, occludedCutoff, SmoothStep(0.0f, 4.0f, occlusion));
        filter.OneOverQ = 1.0f;
    }
    float nyquistSafe = std::max(10.0f, static_cast<float>(sample.format.nSamplesPerSec) * 0.45f);
    cutoffHz = std::clamp(cutoffHz, 10.0f, nyquistSafe);
    filter.Frequency = std::clamp(2.0f * std::sin(3.14159265358979323846f * cutoffHz / static_cast<float>(sample.format.nSamplesPerSec)),
        0.001f, 1.0f);
    instance.voice->SetFilterParameters(&filter);
    if (!instance.spatial || sample.format.nChannels != 1 || outputChannels_ == 0) {
        instance.voice->SetFrequencyRatio(std::clamp(instance.frequencyRatio, XAUDIO2_MIN_FREQ_RATIO, XAUDIO2_MAX_FREQ_RATIO));
        return;
    }

    X3DAUDIO_EMITTER emitter{};
    X3DAUDIO_CONE cone{};
    X3DAUDIO_DISTANCE_CURVE_POINT flatVolumePoints[2] = {{0.0f, 1.0f}, {1.0f, 1.0f}};
    X3DAUDIO_DISTANCE_CURVE flatVolumeCurve{flatVolumePoints, 2};
    emitter.pCone = &cone;
    emitter.pVolumeCurve = &flatVolumeCurve;
    emitter.ChannelCount = 1;
    emitter.CurveDistanceScaler = 1.0f;
    emitter.Position = {instance.pos.x, instance.pos.y, instance.pos.z};
    X3DAUDIO_DSP_SETTINGS dsp{};
    std::array<float, XAUDIO2_MAX_AUDIO_CHANNELS> matrix{};
    dsp.SrcChannelCount = 1;
    dsp.DstChannelCount = outputChannels_;
    dsp.pMatrixCoefficients = matrix.data();
    X3DAudioCalculate(x3dHandle_, &listener_, &emitter,
        X3DAUDIO_CALCULATE_MATRIX | X3DAUDIO_CALCULATE_DOPPLER, &dsp);
    instance.voice->SetOutputMatrix(masterVoice_, 1, outputChannels_, matrix.data());
    instance.voice->SetFrequencyRatio(std::clamp(dsp.DopplerFactor * instance.frequencyRatio, XAUDIO2_MIN_FREQ_RATIO, XAUDIO2_MAX_FREQ_RATIO));
}
