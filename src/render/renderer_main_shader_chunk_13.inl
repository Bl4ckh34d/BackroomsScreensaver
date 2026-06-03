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

float3 LocalLampLightColor(float3 worldPos, float3 worldN, float time)
{
    float2 stride = gMaze0.zw;
    float spacing = gMaze1.w;
    float2 lampOrigin = gMaze0.xy + gMaze0.zw * 0.5;
    float2 baseCell = floor((worldPos.xz - lampOrigin) / stride + 0.5);
    float3 light = float3(0.0, 0.0, 0.0);

    [loop]
    for (int yy = -2; yy <= 2; ++yy)
    {
        [loop]
        for (int xx = -2; xx <= 2; ++xx)
        {
            float2 cell = baseCell + float2(xx, yy);
            float2 lampXZ = lampOrigin + cell * stride;
            if (MazeOpenAt((int2)MazeTile(lampXZ)) < 0.75)
            {
                continue;
            }
            float3 lampPos = float3(lampXZ.x, gMaze1.z - 0.09, lampXZ.y);
            float3 L = lampPos - worldPos;
            float d2 = dot(L, L);
            float distXZ = length(lampPos.xz - worldPos.xz);
            float reach = spacing * 1.18 + gMaze1.w * 0.75;
            float roomFootprint = 1.0 - smoothstep(reach * 0.48, reach, distXZ);
            float power = FixturePower(lampPos, time);
            if (roomFootprint <= 0.002 || power <= 0.002)
            {
                continue;
            }
            float visibility = LampVisibility(worldPos.xz, worldN, lampPos.xz);
            float3 Ln = normalize(L);
            float diffuse = saturate(dot(worldN, Ln) * 0.65 + 0.35);
            float falloff = visibility * roomFootprint / (1.0 + d2 * 0.035);
            float selectedShadowLamp = 1.0 - smoothstep(0.05, 0.42, length(lampPos.xz - gFixtureShadow0.xz));
            if (selectedShadowLamp > 0.001)
            {
                falloff *= lerp(1.0, FixtureShadowVisibility(worldPos, worldN, lampPos), selectedShadowLamp);
            }
            float dirt = LampDirtAmount(cell);
            float dirtTint = smoothstep(0.05, 0.95, dirt);
            float dirtTransmission = lerp(1.0, 0.72, dirtTint);
            float3 tint = lerp(float3(1.0, 0.985, 0.90), float3(1.0, 0.72, 0.34), dirtTint);
            light += tint * (power * falloff * diffuse * dirtTransmission * 1.22);
        }
    }

    return light;
}

float LocalLampLight(float3 worldPos, float3 worldN, float time)
{
    return dot(LocalLampLightColor(worldPos, worldN, time), float3(0.299, 0.587, 0.114));
}
)"
