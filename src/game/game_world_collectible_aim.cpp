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

GameWorldCollectibleAimResult GameWorld::FindCollectiblePageInView(float maxDistance) const {
        GameWorldCollectibleAimResult result{};
        XMFLOAT3 origin{player.position.x, player.position.y, player.position.z};
        float pitchCos = std::cos(player.pitch);
        XMFLOAT3 dir{
            std::sin(player.yaw) * pitchCos,
            std::sin(player.pitch),
            std::cos(player.yaw) * pitchCos
        };
        float bestT = std::max(0.0f, maxDistance);
        Tile cameraTile = maze.TileFromWorld(player.position.x, player.position.z);

        for (size_t i = 0; i < collectiblePages.size(); ++i) {
            const CollectiblePage& page = collectiblePages[i];
            if (page.collected) continue;

            float denom = dir.x * page.normal.x + dir.y * page.normal.y + dir.z * page.normal.z;
            if (std::abs(denom) < 0.08f) continue;

            XMFLOAT3 toPage{
                page.center.x - origin.x,
                page.center.y - origin.y,
                page.center.z - origin.z
            };
            float t = (toPage.x * page.normal.x + toPage.y * page.normal.y + toPage.z * page.normal.z) / denom;
            if (t <= 0.05f || t >= bestT) continue;

            XMFLOAT3 hit{
                origin.x + dir.x * t,
                origin.y + dir.y * t,
                origin.z + dir.z * t
            };
            XMFLOAT3 local{
                hit.x - page.center.x,
                hit.y - page.center.y,
                hit.z - page.center.z
            };
            float u = local.x * page.right.x + local.y * page.right.y + local.z * page.right.z;
            float v = local.x * page.up.x + local.y * page.up.y + local.z * page.up.z;
            if (std::abs(u) > 0.115f || std::abs(v) > 0.160f) continue;

            Tile pageTile = maze.TileFromWorld(page.center.x, page.center.z);
            if (!maze.LineClear(cameraTile, pageTile)) continue;

            bestT = t;
            result.found = true;
            result.pageSlot = i;
        }
        return result;
    }
