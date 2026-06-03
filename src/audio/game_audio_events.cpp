#include "../platform/platform_headers.h"

#include "audio_engine.h"
#include "game_audio_events.h"

GameAudioEvent GameAudioEvent::WithCategory(GameAudioEventCategory value) const {
    GameAudioEvent event = *this;
    event.category = value;
    return event;
}

GameAudioEvent GameAudioEvent::OneShot(GameSound sound,
                                       AudioBus bus,
                                       const XMFLOAT3& pos,
                                       float volume,
                                       bool spatial,
                                       bool useOcclusion,
                                       float occlusionLimit) {
    GameAudioEvent event{};
    event.kind = GameAudioEventKind::PlayOneShot;
    event.sound = sound;
    event.bus = bus;
    event.pos = pos;
    event.volume = volume;
    event.spatial = spatial;
    event.useOcclusion = useOcclusion;
    event.occlusionLimit = occlusionLimit;
    return event;
}

GameAudioEvent GameAudioEvent::OneShotWithPlayerNoise(GameSound sound,
                                                      AudioBus bus,
                                                      const XMFLOAT3& pos,
                                                      float volume,
                                                      bool spatial,
                                                      float hearingRadius,
                                                      float hearingLife,
                                                      bool useOcclusion,
                                                      float occlusionLimit) {
    GameAudioEvent event = OneShot(sound, bus, pos, volume, spatial, useOcclusion, occlusionLimit);
    event.hearingRadius = hearingRadius;
    event.hearingLife = hearingLife;
    return event;
}

GameAudioEvent GameAudioEvent::PlayerNoise(const XMFLOAT3& pos,
                                           float hearingRadius,
                                           float hearingLife,
                                           GameAudioEventCategory category) {
    GameAudioEvent event{};
    event.kind = GameAudioEventKind::PlayerNoise;
    event.category = category;
    event.pos = pos;
    event.hearingRadius = hearingRadius;
    event.hearingLife = hearingLife;
    return event;
}
