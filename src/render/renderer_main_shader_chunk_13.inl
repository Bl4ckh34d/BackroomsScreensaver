R"(
    float3 samplePos = worldPos + normalize(normal) * 0.012;
    float4 lightPos = mul(float4(samplePos, 1.0), gFixtureLightViewProj);
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

    float3 toLight = normalize(lampPos - samplePos);
    float slope = 1.0 - saturate(dot(normalize(normal), toLight));
    float compareDepth = ndc.z - max(gFixtureShadow1.x * (1.0 + slope * 2.0), 0.00036);
    float texel = max(gFixtureShadow1.z, 0.0001);
    float receiver = saturate(dist / max(gFixtureShadow1.y, 0.01));
    float filterRadius = lerp(0.85, 3.40, pow(receiver, 0.70));
    float visible = 0.0;
    float weightSum = 0.0;

    [loop]
    for (int y = -1; y <= 1; ++y)
    {
        [loop]
        for (int x = -1; x <= 1; ++x)
        {
            float weight = 2.0 - max(abs((float)x), abs((float)y));
            visible += gFixtureShadowMap.SampleCmpLevelZero(gShadowSampler, uv + float2(x, y) * texel * filterRadius, compareDepth) * weight;
            weightSum += weight;
        }
    }

    visible = saturate(visible / max(weightSum, 0.001));
    float strength = gFixtureShadow0.w * lerp(1.0, 0.70, receiver);
    return lerp(1.0 - strength, 1.0, visible);
}

float4 LoadBakedLampLightTexel(int2 texel, int2 maxTexel)
{
    texel = clamp(texel, int2(0, 0), maxTexel);
    return gBakedLampLight.Load(int3(texel, 0));
}

float4 SampleBakedLampLight(float2 worldXZ)
{
    uint texW = 0;
    uint texH = 0;
    gBakedLampLight.GetDimensions(texW, texH);
    if (texW == 0 || texH == 0)
    {
        return float4(0.0, 0.0, 0.0, 0.0);
    }

    float2 cell = (worldXZ - gMaze0.xy) / gMaze0.zw;
    if (cell.x < -0.05 || cell.y < -0.05 || cell.x > gMaze1.x + 0.05 || cell.y > gMaze1.y + 0.05)
    {
        return float4(0.0, 0.0, 0.0, 0.0);
    }

    float2 bakeScale = float2((float)texW / max(gMaze1.x, 1.0), (float)texH / max(gMaze1.y, 1.0));
    float2 texelPos = cell * bakeScale - 0.5;
    int2 baseTexel = (int2)floor(texelPos);
    float2 blend = saturate(texelPos - (float2)baseTexel);
    int2 maxTexel = int2((int)texW - 1, (int)texH - 1);

    float4 c00 = LoadBakedLampLightTexel(baseTexel, maxTexel);
    float4 c10 = LoadBakedLampLightTexel(baseTexel + int2(1, 0), maxTexel);
    float4 c01 = LoadBakedLampLightTexel(baseTexel + int2(0, 1), maxTexel);
    float4 c11 = LoadBakedLampLightTexel(baseTexel + int2(1, 1), maxTexel);
    return lerp(lerp(c00, c10, blend.x), lerp(c01, c11, blend.x), blend.y);
}

float2 DynamicDamagedLampResponse(float2 worldXZ, float3 worldN)
{
    float2 cell = (worldXZ - gMaze0.xy) / gMaze0.zw;
    int2 receiverTile = (int2)floor(cell);
    float2 tileStride = max(gMaze0.zw, float2(0.001, 0.001));
    float loss = 0.0;
    float flicker = 0.0;

    [loop]
    for (int y = -3; y <= 3; ++y)
    {
        [loop]
        for (int x = -3; x <= 3; ++x)
        {
            int2 lampTile = receiverTile + int2(x, y);
            if (lampTile.x < 0 || lampTile.y < 0 || lampTile.x >= (int)gMaze1.x || lampTile.y >= (int)gMaze1.y)
            {
                continue;
            }

            float damage = gLampDamage.Load(int3(lampTile, 0)).r;
            if (damage <= 0.001)
            {
                continue;
            }

            float2 lampXZ = gMaze0.xy + ((float2)lampTile + 0.5) * gMaze0.zw;
            float2 deltaTiles = (lampXZ - worldXZ) / tileStride;
            float tileDist = length(deltaTiles);
            float footprint = 1.0 - smoothstep(0.65, 3.85, tileDist);
            if (footprint <= 0.001)
            {
                continue;
            }

            float ray = LampRayClear(worldXZ + worldN.xz * gMaze1.w * 0.08, lampXZ);
            float fail = smoothstep(0.06, 0.96, damage);
            float contribution = footprint * ray / (1.0 + tileDist * tileDist * 0.32);
            loss += contribution * fail;
            flicker = max(flicker, contribution * fail);
        }
    }

    return float2(saturate(loss * 0.82), saturate(flicker));
}

float3 LocalLampLightColor(float3 worldPos, float3 worldN, float time)
{
    if (gLighting1.x <= 0.001 || gLighting1.y <= 0.001)
    {
        return float3(0.0, 0.0, 0.0);
    }

    float3 n = normalize(worldN);
    float floorReceiver = smoothstep(0.12, 0.82, n.y);
    float ceilingReceiver = smoothstep(0.12, 0.82, -n.y);
    float wallReceiver = saturate(1.0 - abs(n.y));
    float lowFeatureCeiling = ceilingReceiver * (1.0 - smoothstep(gMaze1.z * 0.68, gMaze1.z * 0.86, worldPos.y));
    ceilingReceiver *= lerp(1.0, 0.07, lowFeatureCeiling);

    float2 lightingXZ = worldPos.xz;
    if (wallReceiver > 0.35)
    {
        lightingXZ += n.xz * gMaze1.w * 0.18;
    }

    float4 baked = SampleBakedLampLight(lightingXZ);
    float2 dynamicDamage = DynamicDamagedLampResponse(lightingXZ, n);
    baked.rgb *= lerp(1.0, 0.20, dynamicDamage.x);
    baked.a = max(baked.a, dynamicDamage.y * 0.72);
    float flickerInfluence = saturate(baked.a);
    if (flickerInfluence > 0.0005)
    {
        float waveA = sin(time * 9.7 + worldPos.x * 0.031 + worldPos.z * 0.027) * 0.5 + 0.5;
        float waveB = sin(time * 17.3 + worldPos.x * 0.019 - worldPos.z * 0.024) * 0.5 + 0.5;
        float smoothDim = lerp(0.76, 1.04, waveA * 0.62 + waveB * 0.38);
        baked.rgb *= lerp(1.0, smoothDim, flickerInfluence * 0.30);
    }

    float wallHeight = saturate(worldPos.y / max(gMaze1.z, 0.001));
    float wallVertical = lerp(0.48, 1.04, smoothstep(0.04, 0.92, wallHeight));
    float receiverBase = lerp(0.18, 0.035, lowFeatureCeiling);
    float receiver = max(receiverBase, floorReceiver * 1.0 + wallReceiver * 0.72 * wallVertical + ceilingReceiver * 0.42);
    float3 light = baked.rgb * receiver;

    if (gFixtureShadow0.w > 0.001)
    {
        float3 fixturePos = gFixtureShadow0.xyz;
        float distXZ = length(worldPos.xz - fixturePos.xz);
        float selectedShadowLamp = 1.0 - smoothstep(gMaze1.w * 0.18, min(gFixtureShadow1.y, gMaze1.w * 1.85), distXZ);
        if (selectedShadowLamp > 0.001)
        {
            light *= lerp(1.0, FixtureShadowVisibility(worldPos, n, fixturePos), selectedShadowLamp);
        }
    }

    return light;
}

float LocalLampLight(float3 worldPos, float3 worldN, float time)
{
    return dot(LocalLampLightColor(worldPos, worldN, time), float3(0.299, 0.587, 0.114));
}
)"
