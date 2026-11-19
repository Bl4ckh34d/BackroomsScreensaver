R"(

float MazeFloorCeilingAO(float2 worldXZ, bool floorSide)
{
    const float strength = saturate(gAO0.x * gAO0.z);
    if (strength <= 0.0) return 0.0;

    const float2 cell = (worldXZ - gMaze0.xy) / gMaze0.zw;
    const int2 tile = (int2)floor(cell);
    if (any(tile < 0) || any(tile >= (int2)gMaze1.xy)) return 0.0;

    const float4 mask = gMazeOpen.Load(int3(tile, 0));
    const uint occluders = (uint)round((floorSide ? mask.g : mask.b) * 255.0);
    if (occluders == 0) return 0.0;

    const float radius = min(max(0.02, gAO0.y) * 1.20, min(gMaze0.z, gMaze0.w));
    const float2 local = cell - (float2)tile;
    static const float2 offsets[8] = {
        float2(-1, -1), float2(0, -1), float2(1, -1), float2(-1, 0),
        float2(1, 0), float2(-1, 1), float2(0, 1), float2(1, 1)
    };
    float nearestSquared = radius * radius;
    [unroll]
    for (uint i = 0; i < 8; ++i)
    {
        const float2 low = offsets[i];
        const float2 delta = max(max(low - local, local - (low + 1.0)), 0.0) * gMaze0.zw;
        if ((occluders & (1u << i)) != 0)
        {
            nearestSquared = min(nearestSquared, dot(delta, delta));
        }
    }
    return strength * (1.0 - smoothstep(0.0, radius, sqrt(nearestSquared)));
}

float CornerAO(float3 worldPos, float3 normal)
{
    float radius = max(0.02, gAO0.y);
    float floorContact = 1.0 - smoothstep(0.0, radius * 1.20, worldPos.y);
    float ceilingContact = 1.0 - smoothstep(0.0, radius * 1.20, gMaze1.z - worldPos.y);
    float verticalContact = max(floorContact, ceilingContact);
    float wallMask = saturate(1.0 - abs(normal.y));
    float ao = wallMask * verticalContact * gAO0.z;
    return saturate(ao * gAO0.x);
}

float FlashlightAmount(float3 worldPos, float3 worldN)
{
    if (gLighting0.x <= 0.001)
    {
        return 0.0;
    }

    float3 lightPos = gShadow0.xyz;
    float3 lightDir = normalize(gShadow1.xyz);
    float3 fromLight = worldPos - lightPos;
    float dist = length(fromLight);
    if (dist <= 0.001)
    {
        return 0.0;
    }

    float3 ray = fromLight / dist;
    float3 toLight = -ray;
    float cone = smoothstep(gShadow2.z, gShadow2.w, dot(ray, lightDir));
    float attenuation = 1.0 / (1.0 + gLighting0.y * dist * dist);
    float coneAttenuation = cone * attenuation * gLighting0.x;
    if (coneAttenuation <= 0.001)
    {
        return 0.0;
    }

    float diffuse = saturate(dot(worldN, toLight));
    float shadow = ShadowVisibility(worldPos, worldN);
    float4 lightClip = mul(float4(worldPos, 1.0), gLightViewProj);
    float2 lightUv = lightClip.xy / max(lightClip.w, 0.0001) * 0.5 + 0.5;
    float glass = gFlashlightPattern.Sample(gSampler, saturate(lightUv)).r;
    float2 centered = lightUv * 2.0 - 1.0;
    float lensFalloff = smoothstep(1.18, 0.18, dot(centered, centered));
    float pattern = lerp(0.52, 1.22, glass) * (0.72 + lensFalloff * 0.28);
    return coneAttenuation * (0.16 + diffuse * 1.12) * shadow * pattern;
}

float SparkLightOne(float3 worldPos, float3 worldN, float4 lightData)
{
    if (lightData.w <= 0.001)
    {
        return 0.0;
    }
    float3 L = lightData.xyz - worldPos;
    float d = length(L);
    if (d <= 0.001 || d > 5.0)
    {
        return 0.0;
    }
    float visibility = LampRayClear(worldPos.xz + worldN.xz * 0.05, lightData.xz);
    float3 Ln = L / d;
    float diffuse = saturate(dot(worldN, Ln) * 0.72 + 0.28);
    float falloff = pow(saturate(1.0 - d / 5.0), 2.1) / (1.0 + d * d * 0.42);
    return lightData.w * falloff * diffuse * visibility;
}

float SparkLight(float3 worldPos, float3 worldN)
{
    return SparkLightOne(worldPos, worldN, gSparkLight0) + SparkLightOne(worldPos, worldN, gSparkLight1);
}

float3 ExitSignLight(float3 worldPos, float3 worldN, float materialId)
{
    float strength = gExitLight0.w * (1.0 - saturate(gTransition0.z));
    float3 result = float3(0.0, 0.0, 0.0);
    if (strength > 0.001)
    {
        float3 L = gExitLight0.xyz - worldPos;
        float d = length(L);
        float signMaterial = step(6.5, materialId) * (1.0 - step(7.5, materialId));
        float signReach = lerp(2.40, 4.25, signMaterial);
        if (d > 0.001 && d <= signReach)
        {
            float visibility = LampRayClear(worldPos.xz + worldN.xz * 0.035, gExitLight0.xz);
            float3 Ln = L / d;
            float diffuse = saturate(dot(worldN, Ln) * 0.72 + 0.28);
            float falloff = pow(saturate(1.0 - d / signReach), 1.65) / (1.0 + d * d * 0.46);
            float surfaceSpill = lerp(0.44, 1.35, signMaterial);
            result += float3(0.045, 0.92, 0.34) * strength * falloff * diffuse * visibility * surfaceSpill;
        }
    }

    float portalEnabled = step(0.001, strength) * step(0.001, gExitLight3.w);
    if (portalEnabled > 0.001)
    {
        float3 portalDir = normalize(gExitLight1.xyz + float3(0.0001, 0.0, 0.0001));
        float3 portalRight = normalize(float3(portalDir.z, 0.0, -portalDir.x) + float3(0.0001, 0.0, 0.0001));
        float3 rel = worldPos - gExitLight3.xyz;
        float axial = dot(rel, portalDir);
        float lateral = abs(dot(rel, portalRight));
        float doorHalfW = max(0.12, gExitLight3.w);
        float roomDepth = smoothstep(-0.08, 0.16, axial) * (1.0 - smoothstep(2.80, 4.20, axial));
        float width = smoothstep(doorHalfW * 3.45, doorHalfW * 0.42, lateral);
        float height = smoothstep(0.05, 0.32, worldPos.y) * (1.0 - smoothstep(2.72, 3.08, worldPos.y));
)"
