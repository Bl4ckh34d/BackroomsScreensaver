R"(

float2 WorldDirtUv(float3 worldPos, float3 normal)
{
    float3 an = abs(normal);
    if (an.y >= max(an.x, an.z))
    {
        return worldPos.xz - gMaze0.xy;
    }
    if (an.z >= an.x)
    {
        return float2(worldPos.x - gMaze0.x, worldPos.y * 1.16);
    }
    return float2(worldPos.z - gMaze0.y, worldPos.y * 1.16);
}

void ApplyWorldDirtOverlay(inout float3 base, inout float roughness, inout float aoMap, float materialId, float3 worldPos, float3 normal)
{
    if (materialId >= 2.5)
    {
        return;
    }

    float2 p = WorldDirtUv(worldPos, normal);
    float materialSalt = materialId * 37.0 + 11.0;
    float levelDirt = saturate(gTexture0.w);
    float2 tileP = (worldPos.xz - gMaze0.xy) / max(gMaze0.zw, float2(0.20, 0.20));
    float surfaceVar = saturate(Fbm3(float3(p * 0.115 + gMaze1.xy * 0.021, materialSalt + 7.0)) * 1.18 - 0.08);
    float macroNoise = Fbm3(float3(tileP * 0.055 + gMaze1.xy * 0.071, materialSalt + 121.0));
    float hotspot = smoothstep(0.48, 0.88, macroNoise) * lerp(0.20, 0.58, levelDirt);
    [unroll]
    for (int i = 0; i < 5; ++i)
    {
        float fi = (float)i;
        float centerSeed = materialSalt + fi * 19.37 + gMaze1.x * 0.23 + gMaze1.y * 0.41;
        float enabled = step((fi + 0.45) / 5.0, 0.22 + levelDirt * 0.86);
        float2 center = float2(Hash21(float2(centerSeed, 3.1)), Hash21(float2(centerSeed, 9.7))) * gMaze1.xy;
        float2 radius = lerp(float2(3.2, 3.8), float2(8.5, 10.5), Hash21(float2(centerSeed, 17.3))) *
            lerp(0.72, 1.32, levelDirt);
        float d = length((tileP - center) / max(radius, float2(0.50, 0.50)));
        float core = 1.0 - smoothstep(0.22, 1.08, d + (Fbm3(float3(tileP * 0.18 + fi, materialSalt + 141.0)) - 0.5) * 0.24);
        hotspot = max(hotspot, core * enabled);
    }
    float hotspotGain = lerp(0.58, 1.12, levelDirt) + hotspot * lerp(0.45, 1.75, levelDirt);

    float warp = Fbm3(float3(p * 0.055 + surfaceVar * 2.1, materialSalt));
    float2 warped = p + (float2(warp, Noise3(float3(p.yx * 0.071 - surfaceVar * 1.7, materialSalt + 19.0))) - 0.5) *
        (1.6 + surfaceVar * 1.8);
    float2 dirtDx = ddx(warped);
    float2 dirtDy = ddy(warped);
    float pixelFootprint = max(length(dirtDx), length(dirtDy));
    float screenDetail = 1.0 - smoothstep(0.032, 0.175, pixelFootprint);
    float distanceDetail = 1.0 - smoothstep(12.0, 32.0, length(worldPos - gCameraPosTime.xyz));
    float detailFade = saturate(screenDetail * distanceDetail);
    float microFade = saturate(detailFade * (1.0 - smoothstep(0.086, 0.235, pixelFootprint)));

    float broad = Fbm3(float3(warped * 0.28 + surfaceVar * 3.0, materialSalt + 31.0));
    float mid = Fbm3(float3(warped * 1.75 + 17.0, materialSalt + 47.0));
    float fine = lerp(0.5, Noise3(float3(warped * 34.0 + surfaceVar * 13.0, materialSalt + 59.0)), detailFade);
    float pepper = lerp(0.5, Noise3(float3(warped * 92.0 + 31.0, materialSalt + 71.0)), detailFade);
    float micro = lerp(0.5, Noise3(float3(warped * 205.0 + surfaceVar * 41.0, materialSalt + 127.0)), microFade);
    float pin = lerp(0.5, Noise3(float3(warped * 390.0 + surfaceVar * 67.0, materialSalt + 173.0)), microFade);
    float stain = smoothstep(0.43, 0.78, broad + (mid - 0.5) * 0.38);
    float grime = smoothstep(0.26, 0.66, mid + (fine - 0.5) * 0.32);
    float speckle = (smoothstep(0.62, 0.950, pepper) * 0.76 + smoothstep(0.52, 0.92, micro) * 0.42 +
        smoothstep(0.60, 0.97, pin) * 0.20) *
        (0.36 + grime * 0.64) * detailFade;

    float3 an = abs(normal);
    float isWall = 1.0 - step(max(an.x, an.z), an.y);
    float isFloor = step(max(an.x, an.z), normal.y);
    float isCeiling = step(max(an.x, an.z), -normal.y);
    float wallStreak = isWall *
        smoothstep(0.54, 0.84, Fbm3(float3(warped.x * 1.35 + materialSalt, worldPos.y * 0.20, materialSalt + 83.0))) *
        smoothstep(0.16, 0.82, worldPos.y) * (1.0 - smoothstep(2.40, 3.20, worldPos.y));
    float floorWear = isFloor * smoothstep(0.46, 0.82,
        Fbm3(float3(warped * 1.05 + float2(5.0, 19.0), materialSalt + 97.0)) +
        (fine - 0.5) * (0.10 + 0.18 * detailFade) + (micro - 0.5) * 0.13 * microFade + (pin - 0.5) * 0.06 * microFade);
    float ceilingBloom = isCeiling * smoothstep(0.38, 0.76,
        Fbm3(float3(warped * 0.74 + float2(29.0, 7.0), materialSalt + 109.0)) + (broad - 0.5) * 0.20 +
        (fine - 0.5) * 0.12 * detailFade + (micro - 0.5) * 0.06 * microFade);

    float2 coldWarp = p * 0.42 + float2(
        Fbm3(float3(p * 0.034 + 41.0, materialSalt + 181.0)),
        Fbm3(float3(p.yx * 0.041 - 23.0, materialSalt + 197.0))) * 5.8;
    coldWarp += float2(surfaceVar * 9.1, -surfaceVar * 6.7);
    float coldBroad = Fbm3(float3(coldWarp * 0.62 + gMaze1.xy * 0.013, materialSalt + 211.0));
    float coldVeins = Fbm3(float3(coldWarp * float2(1.7, 0.22) + float2(13.0, 5.0), materialSalt + 229.0));
    float coldDots = lerp(0.5, Noise3(float3(coldWarp * 38.0 + 17.0, materialSalt + 241.0)), detailFade);
    float moldPatches = smoothstep(0.57, 0.88, coldBroad + (coldVeins - 0.5) * 0.30);
    float verticalMold = isWall * smoothstep(0.12, 0.74, worldPos.y) * (1.0 - smoothstep(1.95, 3.05, worldPos.y)) *
        smoothstep(0.50, 0.86, coldVeins + (fine - 0.5) * 0.11);
    float floorDust = isFloor * smoothstep(0.50, 0.86, coldBroad + (pepper - 0.5) * 0.18);
    float ceilingMildew = isCeiling * smoothstep(0.44, 0.84, coldBroad + (coldVeins - 0.5) * 0.22);
    float moldFreckles = smoothstep(0.68, 0.965, coldDots) * detailFade * (0.28 + moldPatches * 0.72);
    float coldLayer = saturate((moldPatches * 0.52 + verticalMold * 0.72 + floorDust * 0.32 + ceilingMildew * 0.62 + moldFreckles * 0.36) *
)"
