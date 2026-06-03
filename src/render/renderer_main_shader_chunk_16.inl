R"(

    float doorFrameMaterial = step(9.5, materialId) * (1.0 - step(10.5, materialId));
    float whiteDoorFrameMaterial = step(20.5, materialId) * (1.0 - step(21.5, materialId));
    float frameBlock = max(doorFrameMaterial, whiteDoorFrameMaterial) *
        smoothstep(doorHalfW * 3.10, doorHalfW * 0.10, abs(lateral)) *
        smoothstep(0.62, 0.02, abs(axial)) *
        smoothstep(-0.08, 0.18, worldPos.y) *
        (1.0 - smoothstep(2.78, 3.04, worldPos.y));

    return saturate(doorOpen * max(wallBlock, frameBlock));
}

float3 ApplyPost(float3 color)
{
    color = max(color, 0.0);
    color = 1.0 - exp(-color * gPost0.x);
    color = pow(saturate(color), 1.0 / max(gPost0.y, 0.1));
    float danger = saturate(gPost0.z);
    float death = saturate(gPost0.w);
    float time = gCameraPosTime.w;
    float blinkRate = 5.0 + danger * 14.0 + death * 36.0;
    float blinkPhase = frac(time * blinkRate + sin(time * 17.13) * 0.17 + sin(time * 43.7) * 0.07);
    float blinkGate = step(0.78 - danger * 0.28 - death * 0.42, blinkPhase);
    float blackout = saturate(blinkGate * (danger * danger * 0.30 + death * 0.62));
    color *= 1.0 - blackout;
    color = lerp(color, float3(0.0, 0.0, 0.0), smoothstep(0.76, 1.0, death));
    color = lerp(color, float3(0.0, 0.0, 0.0), saturate(gTransition0.x));
    return color;
}

float DistanceFogBlock(float dist, float scale)
{
    float fog = saturate((dist - gFog0.x) / max(0.01, gFog0.y - gFog0.x));
    fog = 1.0 - exp(-fog * fog * 3.2);
    return saturate(fog * gFog0.z * scale);
}

float MonsterVicinityFog(float3 worldPos)
{
    float radius = max(gMonsterFog0.z, 0.001);
    float nearMonster = saturate(1.0 - length(worldPos.xz - gMonsterFog0.xy) / radius);
    return saturate(nearMonster * gMonsterFog0.w);
}

float SceneFogBlock(float dist, float3 worldPos, float scale)
{
    float distanceFog = DistanceFogBlock(dist, scale);
    float monsterFog = MonsterVicinityFog(worldPos);
    return saturate(distanceFog + monsterFog * (1.0 - distanceFog * 0.35));
}

void SelectBloodRevealSlot(float4 slot, float active, float2 worldXZ, float time, inout float bestMask, inout float bestAge)
{
    float enabled = saturate(active) * step(0.001, slot.w);
    if (enabled <= 0.001)
    {
        return;
    }
    float dist = length(worldXZ - slot.xy);
    float mask = 1.0 - smoothstep(slot.w * 0.72, slot.w, dist);
    float age = max(0.0, time - slot.z);
    float score = mask * smoothstep(0.0, 0.22, age) * enabled;
    if (score > bestMask)
    {
        bestMask = score;
        bestAge = age;
    }
}

float4 PSMain(VSOut input) : SV_TARGET
{
    float materialId = MaterialId(input.material);
    float3 cam = gCameraPosTime.xyz;
    float time = gCameraPosTime.w;
    float3 V = normalize(cam - input.worldPos);
    float3 N = normalize(input.normal);
    float3 T = normalize(input.tangent);
    float3 B = normalize(cross(N, T));
    float2 rawUv = input.uv;
    float2 uv = frac(rawUv);

    if (input.material > 27.05 && input.material < 27.95)
    {
        float seed = frac(input.material);
        float2 p = uv * 2.0 - 1.0;
        float contactRadius = lerp(0.28, 0.42, Hash21(float2(seed * 17.0, 3.0)));
        float spot = 1.0 - smoothstep(contactRadius * 0.58, contactRadius, length(p * float2(1.0, 1.06)));
        float dripChance = step(0.42, Hash21(float2(seed * 29.0, 7.0)));
        float dripLen = lerp(0.22, 0.92, Hash21(float2(seed * 41.0, 11.0))) * dripChance;
        float dripOffset = (Hash21(float2(seed * 53.0, 13.0)) - 0.5) * 0.20;
        float dripWidth = lerp(0.018, 0.040, Hash21(float2(seed * 67.0, 17.0)));
        float dripY = p.y + 0.10;
        float mainDrip = 1.0 - smoothstep(dripWidth, dripWidth * 2.6, abs(p.x - dripOffset));
        mainDrip *= smoothstep(0.02, 0.12, dripY) * (1.0 - smoothstep(dripLen, dripLen + 0.18, dripY));
        float drop = 1.0 - smoothstep(0.035, 0.095, length((p - float2(dripOffset, dripLen + 0.02)) * float2(0.88, 1.18)));
)"
