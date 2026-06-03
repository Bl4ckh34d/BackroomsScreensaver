R"(
    [unroll]
    for (int i = 0; i < 3; ++i)
    {
        float fi = (float)i;
        float enabled = i == 0 ? 1.0 : step(i == 1 ? 0.54 : 0.82, Hash21(float2(seed * 61.0 + fi, 17.0)));
        float localGrow = grow * enabled;
        float2 offset = float2(0.0, 0.0);
        if (i > 0)
        {
            float a = seed * 6.2831853 + fi * 2.43 + Hash21(float2(seed * 71.0, fi + 19.0)) * 1.25;
            float r = 0.22 + Hash21(float2(seed * 73.0 + fi, 23.0)) * 0.26;
            offset = float2(cos(a), sin(a)) * r;
        }
        float2 q = p - offset;
        float angle = seed * 6.2831853 + Hash21(float2(seed * 41.0 + fi, 19.0)) * 1.2;
        q = Rotate2(q, angle);
        q *= float2(0.58 + Hash21(float2(seed * 47.0 + fi, 23.0)) * 0.58,
                    0.84 + Hash21(float2(seed * 53.0, fi + 29.0)) * 0.82);
        float broad = Fbm3(float3(worldPos.xz * (3.4 + seed * 1.1 + fi * 0.35) + seed * 11.0 + fi, seed * 31.0));
        float fine = Noise3(float3(worldPos.xz * (15.0 + seed * 4.0 + fi * 1.7) + seed * 7.0, seed * 71.0 + fi));
        float cellular = Fbm3(float3(worldPos.xz * (8.4 + fi * 1.5) + seed * 19.0, seed * 89.0 + fi));
        float angular = atan2(q.y, q.x);
        float lobedEdge = sin(angular * (2.0 + floor(Hash21(float2(seed * 97.0 + fi, 43.0)) * 4.0)) +
            seed * 9.1 + fi * 1.7) * (0.045 + localGrow * 0.145);
        lobedEdge += sin(angular * 5.0 - seed * 6.4 + fi) * (0.026 + localGrow * 0.065);
        lobedEdge += sin(q.x * (4.3 + fi * 0.7) + q.y * (2.8 + seed) + seed * 8.4) * (0.012 + localGrow * 0.045);
        float radial = length(q) + lobedEdge;
        float edgeNoise = (broad - 0.5) * (0.18 + localGrow * 0.22) + (fine - 0.5) * 0.055;
        float spotScale = i == 0
            ? (0.78 + Hash21(float2(seed * 79.0, fi + 31.0)) * 0.16)
            : (0.52 + Hash21(float2(seed * 79.0, fi + 31.0)) * 0.28);
        float radius = lerp(0.022, maxRadius * spotScale, localGrow);
        float coreEdge = (broad - 0.5) * 0.028 + (fine - 0.5) * 0.012;
        float coreGrow = smoothstep(0.0, 1.0, saturate(age * speed * (0.58 + fi * 0.08)));
        float coreNoise = (cellular - 0.5) * (0.020 + coreGrow * 0.035) + coreEdge;
        float core = 1.0 - smoothstep(radius * (0.070 + coreGrow * 0.040) + coreNoise,
            radius * (0.18 + coreGrow * 0.145) + coreNoise, radial);
        float front = 1.0 - smoothstep(radius * 0.42 + edgeNoise,
            radius * (1.08 + localGrow * 0.20) + edgeNoise, radial);
        float soakMask = smoothstep(0.20, 0.82, broad + (cellular - 0.5) * 0.44 + (fine - 0.5) * 0.14);
        float feather = smoothstep(0.03, 0.82, front);
        float capillary = smoothstep(0.64, 0.93, cellular + fine * 0.10) *
            (1.0 - smoothstep(radius * 0.70, radius + 0.48, radial + (broad - 0.5) * 0.12));
        darkCore = max(darkCore, core * enabled);
        sourceField = max(sourceField, front * enabled);
        soakedLayer = max(soakedLayer, max(feather * soakMask, capillary * 0.45) * enabled);
    }
    float fibers = smoothstep(0.46, 0.90, Fbm3(float3(worldPos.xz * 18.0 + seed * 13.0, seed * 79.0)));
    float dryBreak = smoothstep(0.58, 0.91, Fbm3(float3(worldPos.xz * 10.5 + seed * 23.0, seed * 101.0)));
    float mottledSoak = sourceField * (0.24 + soakedLayer * 0.36) + soakedLayer * (0.88 + fibers * 0.20);
    float sharedSoak = smoothstep(0.13, 0.64, mottledSoak);
    sharedSoak *= 1.0 - dryBreak * (0.28 + grow * 0.18);
    float shape = max(darkCore * 0.94, sharedSoak * (0.30 + fibers * 0.16));
    float border = min(min(uv.x, 1.0 - uv.x), min(uv.y, 1.0 - uv.y));
    shape *= smoothstep(0.004, 0.032, border) * smoothstep(0.02, 0.55, grow);
    thickness = saturate(darkCore * (0.78 + grow * 0.14) + sharedSoak * 0.12);
    return shape;
}

float IrregularPuddleSupport(float2 uv, float3 worldPos, float seed, float age, float waterLiquid)
{
    float2 p = uv * 2.0 - 1.0;
    float angle = seed * 6.2831853 + Hash21(float2(seed * 11.0, 7.0)) * 2.2;
    float2 q = Rotate2(p, angle);
    float2 aspect = float2(0.66 + Hash21(float2(seed * 13.0, 11.0)) * 0.34,
                           0.96 + Hash21(float2(seed * 17.0, 13.0)) * 0.68);
    q *= aspect;
    float theta = atan2(q.y, q.x);
    float broad = Fbm3(float3(worldPos.xz * (0.70 + seed * 0.17) + seed * 19.0, seed * 31.0));
    float mid = Fbm3(float3(worldPos.xz * (2.1 + seed * 0.31) + seed * 7.0, seed * 43.0));
    float fine = Noise3(float3(worldPos.xz * (7.5 + seed * 0.80) + seed * 13.0, seed * 59.0));
    float edge = sin(theta * (3.0 + floor(Hash21(float2(seed * 23.0, 17.0)) * 3.0)) + seed * 8.0) * 0.13;
    edge += sin(theta * 5.0 - seed * 5.7) * 0.070;
    edge += (broad - 0.5) * 0.26 + (mid - 0.5) * 0.12 + (fine - 0.5) * 0.040;
    float lobeField = 1.0 - smoothstep(0.58 + edge, 1.18 + edge * 0.35, length(q));
    float satellites = 0.0;
    [unroll]
    for (int i = 0; i < 4; ++i)
    {
        float fi = (float)i;
        float enabled = step(0.24, Hash21(float2(seed * 37.0 + fi, 29.0)));
        float a = seed * 6.2831853 + fi * 1.71 + Hash21(float2(seed * 41.0, fi + 31.0)) * 1.10;
        float r = 0.30 + Hash21(float2(seed * 43.0 + fi, 37.0)) * 0.44;
        float2 c = float2(cos(a), sin(a)) * r;
        float2 lq = Rotate2(p - c, a + 1.13);
        lq *= float2(0.74 + Hash21(float2(seed * 47.0, fi + 41.0)) * 0.42,
                     1.04 + Hash21(float2(seed * 53.0 + fi, 43.0)) * 0.62);
        float oval = 1.0 - smoothstep(0.30, 0.72, length(lq) + (mid - 0.5) * 0.16);
        satellites = max(satellites, oval * enabled * (0.45 + Hash21(float2(seed * 59.0, fi + 47.0)) * 0.42));
    }
    float feedAge = smoothstep(1.0, lerp(34.0, 24.0, waterLiquid), age);
    return saturate(max(lobeField, satellites * feedAge));
}

float FleshMaterialEligible(float materialId)
)"
