R"(
                                      sin(angle + Hash21(float2(seed, 14.9)) * 1.65)));
    float satellites = 0.0;
    float tinyMist = 0.0;
    [loop]
    for (int i = 0; i < 76; ++i)
    {
        float fi = (float)i;
        float chooseDir = Hash21(float2(seed * 29.0 + fi, 11.0));
        float a = fi * 2.39996 + seed * 8.3 + Hash21(float2(fi, seed * 17.0)) * 1.1;
        float2 radialDir = float2(cos(a), sin(a));
        float2 dir = normalize(lerp(radialDir, sprayDir, step(0.54, chooseDir) * (0.30 + Hash21(float2(fi, seed * 43.0)) * 0.55)));
        float r = 0.10 + pow(Hash21(float2(seed * 23.0 + fi, 3.0)), 0.52) * 1.02;
        float2 c = dir * r;
        c += (float2(Hash21(float2(fi, seed * 41.0)), Hash21(float2(seed * 53.0, fi))) - 0.5) * 0.22;
        float2 p = local - c;
        float dropletRand = Hash21(c + seed);
        float sizeRand = pow(dropletRand, 2.65);
        float dotRadius = 0.0038 + sizeRand * 0.040;
        dotRadius *= lerp(0.62, 1.0, step(0.88, Hash21(float2(fi, seed + 88.0))));
        float stretch = 0.78 + Hash21(c + 12.0) * 1.18;
        stretch = lerp(stretch, 1.65 + Hash21(c + 51.0) * 2.10, step(0.74, chooseDir));
        float2 pr = Rotate2(p, atan2(dir.y, dir.x));
        pr.x /= stretch;
        float spot = exp(-dot(pr, pr) / max(0.00012, dotRadius * dotRadius));
        float breakup = smoothstep(0.22, 0.62, Fbm3(float3(pr * 28.0, seed * 14.0 + fi)));
        float satellite = spot * lerp(0.36, 0.92, breakup);
        satellites = max(satellites, satellite);
        tinyMist = max(tinyMist, spot * (1.0 - smoothstep(0.011, 0.046, dotRadius)) * lerp(0.46, 1.0, breakup));
    }

    float streaks = 0.0;
    [loop]
    for (int k = 0; k < 16; ++k)
    {
        float fk = (float)k;
        float armAngle = atan2(sprayDir.y, sprayDir.x) + (Hash21(float2(fk, seed * 9.0)) - 0.5) * (1.05 + Hash21(float2(seed, fk + 3.0)) * 1.7);
        float2 dir = float2(cos(armAngle), sin(armAngle));
        float2 origin = (float2(Hash21(float2(seed * 13.0, fk)), Hash21(float2(fk, seed * 19.0))) - 0.5) * 0.28;
        float2 p = local - origin;
        float along = dot(p, dir);
        float across = abs(dot(local, float2(-dir.y, dir.x)));
        float len = 0.22 + Hash21(float2(seed + 4.0, fk)) * 0.72;
        float width = 0.0035 + Hash21(float2(seed * 7.0, fk + 5.0)) * 0.013;
        float gate = smoothstep(0.010, 0.060, along) * (1.0 - smoothstep(len, len + 0.11, along));
        float strand = exp(-(across * across) / max(0.00005, width * width)) * gate;
        float breakup = smoothstep(0.20, 0.68, Fbm3(float3(p * (12.0 + fk), seed * 19.0 + fk)));
        float2 tip = p - dir * len;
        float tipDrop = exp(-dot(tip, tip) / max(0.00018, width * width * 7.5));
        streaks = max(streaks, max(strand * breakup, tipDrop * 0.76));
    }

    [loop]
    for (int j = 0; j < 18; ++j)
    {
        float fj = (float)j;
        float x = 0.14 + Hash21(float2(seed * 41.0 + fj, 91.0)) * 0.72;
        float top = 0.13 + Hash21(float2(seed * 13.0, fj)) * 0.42;
        float len = 0.16 + Hash21(float2(seed * 7.0, fj + 2.0)) * 0.76;
        float width = 0.0025 + Hash21(float2(seed * 31.0, fj + 4.0)) * 0.014;
        float yRel = uv.y - top;
        float fall = smoothstep(0.0, 0.026, yRel) * (1.0 - smoothstep(len, len + 0.075, yRel));
        float wander = (Fbm3(float3(fj * 3.1, yRel * 6.8, seed * 18.0)) - 0.5) * 0.060;
        float taper = lerp(0.80, 0.20, saturate(yRel / max(0.001, len)));
        float xDelta = uv.x - x - wander;
        float trail = exp(-(xDelta * xDelta) / max(0.00002, width * width * taper * taper * 5.5)) * fall;
        float broken = smoothstep(0.17, 0.56, Fbm3(float3(uv * 21.0 + fj, seed * 29.0)));
        float bead = exp(-((uv.x - x - wander) * (uv.x - x - wander)) / max(0.00015, width * width * 8.0)) *
                     exp(-((uv.y - (top + len)) * (uv.y - (top + len))) / max(0.00020, width * width * 38.0));
        float nose = max(bead * 1.38, trail * smoothstep(0.55, 1.0, saturate(yRel / max(0.001, len))));
        drips = max(drips, max(trail * broken, nose));
    }
    drips *= wallMask;

    float ceilingBlebs = ceilingMask * smoothstep(0.62, 0.88, Fbm3(float3(local * 13.0, seed * 33.0))) *
        (1.0 - smoothstep(0.24, 1.05, radial));
    float pepper = smoothstep(0.78, 0.965, Fbm3(float3(local * 73.0 + seed * 3.1, seed * 61.0))) *
        smoothstep(1.14, 0.20, radial);
    float merged = max(max(impact * 0.50, satellites * 0.96), max(streaks * 0.82, max(drips, max(ceilingBlebs * 0.54, max(tinyMist * 1.12, pepper * 0.70)))));
    float edgeNoise = Fbm3(float3(local * 13.0, seed * 39.0));
    float alpha = smoothstep(0.20, 0.48, merged + (edgeNoise - 0.5) * 0.16);
    float fullBorder = min(min(uv.x, 1.0 - uv.x), min(uv.y, 1.0 - uv.y));
    float wallBorder = min(min(uv.x, 1.0 - uv.x), uv.y);
    float border = lerp(fullBorder, wallBorder, wallMask);
    alpha *= smoothstep(0.006, 0.052 + edgeNoise * 0.035, border);
    thickness = saturate(impact * 0.24 + satellites * 0.42 + tinyMist * 0.22 + pepper * 0.10 + streaks * 0.28 + drips * 0.96 + ceilingBlebs * 0.34 + alpha * 0.32);
    return alpha;
}

float CenterSeepPool(float2 uv, float3 worldPos, float seed, float age, float speed, float maxRadius, out float thickness)
{
    float grow = smoothstep(0.0, 1.0, saturate(age * speed));
    float2 p = uv * 2.0 - 1.0;
    float darkCore = 0.0;
    float soakedLayer = 0.0;
    float sourceField = 0.0;
)"
