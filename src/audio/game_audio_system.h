#pragma once

struct DelayedAudioEvent {
    size_t sampleIndex = static_cast<size_t>(-1);
    GameAudioEventCategory category = GameAudioEventCategory::Generic;
    GameSound sound = GameSound::MonsterGrowl;
    AudioBus bus = AudioBus::Effects;
    XMFLOAT3 pos{};
    float volume = 1.0f;
    float delay = 0.0f;
    float frequencyRatio = 1.0f;
    AudioToneProfile toneProfile = AudioToneProfile::Normal;
    bool spatial = true;
};

struct LampHumCandidate {
    size_t index = 0;
    float distSq = 0.0f;
};

struct AudioRuntimeState {
    static constexpr size_t kMaxDelayedEvents = 24;

    float nextMonsterGrowlSeconds = 0.0f;
    float monsterSpottedScreamCooldown = 0.0f;
    float monsterAlertVocalTimer = 0.0f;
    bool monsterAlertAudioActive = false;
    float ventMonsterGroanTimer = 0.0f;
    float ventMonsterGroanCooldown = 0.0f;
    float previousStepAudioPhase = 0.0f;
    bool exitDoorOpenSoundPlayed = false;
    bool exitDoorCloseCreakSoundPlayed = false;
    bool exitDoorCloseSoundPlayed = false;
    bool menuDoorAudioPrimed = false;
    bool menuDoorAudioOpen = false;
    bool menuDoorCloseCreakPlayed = false;
    bool menuDoorCloseLockPlayed = false;
    float previousMenuDoorAudioOpen = 0.0f;
    float menuDoorAudioPeakOpen = 0.0f;
    float lampHumRefreshTimer = 0.0f;
    std::vector<LampHumCandidate> lampHumCandidates;
    std::vector<uint8_t> lampHumShouldPlay;
    std::vector<DelayedAudioEvent> delayedEvents;

    static int FootstepDownBobIndex(float phase, float pi);

    bool ConsumeFootstepTrigger(bool moving, float stepPhase, float pi);

    void QueueDelayedEvent(size_t sampleIndex, GameSound sound, AudioBus bus, XMFLOAT3 pos, float volume, float delay,
                           float frequencyRatio = 1.0f, bool spatial = true,
                           AudioToneProfile toneProfile = AudioToneProfile::Normal,
                           GameAudioEventCategory category = GameAudioEventCategory::Generic);

    template <typename PlayReadyEvent>
    void DrainReadyDelayedEvents(float dt, PlayReadyEvent&& playReadyEvent) {
        for (size_t i = 0; i < delayedEvents.size();) {
            DelayedAudioEvent& e = delayedEvents[i];
            e.delay -= dt;
            if (e.delay <= 0.0f) {
                playReadyEvent(e);
                delayedEvents.erase(delayedEvents.begin() + static_cast<std::ptrdiff_t>(i));
                continue;
            }
            ++i;
        }
    }

    void ResetMenuDoorTracking();

    void PrimeMenuDoorTracking(float doorOpen);

    void ResetForScene(float monsterGrowlSeconds, float ventGroanTimer, float ventGroanCooldown, float stepPhase);
};
