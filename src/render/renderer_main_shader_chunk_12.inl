R"(
    float2 tMax = abs((nextBoundary - startCell) / safeRay);
    float2 tDelta = 1.0 / absRay;

    [loop]
    for (int i = 0; i < 96; ++i)
    {
        if (tile.x == endTile.x && tile.y == endTile.y)
        {
            return 1.0;
        }
        if (tMax.x < tMax.y)
        {
            tile.x += stepTile.x;
            tMax.x += tDelta.x;
        }
        else
        {
            tile.y += stepTile.y;
            tMax.y += tDelta.y;
        }
        if (MazeLightOpenAt(tile) < 0.5)
        {
            return 0.0;
        }
    }
    return 0.0;
}

float ShadowVisibility(float3 worldPos, float3 normal)
{
    if (gShadow0.w <= 0.001)
    {
        return 1.0;
    }

    float3 samplePos = worldPos + normalize(normal) * 0.018;
    float4 lightPos = mul(float4(samplePos, 1.0), gLightViewProj);
    if (lightPos.w <= 0.0001)
    {
        return 1.0;
    }

    float3 ndc = lightPos.xyz / lightPos.w;
    float2 uv = float2(ndc.x * 0.5 + 0.5, -ndc.y * 0.5 + 0.5);
    if (uv.x <= 0.001 || uv.x >= 0.999 || uv.y <= 0.001 || uv.y >= 0.999 || ndc.z <= 0.0 || ndc.z >= 1.0)
    {
        return 1.0;
    }

    float3 lightPosWorld = gShadow0.xyz;
    float3 toLight = normalize(lightPosWorld - samplePos);
    float slope = 1.0 - saturate(dot(normalize(normal), toLight));
    float compareDepth = ndc.z - max(gShadow1.w * (1.0 + slope * 2.4), 0.00045);
    float texel = max(gShadow2.x, 0.0001);
    float receiver = saturate(length(worldPos - lightPosWorld) / max(gShadow2.y, 0.01));
    float penumbra = pow(receiver, 0.72);
    float filterRadius = lerp(0.85, 5.20, penumbra);
    float2 stableCell = floor(worldPos.xz * 3.1 + worldPos.y * 1.7);
    float2 jitter = (float2(Hash21(stableCell + 13.0), Hash21(stableCell + 71.0)) - 0.5) * texel * lerp(0.16, 0.42, penumbra);
    float visible = 0.0;
    float weightSum = 0.0;

    [loop]
    for (int y = -1; y <= 1; ++y)
    {
        [loop]
        for (int x = -1; x <= 1; ++x)
        {
            float wx = 2.0 - abs((float)x);
            float wy = 2.0 - abs((float)y);
            float weight = wx * wy;
            float2 offset = float2(x, y) * texel * filterRadius + jitter;
            visible += gShadowMap.SampleCmpLevelZero(gShadowSampler, uv + offset, compareDepth) * weight;
            weightSum += weight;
        }
    }
    visible /= max(weightSum, 0.001);
    visible = saturate(visible + (1.0 - visible) * penumbra * 0.035);
    float strength = gShadow0.w * lerp(1.0, 0.74, penumbra);
    return lerp(1.0 - strength, 1.0, visible);
}

float FixtureShadowVisibility(float3 worldPos, float3 normal, float3 lampPos)
{
    if (gFixtureShadow0.w <= 0.001)
    {
        return 1.0;
    }

    float dist = length(worldPos - lampPos);
    if (dist > gFixtureShadow1.y)
    {
        return 1.0;
    }

)"
