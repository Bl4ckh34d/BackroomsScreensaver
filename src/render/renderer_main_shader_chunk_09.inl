R"(
{
    return (gHorror0.x > 0.01 && materialId < 2.5) ? 1.0 : 0.0;
}

float LampDamageAtWorld(float2 worldXZ)
{
    int2 tile = (int2)floor((worldXZ - gMaze0.xy) / gMaze0.zw);
    if (tile.x < 0 || tile.y < 0 || tile.x >= (int)gMaze1.x || tile.y >= (int)gMaze1.y)
    {
        return 0.0;
    }
    return gLampDamage.Load(int3(tile, 0)).r;
}

float LampFailureMultiplier(float damage, float seed, float time)
{
    if (damage >= 0.995)
    {
        return 0.0;
    }
    float fail = smoothstep(0.06, 0.96, damage);
    float tickRate = lerp(2.2, 48.0, fail);
    float tick = floor(time * tickRate + seed * 19.0);
    float eventChance = lerp(0.98, 0.48, fail);
    float dropoutEvent = step(eventChance, Hash21(float2(seed * 37.0 + tick, seed * 11.0 + 211.0)));
    float violent = 0.22 + 1.38 * saturate(sin(time * (42.0 + seed * 91.0)) * 0.5 + 0.5);
    float pulse = lerp(1.0, violent, fail);
    pulse *= lerp(1.0, 0.035, dropoutEvent * fail);
    float brownout = lerp(1.0, 0.24, fail * fail);
    return max(0.0, pulse * brownout);
}

struct HSConstData
{
    float edges[3] : SV_TessFactor;
    float inside : SV_InsideTessFactor;
};

HSConstData HSPatchConstants(InputPatch<VSOut, 3> patch, uint patchId : SV_PrimitiveID)
{
    HSConstData o;
    float materialId = MaterialId(patch[0].material);
    float eligible = FleshMaterialEligible(materialId);
    float maxTess = lerp(28.0, 64.0, saturate(gHorror0.w / 0.22));
    float farTess = lerp(6.0, 18.0, saturate(gHorror0.w / 0.22));
    float flesh = saturate(gHorror0.x);
    float e01Dist = length((patch[0].worldPos + patch[1].worldPos) * 0.5 - gCameraPosTime.xyz);
    float e12Dist = length((patch[1].worldPos + patch[2].worldPos) * 0.5 - gCameraPosTime.xyz);
    float e20Dist = length((patch[2].worldPos + patch[0].worldPos) * 0.5 - gCameraPosTime.xyz);
    float e01 = max(1.0, floor(lerp(farTess, maxTess, 1.0 - smoothstep(10.0, 30.0, e01Dist)) * flesh + 0.5));
    float e12 = max(1.0, floor(lerp(farTess, maxTess, 1.0 - smoothstep(10.0, 30.0, e12Dist)) * flesh + 0.5));
    float e20 = max(1.0, floor(lerp(farTess, maxTess, 1.0 - smoothstep(10.0, 30.0, e20Dist)) * flesh + 0.5));
    o.edges[0] = eligible > 0.5 ? e12 : 1.0;
    o.edges[1] = eligible > 0.5 ? e20 : 1.0;
    o.edges[2] = eligible > 0.5 ? e01 : 1.0;
    o.inside = eligible > 0.5 ? max(o.edges[0], max(o.edges[1], o.edges[2])) : 1.0;
    return o;
}

[domain("tri")]
[partitioning("integer")]
[outputtopology("triangle_cw")]
[outputcontrolpoints(3)]
[patchconstantfunc("HSPatchConstants")]
VSOut HSMain(InputPatch<VSOut, 3> patch, uint i : SV_OutputControlPointID, uint patchId : SV_PrimitiveID)
{
    return patch[i];
}

[domain("tri")]
VSOut DSMain(HSConstData input, const OutputPatch<VSOut, 3> patch, float3 bary : SV_DomainLocation)
{
    VSOut o;
    float3 worldPos = patch[0].worldPos * bary.x + patch[1].worldPos * bary.y + patch[2].worldPos * bary.z;
    float3 normal = normalize(patch[0].normal * bary.x + patch[1].normal * bary.y + patch[2].normal * bary.z);
    float3 tangent = normalize(patch[0].tangent * bary.x + patch[1].tangent * bary.y + patch[2].tangent * bary.z);
    float2 rawUv = patch[0].uv * bary.x + patch[1].uv * bary.y + patch[2].uv * bary.z;
    float material = patch[0].material;
    float materialId = MaterialId(material);
    float eligible = FleshMaterialEligible(materialId);
    if (eligible > 0.5)
    {
        float time = gCameraPosTime.w;
        float flesh = saturate(gHorror0.x);
        float2 fleshUv = rawUv * 0.72 + float2(Hash21(floor(worldPos.xz * 0.27)), Hash21(floor(worldPos.zx * 0.31))) * 0.23;
        fleshUv += float2(sin(time * 0.18 + worldPos.z * 0.19), cos(time * 0.14 + worldPos.x * 0.17)) * 0.012 * flesh;
        float height = gNormalHeight.SampleLevel(gSampler, float3(fleshUv, 15.0), 0).a;
        float lowHeight = gNormalHeight.SampleLevel(gSampler, float3(fleshUv, 15.0), 4).a;
        float ridge = (height - lowHeight) * 2.65;
        float foldRelief = ridge + (lowHeight - 0.50) * 0.70;
        float pulse = 0.78 + 0.22 * sin(time * 3.2 + dot(worldPos, float3(0.73, 1.31, 0.51)));
        float crawl = sin(time * 2.1 + rawUv.x * 9.0 + rawUv.y * 6.0 + Hash21(floor(worldPos.xz * 0.5)) * 6.2831853) * 0.10;
        float suction = smoothstep(0.34, 0.02, height) * -0.16;
        float wallEdgeFade = 1.0;
        if (abs(normal.y) < 0.35)
)"
