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

bool GameWorld::PlayableRunFinished() const {
    return playableRun.runFinished;
}

bool GameWorld::PlayableRunActive() const {
    return playableRun.IsActive();
}

bool GameWorld::CanSaveActiveRun() const {
    return playableRun.CanSaveActiveRun();
}

void GameWorld::WritePlayableRunFields(std::wostream& out) const {
    playableRun.WriteSavedRunFields(out);
}

int GameWorld::RestoreSavedPlayableRunState(PlayableRunState::SavedRunRestoreState savedRun) {
    return playableRun.RestoreSavedRunState(std::move(savedRun));
}

void GameWorld::RestorePlayableSaveItemsSpawned(int saveItemsSpawned) {
    playableRun.RestoreSaveItemsSpawned(saveItemsSpawned);
}

PlayableLevelSpec GameWorld::CurrentPlayableLevel() const {
    return playableRun.CurrentLevel();
}

bool GameWorld::PlayableRunIsCustomGame() const {
    return playableRun.IsCustomGame();
}

const CustomGameSpec& GameWorld::CurrentCustomSpec() const {
    return playableRun.CustomSpec();
}

bool GameWorld::DarkLayerOneAppliesTo(int layer) const {
    return playableRun.DarkLayerOneAppliesTo(layer);
}

PlayableLevelSpec GameWorld::BuildLayerOneLevelSpec(int levelInLayer, std::mt19937& rng) {
    return playableRun.BuildLayerOneLevelSpec(levelInLayer, rng);
}

void GameWorld::BeginPlayableLevel(const PlayableLevelSpec& level) {
    playableRun.BeginLevel(level);
}

void GameWorld::BeginLayerRun(bool darkLayerOne, std::mt19937& rng) {
    playableRun.BeginLayerRun(darkLayerOne, rng);
}

void GameWorld::BeginCustomRun(const CustomGameSpec& customSpec, std::mt19937& rng) {
    playableRun.BeginCustomRun(customSpec, rng);
}

void GameWorld::WriteLevelStartNotice(std::wostream& out) const {
    playableRun.WriteLevelStartNotice(out);
}

float GameWorld::RunSeconds() const {
    return playableRun.RunSeconds();
}

float GameWorld::CurrentPlayableLevelSeconds() const {
    return playableRun.CurrentLevelSeconds();
}

int GameWorld::TotalScore() const {
    return playableRun.TotalScore();
}

bool GameWorld::PlayableScoreScreenActive() const {
    return playableRun.scoreScreenActive;
}

bool GameWorld::PlayableLevelRunning() const {
    return playableRun.active && playableRun.levelRunning;
}

bool GameWorld::PlayableBossLevelRunning() const {
    return PlayableLevelRunning() && playableRun.currentLevel.bossEncounter;
}

float GameWorld::AirParticleDensityScale() const {
    return playableRun.AirParticleDensityScale();
}

float GameWorld::MapDirtProgression() const {
    return playableRun.MapDirtProgression();
}

PlayableCustomScareGate GameWorld::CustomScareGateFor(int effectIndex) const {
    return playableRun.CustomScareGateFor(effectIndex);
}

int GameWorld::ScoreCompletedPlayableLevel(float levelSeconds, bool bossEncounter) const {
    int base = 1000 + playableRun.currentLevel.levelInLayer * 250;
    int speedBonus = std::max(0, 900 - static_cast<int>(std::round(levelSeconds * 6.0f)));
    int healthBonus = static_cast<int>(std::round(std::clamp(player.health, 0.0f, 100.0f) * 3.0f));
    int pageBonus = collectiblePagesCollected * 150;
    int bossBonus = bossEncounter ? 750 : 0;
    return std::max(0, base + speedBonus + healthBonus + pageBonus + bossBonus);
}

int GameWorld::ScoreCurrentPlayableLevel() const {
    return ScoreCompletedPlayableLevel(
        playableRun.CurrentLevelSeconds(),
        playableRun.CurrentLevelHasBossEncounter());
}

bool GameWorld::CanAdvancePlayableProgression() const {
    return progressionEnabled && PlayableLevelRunning() && !deathActive && !exitTransitionActive;
}

PlayableLevelCompletionUpdate GameWorld::CompleteCurrentPlayableLevel() {
    return playableRun.CompleteCurrentLevel(ScoreCurrentPlayableLevel());
}

bool GameWorld::CanContinueAfterScoreScreen() const {
    return playableRun.CanContinueAfterScoreScreen();
}

int GameWorld::NextLevelAfterScoreScreen() const {
    return playableRun.NextLevelAfterScoreScreen();
}

bool GameWorld::AdvancePlayableProgressionTimers(float dt) {
    if (!CanAdvancePlayableProgression()) return false;
    playableRun.AdvanceRunningTimers(dt);
    return true;
}
