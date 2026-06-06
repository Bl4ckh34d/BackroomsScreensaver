R"(
        {
            wallEdgeFade = smoothstep(0.035, 0.24, worldPos.y) * smoothstep(0.035, 0.24, gMaze1.z - worldPos.y);
        }
        float floorCeilingFade = lerp(1.0, 0.88, smoothstep(0.55, 0.95, abs(normal.y)));
        float displacement = (foldRelief * pulse + crawl + suction) * gHorror0.w * (2.35 + flesh * 1.40) * wallEdgeFade * floorCeilingFade;
        displacement = clamp(displacement, -0.24, 0.42);
        worldPos += normal * displacement;
        float texel = 1.0 / 512.0;
        float hU = gNormalHeight.SampleLevel(gSampler, float3(fleshUv + float2(texel, 0.0), 15.0), 0).a;
        float hV = gNormalHeight.SampleLevel(gSampler, float3(fleshUv + float2(0.0, texel), 15.0), 0).a;
        float3 bitangent = normalize(cross(normal, tangent));
        float slopeScale = gHorror0.w * (1.9 + flesh * 0.9) * wallEdgeFade;
        float3 displacedNormal = normalize(normal - tangent * ((hU - height) * slopeScale * 10.0) - bitangent * ((hV - height) * slopeScale * 10.0));
        normal = normalize(lerp(normal, displacedNormal, saturate(flesh * 0.95)));
    }
    o.worldPos = worldPos;
    o.normal = normal;
    o.tangent = tangent;
    o.uv = rawUv;
    o.material = material;
    o.pos = mul(float4(worldPos, 1.0), gViewProj);
    return o;
}

float FixturePower(float3 worldPos, float time)
{
    float2 stride = gMaze0.zw;
    float2 lampOrigin = gMaze0.xy + gMaze0.zw * 0.5;
    float2 cell = floor((worldPos.xz - lampOrigin) / stride + 0.5);
    float h = Hash21(cell);
    float variation = lerp(0.86, 1.14, Hash21(cell + 53.0));
    float isOn = step(1.0 - gLighting1.y, h);
    float flickerFixture = step(1.0 - gLighting1.z, Hash21(cell + 17.0));
    float tick = floor(time * (1.3 + Hash21(cell + 37.0) * 2.5));
    float event = step(0.86, Hash21(cell + tick + 71.0));
    float flutter = 0.18 + 0.82 * saturate(sin(time * (41.0 + h * 50.0)) * 0.5 + 0.5);
    float buzz = lerp(1.0, 0.98 + 0.02 * sin(time * (55.0 + h * 80.0)), flickerFixture);
    float levelDirt = saturate(gTexture0.w);
    float broadGrime = Fbm3(float3(cell * 0.23 + gMaze1.xy * 0.017, 531.0));
    float clusterGrime = Fbm3(float3(cell * 0.071 - gMaze1.yx * 0.011, 577.0));
    float lampDirt = saturate((0.10 + levelDirt * 0.74) * (0.30 + broadGrime * 0.74) + smoothstep(0.58, 0.88, clusterGrime) * (0.12 + levelDirt * 0.22));
    float dirtDim = lerp(1.08, 0.56, smoothstep(0.04, 0.96, lampDirt));
    float basePower = isOn * lerp(1.0, flutter, flickerFixture * event) * buzz * variation * dirtDim;
    return basePower * LampFailureMultiplier(LampDamageAtWorld(worldPos.xz), h, time);
}

float LampDirtAmount(float2 cell)
{
    float levelDirt = saturate(gTexture0.w);
    float broadGrime = Fbm3(float3(cell * 0.23 + gMaze1.xy * 0.017, 531.0));
    float clusterGrime = Fbm3(float3(cell * 0.071 - gMaze1.yx * 0.011, 577.0));
    float flyChance = smoothstep(0.60, 0.95, Hash21(cell + 311.0) + levelDirt * 0.22);
    return saturate((0.10 + levelDirt * 0.74) * (0.30 + broadGrime * 0.74) +
        smoothstep(0.58, 0.88, clusterGrime) * (0.12 + levelDirt * 0.22) +
        flyChance * (0.04 + levelDirt * 0.12));
}

float LampVisualPower(float material, float3 worldPos, float time)
{
    float materialSeed = frac(material);
    float visualVariation = lerp(0.96, 1.06, frac(materialSeed * 23.71 + 0.31));
    return FixturePower(worldPos, time) * visualVariation;
}

float2 MazeTile(float2 worldXZ)
{
    return floor((worldXZ - gMaze0.xy) / gMaze0.zw);
}

float MazeVirtualExitCorridorOpen(int2 tile)
{
    if (gExitLight0.w <= 0.001 || gExitLight3.w <= 0.001)
    {
        return 0.0;
    }

    float2 tileCenter = gMaze0.xy + ((float2)tile + 0.5) * gMaze0.zw;
    float2 inward = normalize(gExitLight1.xz + float2(0.0001, 0.0001));
    float2 outward = -inward;
    float2 right = float2(outward.y, -outward.x);
    float2 rel = tileCenter - gExitLight3.xz;
    float axial = dot(rel, outward);
    float lateral = abs(dot(rel, right));
    float maxDepth = max(gMaze1.w * 14.0, 14.0);
    float depthOpen = smoothstep(gMaze1.w * 0.10, gMaze1.w * 0.35, axial) *
        (1.0 - smoothstep(maxDepth - gMaze1.w * 0.35, maxDepth + gMaze1.w * 0.35, axial));
    float widthOpen = 1.0 - step(gMaze1.w * 0.52, lateral);
    return depthOpen * widthOpen;
}

float MazeOpenAt(int2 tile)
{
    if (tile.x < 0 || tile.y < 0 || tile.x >= (int)gMaze1.x || tile.y >= (int)gMaze1.y)
    {
        return MazeVirtualExitCorridorOpen(tile);
    }
    return gMazeOpen.Load(int3(tile, 0)).r;
}

float MazeLightOpenAt(int2 tile)
{
    if (tile.x < 0 || tile.y < 0 || tile.x >= (int)gMaze1.x || tile.y >= (int)gMaze1.y)
    {
        return MazeVirtualExitCorridorOpen(tile);
    }
    return step(0.001, gMazeOpen.Load(int3(tile, 0)).r);
}

float LampRayClear(float2 startXZ, float2 endXZ);

float NearestClosedCellDistance(float2 cell)
{
    int2 baseTile = (int2)floor(cell);
    float best = 4.0;

    [loop]
    for (int yy = -1; yy <= 1; ++yy)
    {
        [loop]
        for (int xx = -1; xx <= 1; ++xx)
        {
            int2 tile = baseTile + int2(xx, yy);
            if (MazeLightOpenAt(tile) < 0.5)
            {
                float2 lo = (float2)tile;
)"
