#include "../platform/platform_headers.h"

#include "audio_engine.h"
#include "game_audio_events.h"
#include "game_audio_system.h"

int AudioRuntimeState::FootstepDownBobIndex(float phase, float pi) {
    const float downBobPhaseOffset = pi * 0.75f;
    return static_cast<int>(std::floor((phase - downBobPhaseOffset) / pi));
}

bool AudioRuntimeState::ConsumeFootstepTrigger(bool moving, float stepPhase, float pi) {
    bool triggered = false;
    if (moving) {
        float oldPhase = previousStepAudioPhase;
        float newPhase = stepPhase;
        if (newPhase < oldPhase) newPhase += pi * 2.0f;
        triggered = FootstepDownBobIndex(newPhase, pi) > FootstepDownBobIndex(oldPhase, pi);
    }
    previousStepAudioPhase = stepPhase;
    return triggered;
}

void AudioRuntimeState::QueueDelayedEvent(size_t sampleIndex,
                                          GameSound sound,
                                          AudioBus bus,
                                          XMFLOAT3 pos,
                                          float volume,
                                          float delay,
                                          float frequencyRatio,
                                          bool spatial,
                                          AudioToneProfile toneProfile,
                                          GameAudioEventCategory category) {
    if (delayedEvents.size() >= kMaxDelayedEvents) delayedEvents.erase(delayedEvents.begin());
    DelayedAudioEvent e{};
    e.sampleIndex = sampleIndex;
    e.category = category;
    e.sound = sound;
    e.bus = bus;
    e.pos = pos;
    e.volume = volume;
    e.delay = std::max(0.0f, delay);
    e.frequencyRatio = frequencyRatio;
    e.toneProfile = toneProfile;
    e.spatial = spatial;
    delayedEvents.push_back(e);
}

void AudioRuntimeState::ResetMenuDoorTracking() {
    menuDoorAudioPrimed = false;
    menuDoorAudioOpen = false;
    menuDoorCloseCreakPlayed = false;
    menuDoorCloseLockPlayed = false;
    previousMenuDoorAudioOpen = 0.0f;
    menuDoorAudioPeakOpen = 0.0f;
}

void AudioRuntimeState::PrimeMenuDoorTracking(float doorOpen) {
    menuDoorAudioPrimed = true;
    menuDoorAudioOpen = doorOpen >= 0.035f;
    previousMenuDoorAudioOpen = doorOpen;
    menuDoorAudioPeakOpen = doorOpen;
    menuDoorCloseCreakPlayed = false;
    menuDoorCloseLockPlayed = doorOpen < 0.10f;
}

void AudioRuntimeState::ResetForScene(float monsterGrowlSeconds,
                                      float ventGroanTimer,
                                      float ventGroanCooldown,
                                      float stepPhase) {
    nextMonsterGrowlSeconds = monsterGrowlSeconds;
    monsterSpottedScreamCooldown = 0.0f;
    monsterAlertVocalTimer = 0.0f;
    monsterAlertAudioActive = false;
    this->ventMonsterGroanTimer = ventGroanTimer;
    this->ventMonsterGroanCooldown = ventGroanCooldown;
    previousStepAudioPhase = stepPhase;
    exitDoorOpenSoundPlayed = false;
    exitDoorCloseCreakSoundPlayed = false;
    exitDoorCloseSoundPlayed = false;
    lampHumRefreshTimer = 0.0f;
    delayedEvents.clear();
    ResetMenuDoorTracking();
}
