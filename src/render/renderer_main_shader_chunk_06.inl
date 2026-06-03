R"(
        lerp(0.30, 1.10, levelDirt) * (0.55 + hotspot * 0.65));

    float stainAlpha = min(stain * lerp(0.055, 0.115, levelDirt) * hotspotGain, 0.145);
    float grimeAlpha = min(grime * lerp(0.045, 0.098, levelDirt) * (0.82 + hotspot * 0.55), 0.125);
    float speckleAlpha = min(speckle * lerp(0.038, 0.090, levelDirt) * (0.74 + hotspot * 0.40), 0.095);
    float streakAlpha = min((wallStreak + floorWear * 0.72 + ceilingBloom * 0.82) * lerp(0.045, 0.105, levelDirt) *
        (0.80 + hotspot * 0.62), 0.120);
    float alpha = min(stainAlpha + grimeAlpha + speckleAlpha + streakAlpha, lerp(0.14, 0.25, levelDirt));
    float influence = saturate((stain * 0.70 + grime * 0.50 + speckle * 0.46 + wallStreak * 0.74 + floorWear * 0.46 + ceilingBloom * 0.58) *
        (0.70 + hotspot * 0.55 + levelDirt * 0.28));
    float3 wallTint = float3(0.145, 0.115, 0.045);
    float3 floorTint = float3(0.055, 0.044, 0.028);
    float3 ceilingTint = float3(0.17, 0.15, 0.075);
    float3 dirtTint = materialId < 0.5 ? wallTint : (materialId < 1.5 ? floorTint : ceilingTint);
    float3 dirtColor = base * (1.0 - (0.24 + influence * 0.48)) + dirtTint * (0.20 + influence * 0.24);
    base = lerp(base, dirtColor, alpha);
    float3 speckleTint = materialId < 1.5 ? float3(0.038, 0.030, 0.018) : float3(0.12, 0.105, 0.052);
    base = lerp(base, speckleTint, speckleAlpha * (0.30 + speckle * 0.46));
    float coldAlpha = min(coldLayer * lerp(0.036, 0.120, levelDirt), lerp(0.050, 0.135, levelDirt));
    float3 coldWallTint = float3(0.060, 0.096, 0.070);
    float3 coldFloorTint = float3(0.044, 0.050, 0.042);
    float3 coldCeilingTint = float3(0.074, 0.092, 0.068);
    float3 coldTint = materialId < 0.5 ? coldWallTint : (materialId < 1.5 ? coldFloorTint : coldCeilingTint);
    float3 coldColor = base * (0.74 - coldLayer * 0.18) + coldTint * (0.34 + coldLayer * 0.28);
    base = lerp(base, coldColor, coldAlpha * (1.0 - alpha * 0.42));
    roughness = saturate(roughness + alpha * (0.10 + influence * 0.08) + coldAlpha * (0.16 + coldLayer * 0.12));
    aoMap = saturate(aoMap - alpha * (0.09 + influence * 0.05) - coldAlpha * (0.12 + coldLayer * 0.08));
}

float SmokeFbm(float3 p, float seed, float time)
{
    p = p * 0.60 + float3(seed * 17.0, seed * 7.0, seed * 13.0);
    float v = Noise3(p);
    p *= 0.30;
    v = lerp(v, Noise3(p + time * float3(0.11, -0.31, 0.07)), 0.70);
    p *= 0.30;
    v = lerp(v, Noise3(p - time * float3(0.07, 0.23, 0.13)), 0.70);
    return v;
}

float BlackSmokeDensity(float3 p, float seed, float time)
{
    float3 uvw = p;
    float3 lmn = (p + 1.0) * 63.5;
    float t = time * (0.055 + seed * 0.010) + 32.0;

    float d2 = SmokeFbm(float3(0.6, 0.3, 0.6) * lmn + float3(0.0, 8.0 * t, 0.0), seed, time);
    float phase = d2 * 6.2831853;
    float d1 = SmokeFbm(0.3 * lmn + float3(0.0, 4.0 * t, 0.0) +
        5.0 * float3(cos(phase), 2.0 * d2, sin(phase)), seed + 0.37, time);
    d1 = pow(max(d1, 0.0001), lerp(4.0, 12.0, smoothstep(0.56, 1.0, Len2Inf(uvw.xz))));

    float density = 0.18 * smoothstep(0.0, 0.02, d1) +
        0.50 * smoothstep(0.02, 0.08, d1) +
        0.18 * smoothstep(0.08, 1.0, d1);
    float radialFade = 1.0 - smoothstep(0.68, 1.18, length(p.xz * float2(0.92, 1.04)));
    float verticalFade = smoothstep(-1.10, -0.74, p.y) * (1.0 - smoothstep(0.86, 1.18, p.y));
    float boxFade = 1.0 - smoothstep(0.90, 1.20, max(Len2Inf(p.xz), abs(p.y) * 0.82));
    return saturate(density * radialFade * verticalFade * boxFade);
}

float2 Rotate2(float2 p, float a)
{
    float c = cos(a);
    float s = sin(a);
    return float2(p.x * c - p.y * s, p.x * s + p.y * c);
}

float BloodShape(float2 uv, float3 worldPos, float3 normal, float seed, out float drips, out float thickness)
{
    drips = 0.0;
    thickness = 0.0;
    if (uv.x <= 0.001 || uv.x >= 0.999 || uv.y <= 0.001 || uv.y >= 0.999)
    {
        return 0.0;
    }

    float wallMask = saturate(1.0 - abs(normal.y));
    float ceilingMask = smoothstep(0.45, 0.90, -normal.y);
    float2 local = uv * 2.0 - 1.0;
    float angle = seed * 6.2831853 + Hash21(floor(worldPos.xz * 0.19 + seed * 17.0)) * 0.75;
    float2 q = Rotate2(local, angle);
    float3 volumeP = float3(q * (2.75 + seed * 1.20), seed * 21.7 + dot(worldPos, float3(0.071, 0.137, 0.093)));
    float largeNoise = Fbm3(volumeP * 1.95);
    float midNoise = Fbm3(volumeP * 6.40 + 9.1);
    float fineNoise = Fbm3(volumeP * 18.5 + 31.0);

    float2 axis = float2(0.74 + Hash21(float2(seed, 2.1)) * 0.30, 0.48 + Hash21(float2(seed, 5.7)) * 0.22);
    float radial = length(q / axis);
    float volumeField = 0.70 - radial * 1.55 + (largeNoise - 0.5) * 0.42 + (midNoise - 0.5) * 0.28;
    float impact = smoothstep(0.20, 0.48, volumeField) * (1.0 - smoothstep(0.15, 0.70, radial));
    float tornHoles = smoothstep(0.48, 0.78, fineNoise + radial * 0.30);
    impact *= lerp(0.72, 0.18, tornHoles);

    float2 sprayDir = normalize(float2(cos(angle + Hash21(float2(seed, 12.7)) * 1.65),
)"
