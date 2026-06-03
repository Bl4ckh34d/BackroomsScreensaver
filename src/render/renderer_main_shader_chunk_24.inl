R"(
                    (1.0 - smoothstep(0.888 - lateralNoise, 0.982 - lateralNoise, u));
                float waterFarStart = 0.68 + farEdgeNoise * 0.24;
                float waterFarEnd = 0.96 + farEdgeNoise * 0.16;
                float waterFarFade = 1.0 - smoothstep(waterFarStart, waterFarEnd, away);
                float waterNearFade = smoothstep(-0.22 + farEdgeNoise * 0.12, -0.035 + farEdgeNoise * 0.10, away);
                float raggedCardFade = lateral * waterFarFade * waterNearFade;
                float bloodFarFade = 1.0 - smoothstep(0.90 + farEdgeNoise * 0.18, 1.10 + farEdgeNoise * 0.18, away);
                float bloodNearFade = smoothstep(-0.20 + farEdgeNoise * 0.10, -0.030 + farEdgeNoise * 0.09, away);
                float bloodCardFade = lateral * bloodFarFade * bloodNearFade;
                float floorCardFade = lerp(bloodCardFade, raggedCardFade, waterLiquid);
                float organicEdgeNoise = (Fbm3(float3(input.worldPos.xz * 2.15 + seed * 13.0, seed * 47.0)) - 0.5) * 0.22 +
                    (Noise3(float3(input.worldPos.xz * 8.0 + seed * 19.0, seed * 71.0)) - 0.5) * 0.070;
                float sideAbs = abs(u - 0.5) * 2.0;
                float forwardSoftEdge = 1.0 - smoothstep(0.76 + organicEdgeNoise, 1.10 + organicEdgeNoise, away);
                float sideSoftEdge = 1.0 - smoothstep(lerp(0.98, 0.70, saturate(away)) + organicEdgeNoise * 0.55,
                    1.13 + organicEdgeNoise * 0.40, sideAbs);
                float openBack = wallLeakSurface > 0.5 ? 1.0 : smoothstep(-0.08 + organicEdgeNoise, 0.05 + organicEdgeNoise, away);
                float organicFloorFootprint = saturate(forwardSoftEdge * sideSoftEdge * openBack);
                floorCardFade *= wallLeakSurface > 0.5 ? organicFloorFootprint : saturate(organicFloorFootprint + 0.35);
                float seam = 1.0 - smoothstep(0.0, 0.040, away);
                float floodNoise = 0.70 + 0.30 * Fbm3(float3(input.worldPos.xz * 2.7 + seed * 9.0, seed * 31.0));
                float floorFrontNoise = (Fbm3(float3(u * 3.1 + seed * 17.0, away * 1.9, seed * 53.0)) - 0.5) * 0.20 +
                    (Noise3(float3(u * 10.0, away * 5.5, seed * 79.0)) - 0.5) * 0.055;
                float lateFeatherStart = lerp(14.0, 10.0, waterLiquid);
                float lateFeatherEnd = lerp(34.0, 52.0, waterLiquid);
                float lateFeather = smoothstep(lateFeatherStart, lateFeatherEnd, leakAge) *
                    smoothstep(0.22, 0.86, pooledField + wetRim * 0.70) *
                    smoothstep(0.56, 0.91, floodNoise + floorFrontNoise * 0.55) *
                    (1.0 - smoothstep(lerp(0.74, 0.88, waterLiquid), lerp(1.16, 1.42, waterLiquid), away)) *
                    lateral * lerp(0.22, 0.38, waterLiquid);
                lateFeather *= lerp(1.0, 0.12, wallLeakSurface * waterLiquid);
                merged = saturate(merged + lateFeather);
                merged *= floorCardFade;
                wetRim *= floorCardFade;
                earlySoakField *= floorCardFade;
                lobeThickness *= floorCardFade;
                float soakNoise = 0.78 + 0.22 * Hash21(float2(floor(input.worldPos.x * 7.0) + seed * 41.0, floor(input.worldPos.z * 7.0)));
                float fibers = 0.82 + 0.18 * Hash21(float2(floor(input.worldPos.x * 9.0) + seed * 47.0, floor(input.worldPos.z * 9.0)));
                if (highBloodDetail)
                {
                    soakNoise = 0.65 + 0.35 * Fbm3(float3(input.worldPos.xz * 10.0, seed * 41.0));
                    fibers = 0.70 + 0.30 * Fbm3(float3(input.worldPos.xz * 11.0, seed * 41.0));
                }
                float soak = saturate(merged + earlySoakField * lerp(0.42, 0.68, waterLiquid)) *
                    lateral * lerp(0.22, 0.36, waterLiquid) * soakNoise;
                alpha = max(max(merged, soak * fibers), seam * lateral * wetRim * 0.75);
                alpha = smoothstep(lerp(0.024, 0.014, waterLiquid), lerp(0.112, 0.088, waterLiquid), alpha);
                thickness = saturate(lobeThickness * 0.86 + soak * lerp(0.28, 0.48, waterLiquid) +
                    earlySoakField * lerp(0.070, 0.16, waterLiquid) + seam * wetRim * 0.46);
                drips = saturate(merged + seam * wetRim * 0.45);
                thickness *= alpha;
                drips *= alpha;
            }
        }


        else
        {
            if (canvasSurface > 0.5)
            {
                float2 cuv = frac(rawUv);
                float code = round(rawUv.x - cuv.x);
                float neighborCode = round(rawUv.y - cuv.y);
                float centerCode = step(15.5, code);
                float sideCode = code - centerCode * 16.0;
                float edgeContinue0 = fmod(floor(neighborCode / 1.0), 2.0);
                float edgeContinue1 = fmod(floor(neighborCode / 2.0), 2.0);
                float edgeContinue2 = fmod(floor(neighborCode / 4.0), 2.0);
                float edgeContinue3 = fmod(floor(neighborCode / 8.0), 2.0);
                float sideActive0 = fmod(floor(sideCode / 1.0), 2.0);
                float sideActive1 = fmod(floor(sideCode / 2.0), 2.0);
                float sideActive2 = fmod(floor(sideCode / 4.0), 2.0);
                float sideActive3 = fmod(floor(sideCode / 8.0), 2.0);
                float topSource = 0.0;
                float topField = 0.0;
                float topThickness = 0.0;
                float topFiniteReachField = 0.0;
                float sideActives[4] = {sideActive0, sideActive1, sideActive2, sideActive3};
                [loop]
                for (int sideIndex = 0; sideIndex < 4; ++sideIndex)
                {
                    float sideActive = sideActives[sideIndex];
                    if (sideActive < 0.5) continue;
                    float sideU = sideIndex < 2 ? cuv.x : cuv.y;
                    float away = sideIndex == 0 ? cuv.y : (sideIndex == 1 ? 1.0 - cuv.y : (sideIndex == 2 ? cuv.x : 1.0 - cuv.x));
                    float alongMeters = sideIndex < 2 ? bloodUvMeters.x : bloodUvMeters.y;
                    float awayMeters = sideIndex < 2 ? bloodUvMeters.y : bloodUvMeters.x;
                    float sideSalt = sideIndex < 2 ? 0.0 : 0.347;
                    [loop]
                    for (int i = 0; i < 18; ++i)
                    {
                        float fi = (float)i;
                        if (fi >= canvasStreamCount) break;
                        float r0 = Hash21(float2(seed * 47.0 + fi + sideSalt, 3.0));
                        float r1 = Hash21(float2(seed * 31.0 + sideSalt, fi + 5.0));
)"
