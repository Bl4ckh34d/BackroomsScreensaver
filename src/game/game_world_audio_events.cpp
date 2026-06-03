#include "../platform/platform_headers.h"
#include "../core/maze_types.h"
#include "../core/math_utils.h"
#include "../core/constants.h"
#include "../debug/effect_debug_constants.h"
#include "../config/settings.h"
#include "../audio/audio_engine.h"
#include "../audio/game_audio_events.h"
#include "../maze/maze.h"
#include "../gameplay/playable_progression_types.h"
#include "../monster/monster_state.h"

#include "player_controller.h"
#include "player_state.h"
#include "game_world.h"

void GameWorld::RecomputePlayerNoiseRadiusFromPulses() {
    player.audibleNoiseRadiusMeters = 0.0f;
    for (const PlayerAudibleSoundPulse& pulse : playerSoundPulses) {
        if (pulse.radius <= 0.0f || pulse.life <= 0.0f || pulse.age >= pulse.life) continue;
        player.audibleNoiseRadiusMeters = std::max(player.audibleNoiseRadiusMeters, pulse.radius);
    }
}

void GameWorld::AdvancePlayerSoundPulses(float dt) {
    float step = std::max(0.0f, dt);
    for (PlayerAudibleSoundPulse& pulse : playerSoundPulses) {
        pulse.age += step;
    }
    playerSoundPulses.erase(std::remove_if(playerSoundPulses.begin(), playerSoundPulses.end(),
        [](const PlayerAudibleSoundPulse& pulse) {
            return pulse.life <= 0.0f || pulse.age >= pulse.life;
        }), playerSoundPulses.end());
    RecomputePlayerNoiseRadiusFromPulses();
}

bool GameWorld::EmitPlayerSoundPulse(const XMFLOAT3& pos, float radius, float life, size_t maxPulses) {
    if (radius <= 0.01f) return false;

    PlayerAudibleSoundPulse pulse{};
    pulse.pos = pos;
    pulse.radius = radius;
    pulse.life = std::max(0.10f, life);
    playerSoundPulses.push_back(pulse);
    while (playerSoundPulses.size() > maxPulses) {
        playerSoundPulses.erase(playerSoundPulses.begin());
    }
    player.audibleNoiseRadiusMeters = std::max(player.audibleNoiseRadiusMeters, radius);
    return true;
}

void GameWorld::QueueAudioEvent(const GameAudioEvent& event) {
    if (event.kind == GameAudioEventKind::PlayOneShot && event.volume <= 0.0f && event.hearingRadius <= 0.0f) return;
    if (event.kind == GameAudioEventKind::PlayerNoise && event.hearingRadius <= 0.0f) return;
    audioEvents.push_back(event);
    while (audioEvents.size() > 64) {
        audioEvents.erase(audioEvents.begin());
    }
}

std::vector<GameAudioEvent> GameWorld::DrainAudioEvents() {
    std::vector<GameAudioEvent> events;
    events.swap(audioEvents);
    return events;
}
