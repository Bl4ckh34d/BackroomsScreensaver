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

void GameWorld::ResetCollectiblePagesForGeneration() {
    collectiblePagesCollected = 0;
    for (CollectiblePage& page : collectiblePages) {
        page = {};
        page.collected = true;
    }
}

bool GameWorld::PlaceCollectiblePage(int pageSlot, const CollectiblePage& page) {
    if (pageSlot < 0 || pageSlot >= static_cast<int>(collectiblePages.size())) return false;
    collectiblePages[static_cast<size_t>(pageSlot)] = page;
    return true;
}

bool GameWorld::IsLayerPageCollected(int pageIndex) const {
    return playableRun.active && playableRun.IsLayerPageCollected(pageIndex);
}

GameWorldCollectiblePickupResult GameWorld::CollectPage(size_t pageSlot, bool updatePlayableRun) {
    GameWorldCollectiblePickupResult result{};
    if (pageSlot >= collectiblePages.size()) return result;

    CollectiblePage& page = collectiblePages[pageSlot];
    if (page.collected) return result;

    page.collected = true;
    collectiblePagesCollected = std::clamp(collectiblePagesCollected + 1, 0, kCollectiblePageMaterialCount);
    if (updatePlayableRun && playableRun.active) {
        playableRun.MarkLayerPageCollected(page.pageIndex);
    }

    result.collected = true;
    result.pageIndex = page.pageIndex;
    result.displayCollected = updatePlayableRun && playableRun.active
        ? playableRun.layerPagesCollected
        : collectiblePagesCollected;
    return result;
}

void GameWorld::ClearSavePoint() {
    savePoint = {};
}

void GameWorld::RestoreSavePoint(bool active, const XMFLOAT3& pos, float yaw) {
    savePoint.active = active;
    savePoint.pos = pos;
    savePoint.yaw = yaw;
}

void GameWorld::RestoreCollectiblePageCollected(size_t pageSlot, bool collected, bool updatePlayableRun) {
    if (pageSlot >= collectiblePages.size()) return;

    CollectiblePage& page = collectiblePages[pageSlot];
    page.collected = collected;
    if (updatePlayableRun && playableRun.active && page.collected) {
        playableRun.RestoreLayerPageCollected(page.pageIndex, true);
    }
}

void GameWorld::ReconcileCollectiblePageRestore(int savedCollectedPages) {
    collectiblePagesCollected = std::clamp(savedCollectedPages, 0, kCollectiblePageMaterialCount);
    playableRun.ReconcileLayerPagesCollected();
}
