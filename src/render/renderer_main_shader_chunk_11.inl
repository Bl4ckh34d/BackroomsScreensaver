R"(
                float2 hi = lo + 1.0;
                float2 outside = max(max(lo - cell, cell - hi), 0.0);
                best = min(best, length(outside));
            }
        }
    }

    return best;
}

float2 LampAreaSample(int index)
{
    if (index == 0) return float2(0.0, 0.0);
    if (index == 1) return float2(0.66, 0.0);
    if (index == 2) return float2(-0.66, 0.0);
    if (index == 3) return float2(0.0, 0.58);
    if (index == 4) return float2(0.0, -0.58);
    if (index == 5) return float2(0.48, 0.42);
    if (index == 6) return float2(-0.48, 0.42);
    if (index == 7) return float2(0.48, -0.42);
    return float2(-0.48, -0.42);
}

float LampAreaRayVisibility(float2 startXZ, float2 lampXZ, float2 dir, float2 perp, float tileSize, float distFade)
{
    float sourceWidth = tileSize * lerp(0.18, 0.58, distFade);
    float sourceLength = tileSize * lerp(0.10, 0.30, distFade);
    float receiverWidth = tileSize * lerp(0.040, 0.210, distFade);
    float vis = 0.0;
    float weightSum = 0.0;

    [loop]
    for (int i = 0; i < 9; ++i)
    {
        float2 s = LampAreaSample(i);
        float diagonal = step(0.82, abs(s.x) + abs(s.y));
        float center = 1.0 - step(0.01, dot(s, s));
        float weight = lerp(1.0, 0.72, diagonal) + center * 0.38;
        float2 source = lampXZ + perp * (s.x * sourceWidth) + dir * (s.y * sourceLength);
        float2 receiver = startXZ - perp * (s.x * receiverWidth * 0.42) - dir * (s.y * receiverWidth * 0.18);
        vis += LampRayClear(receiver, source) * weight;
        weightSum += weight;
    }

    return vis / max(weightSum, 0.001);
}

float LampVisibility(float2 worldXZ, float3 worldN, float2 lampXZ)
{
    float tileSize = gMaze1.w;
    float sourceTileSize = tileSize / 3.0;
    float2 startXZ = worldXZ + worldN.xz * tileSize * 0.16;
    float2 delta = lampXZ - startXZ;
    float dist = max(length(delta), 0.001);
    float2 dir = delta / dist;
    float2 perp = float2(-dir.y, dir.x);
    float distFade = saturate(dist / max(tileSize * 4.5, 0.001));
    float2 deltaCells = abs(delta / max(tileSize, 0.001));
    float hallFill = max(exp2(-deltaCells.x * deltaCells.x * 1.95),
                         exp2(-deltaCells.y * deltaCells.y * 1.95));
    float localFill = exp2(-dot(deltaCells, deltaCells) * 0.36);
    float areaFill = saturate(max(hallFill * 0.95, localFill * 0.58));

    float2 startCell = (startXZ - gMaze0.xy) / gMaze0.zw;
    float clearance = NearestClosedCellDistance(startCell);
    float cornerFeather = smoothstep(0.045, 0.44 + distFade * 0.20, clearance);

    float rayArea = LampAreaRayVisibility(startXZ, lampXZ, dir, perp, sourceTileSize, distFade);
    float raySoft = smoothstep(-0.04, 1.04, rayArea);
    raySoft = saturate(raySoft + rayArea * (1.0 - rayArea) * (0.18 + distFade * 0.12));
    float occludedBounce = 0.10 + cornerFeather * 0.12 + distFade * 0.045;
    float occlusion = lerp(occludedBounce, 1.0, raySoft);
    return saturate(areaFill * occlusion * (0.58 + cornerFeather * 0.42));
}

float LampRayClear(float2 startXZ, float2 endXZ)
{
    float2 startCell = (startXZ - gMaze0.xy) / gMaze0.zw;
    float2 endCell = (endXZ - gMaze0.xy) / gMaze0.zw;
    int2 tile = (int2)floor(startCell);
    int2 endTile = (int2)floor(endCell);
    if (MazeOpenAt(tile) < 0.5 || MazeOpenAt(endTile) < 0.5)
    {
        return 0.0;
    }

    float2 ray = endCell - startCell;
    float2 safeRay = float2(
        abs(ray.x) < 0.0001 ? (ray.x < 0.0 ? -0.0001 : 0.0001) : ray.x,
        abs(ray.y) < 0.0001 ? (ray.y < 0.0 ? -0.0001 : 0.0001) : ray.y);
    int2 stepTile = int2(ray.x >= 0.0 ? 1 : -1, ray.y >= 0.0 ? 1 : -1);
    float2 absRay = max(abs(ray), float2(0.0001, 0.0001));
    float2 nextBoundary = float2(
        stepTile.x > 0 ? (float)tile.x + 1.0 : (float)tile.x,
        stepTile.y > 0 ? (float)tile.y + 1.0 : (float)tile.y);
)"
