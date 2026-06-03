R"(
                    float reachFloor = smoothstep(0.83, 0.98, len) * streamStrength;
                    float contact = reachFloor;
                    if (wallLeakSurface > 0.5)
                    {
                        float waterFloorHit = smoothstep(0.88, 1.00, len) * smoothstep(5.8, 9.8, flowAge);
                        float bloodFloorHit = smoothstep(0.985, 1.075, len) * smoothstep(12.0, 17.0, flowAge);
                        contact = lerp(bloodFloorHit, waterFloorHit, waterLiquid) * streamStrength;
                    }
                    float poolDelay = wallLeakSurface > 0.5
                        ? lerp(11.8 + r2 * 4.8, 3.8 + r2 * 2.4, waterLiquid)
                        : 4.8;
                    float extraDelay = r2 * (wallLeakSurface > 0.5 ? lerp(1.3, 0.45, waterLiquid) : 1.9);
                    float poolAge = max(0.0, flowAge - poolDelay - extraDelay);
                    float poolGrowRate = wallLeakSurface > 0.5 ? (0.034 + r1 * 0.030) : (0.112 + r1 * 0.056);
                    poolGrowRate *= wallLeakSurface > 0.5 ? lerp(1.0, 1.42, waterLiquid) : lerp(1.0, 0.56, waterLiquid);
                    float poolGrow = smoothstep(0.0, 1.0, saturate(poolAge * poolGrowRate)) * contact;
                    float leanAtFloor = (r1 - 0.5) * (0.0025 + r2 * 0.0035);
                    float bottomWobble = 0.0;
                    if (wallLeakSurface > 0.5)
                    {
                        bottomWobble = (Hash21(float2(fi * 5.1 + seed * 23.0, 3.0)) - 0.5) * (0.0007 + r2 * 0.0010);
                        if (highBloodDetail)
                        {
                            bottomWobble = (Fbm3(float3(2.2, fi * 1.7, seed * 23.0)) - 0.5) * (0.0011 + r2 * 0.0018);
                        }
                    }
                    float sourceU = center + leanAtFloor + bottomWobble;
                    float edgeNoise = Hash21(float2(floor(u * 12.0) + fi, floor(away * 10.0) + seed * 23.0)) - 0.5;
                    if (highBloodDetail)
                    {
                        edgeNoise = Fbm3(float3(u * 7.5 + fi, away * 4.0, seed * 23.0)) - 0.5;
                    }
                    float spreadAway = wallLeakSurface > 0.5
                        ? (0.070 + poolGrow * (1.02 + r2 * 0.58) + edgeNoise * 0.046)
                        : (0.070 + poolGrow * (0.78 + r2 * 0.48) + edgeNoise * 0.040);
                    float spreadSide = (wallLeakSurface > 0.5
                        ? (0.032 + poolGrow * (0.155 + r1 * 0.210))
                        : (0.030 + poolGrow * (0.115 + r1 * 0.180))) * lerp(0.78, 1.0, streamWidthScale);
                    spreadAway *= lerp(1.0, 1.34, waterLiquid);
                    spreadSide *= lerp(1.0, 1.18, waterLiquid);
                    float wallWaterPool = wallLeakSurface * waterLiquid;
                    spreadAway *= lerp(1.0, 0.34 + r2 * 0.10, wallWaterPool);
                    spreadSide *= lerp(1.0, 0.42 + r1 * 0.12, wallWaterPool);
                    float sideWorld = (u - sourceU) * bloodUvMeters.x;
                    float awayWorld = away * bloodUvMeters.y;
                    float soakRag = Fbm3(float3(input.worldPos.xz * (7.0 + r2 * 7.0) + fi * 0.37, seed * 31.0 + fi));
                    if (wallLeakSurface > 0.5)
                    {
                        float soakPoolDelay = lerp(9.4 + r2 * 3.8, 3.1 + r2 * 2.0, waterLiquid);
                        float soakPoolAge = max(0.0, flowAge - soakPoolDelay);
                        float waterBottomReached = smoothstep(0.86, 0.99, len) * smoothstep(4.8, 8.8, flowAge);
                        float bloodBottomReached = smoothstep(0.965, 1.055, len) * smoothstep(10.5, 15.5, flowAge);
                        float bottomReached = lerp(bloodBottomReached, waterBottomReached, waterLiquid);
                        float soakGrowRate = (0.038 + r1 * 0.032) * lerp(1.0, 1.30, waterLiquid);
                        float soakGrow = smoothstep(0.0, 1.0, saturate(soakPoolAge * soakGrowRate)) *
                            bottomReached * streamStrength;
                        float soakSpreadAway = 0.055 + soakGrow * (0.72 + r2 * 0.42) + edgeNoise * 0.042;
                        float soakSpreadSide = (0.030 + soakGrow * (0.170 + r1 * 0.150)) * lerp(0.82, 1.0, streamWidthScale);
                        soakSpreadAway *= lerp(1.0, 1.42, waterLiquid);
                        soakSpreadSide *= lerp(1.0, 1.18, waterLiquid);
                        float soakSideWorld = max(0.016, soakSpreadSide * bloodUvMeters.x);
                        float soakAwayWorld = max(0.018, soakSpreadAway * bloodUvMeters.x);
                        float2 soakQ = float2(sideWorld / soakSideWorld, awayWorld / soakAwayWorld);
                        float soakBreakup = (soakRag - 0.5) * (0.28 + soakGrow * 0.34);
                        float soakLayer = 1.0 - smoothstep(0.72, 1.14, dot(soakQ, soakQ) + soakBreakup);
                        soakLayer *= smoothstep(0.0, 0.16, soakGrow);
                        earlySoakField = max(earlySoakField, soakLayer * (0.24 + soakGrow * 0.18));
                    }
                    float spreadSideWorld = max(0.010, spreadSide * bloodUvMeters.x);
                    float spreadAwayWorld = max(0.012, spreadAway * bloodUvMeters.x);
                    float2 q = float2(sideWorld / spreadSideWorld, awayWorld / spreadAwayWorld);
                    float edgeBreakup = (soakRag - 0.5) * (0.34 + poolGrow * 0.42);
                    float lobe = 1.0 - smoothstep(0.70, 1.05, dot(q, q) + edgeBreakup);
                    lobe *= smoothstep(0.0, 0.14, poolGrow);
                    float feeder = exp(-(sideWorld * sideWorld) / max(0.000035, spreadSideWorld * spreadSideWorld * 0.22)) *
                        (1.0 - smoothstep(0.0, (0.135 + poolGrow * 0.075) * bloodUvMeters.x, awayWorld)) * contact;
                    float capillary = smoothstep(0.62, 0.92,
                        Fbm3(float3(input.worldPos.xz * (18.0 + r1 * 12.0) + fi, seed * 57.0))) *
                        (1.0 - smoothstep(0.72, 1.35, dot(q, q))) * smoothstep(0.12, 0.85, poolGrow) * contact * 0.22;
                    lobe = max(lobe, capillary);
                    pooled = max(pooled, lobe);
                    pooledField += saturate(lobe) * (0.62 + poolGrow * 0.32) + feeder * 0.22;
                    wetRim = max(wetRim, feeder);
                    lobeThickness = max(lobeThickness, lobe * (0.64 + poolGrow * 0.54) + feeder * 0.52);
                }
                float sdfMerge = smoothstep(0.42, 1.18, pooledField + wetRim * 0.40);
                float merged = saturate(max(max(pooled, sdfMerge), earlySoakField * 0.58) + wetRim * 0.72);
                float lateralNoise = (Hash21(float2(floor(u * 11.0) + seed * 11.0, floor(away * 8.0))) - 0.5) * 0.024;
                if (highBloodDetail)
                {
                    lateralNoise = (Fbm3(float3(u * 6.1, away * 3.7, seed * 11.0)) - 0.5) * 0.030;
                }
                float farEdgeNoise = (Fbm3(float3(u * 4.7 + seed * 23.0, away * 2.1, seed * 67.0)) - 0.5) * 0.16 +
                    (Noise3(float3(u * 13.0, away * 8.0, seed * 97.0)) - 0.5) * 0.045;
                float lateral = smoothstep(0.018 + lateralNoise, 0.112 + lateralNoise, u) *
)"
