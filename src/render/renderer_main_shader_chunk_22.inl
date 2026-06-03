R"(
                merged *= tileEdgeFade;
                wetRim *= tileEdgeFade;
                soakField *= tileEdgeFade;
                if (waterLiquid > 0.5)
                {
                    float broadPuddleNoise = Fbm3(float3(input.worldPos.xz * 0.62 + seed * 19.0, seed * 31.0));
                    float midPuddleNoise = Fbm3(float3(input.worldPos.xz * 1.85 + seed * 7.0, seed * 43.0));
                    float finePuddleNoise = Noise3(float3(input.worldPos.xz * 7.5 + seed * 13.0, seed * 59.0));
                    float puddleShape = IrregularPuddleSupport(cuv, input.worldPos, seed + 0.49, leakAge, waterLiquid);
                    float drainageAge = smoothstep(18.0, 72.0, leakAge);
                    float downstreamCoverageGate = lerp(1.0, smoothstep(22.0, 58.0, leakAge), downstreamWater);
                    float organicCoverage = smoothstep(0.18, 0.74,
                        broadPuddleNoise * 0.54 + midPuddleNoise * 0.32 + finePuddleNoise * 0.14 +
                        puddleShape * 0.36 + sourceReachMask * 0.26 - drainageAge * 0.30) * downstreamCoverageGate;
                    float edgeDissolve = smoothstep(0.03, 0.42, tileEdgeFade + broadPuddleNoise * 0.34 + midPuddleNoise * 0.16);
                    float waterDecay = organicCoverage * edgeDissolve;
                    merged *= waterDecay;
                    wetRim *= lerp(0.42, 1.0, organicCoverage);
                    soakField *= lerp(0.12, 0.88, organicCoverage) * edgeDissolve;
                    lobeThickness *= lerp(0.58, 1.0, organicCoverage);
                }
                float soakNoise = highBloodDetail
                    ? (0.65 + 0.35 * Fbm3(float3(input.worldPos.xz * 10.0, seed * 41.0)))
                    : (0.78 + 0.22 * Hash21(float2(floor(input.worldPos.x * 7.0) + seed * 41.0, floor(input.worldPos.z * 7.0))));
                float fibers = highBloodDetail
                    ? (0.70 + 0.30 * Fbm3(float3(input.worldPos.xz * 11.0, seed * 41.0)))
                    : (0.82 + 0.18 * Hash21(float2(floor(input.worldPos.x * 9.0) + seed * 47.0, floor(input.worldPos.z * 9.0))));
                float soak = saturate(merged + soakField * lerp(0.48, 0.72, waterLiquid)) *
                    lerp(0.20, 0.38, waterLiquid) * soakNoise;
                alpha = max(max(merged, soak * fibers), wetRim * 0.82);
                alpha = smoothstep(lerp(0.020, 0.012, waterLiquid), lerp(0.112, 0.086, waterLiquid), alpha);
                thickness = saturate(lobeThickness * 0.84 + soak * lerp(0.26, 0.46, waterLiquid) + soakField * lerp(0.12, 0.22, waterLiquid));
                drips = saturate(merged + wetRim * 0.36);
                thickness *= alpha;
                drips *= alpha;
            }
            else if (centerSeepSurface > 0.5)
            {
                float centerThickness = 0.0;
                float centerSpeed = lerp(0.026, 0.017, waterLiquid) * lerp(1.0, 3.15, menuCenterSeepSurface);
                float centerRadius = lerp(0.56, 0.72, waterLiquid) * lerp(1.0, 1.18, menuCenterSeepSurface);
                float source = CenterSeepPool(bloodUv, input.worldPos, seed, leakAge, centerSpeed, centerRadius, centerThickness);
                alpha = source * floorMask * smoothstep(0.0, lerp(0.45, 0.82, waterLiquid) * lerp(1.0, 0.34, menuCenterSeepSurface), leakAge);
                alpha = smoothstep(lerp(0.020, 0.012, waterLiquid), lerp(0.118, 0.090, waterLiquid), alpha);
                thickness = centerThickness * alpha;
                drips = 0.0;
            }
            else
            {
                float u = bloodUv.x;
                float away = 1.0 - bloodUv.y;
                float pooled = 0.0;
                float pooledField = 0.0;
                float wetRim = 0.0;
                float earlySoakField = 0.0;
                float lobeThickness = 0.0;
                [loop]
                for (int i = 0; i < 32; ++i)
                {
                    float fi = (float)i;
                    if (fi >= floorStreamCount) break;
                    float r0 = Hash21(float2(seed * 47.0 + fi, 3.0));
                    float r1 = Hash21(float2(seed * 31.0, fi + 5.0));
                    float r2 = Hash21(float2(fi + 9.0, seed * 71.0));
                    float clusterCount = 2.0 + floor(Hash21(float2(seed * 83.0, 19.0)) * 3.0);
                    float clusterId = floor(fmod(fi + floor(seed * 31.0), clusterCount));
                    float uniformCenter = 0.055 + ((fi + 0.20 + r0 * 0.60) / max(1.0, streamCount)) * 0.89;
                    float clusterCenter = 0.08 + Hash21(float2(seed * 89.0 + clusterId * 5.7, 13.0)) * 0.84;
                    float clusterSpread = 0.025 + Hash21(float2(seed * 97.0 + clusterId, 29.0)) * 0.080;
                    float clusteredCenter = clusterCenter + (Hash21(float2(seed * 131.0 + fi, 37.0)) - 0.5) * clusterSpread;
                    float center = clamp(lerp(uniformCenter, clusteredCenter, 0.58 + Hash21(float2(seed * 151.0 + fi, 43.0)) * 0.34), 0.045, 0.955);
                    float densityBase = Hash21(float2(center * 17.0 + seed * 11.0, clusterId * 3.1 + 5.0));
                    if (highBloodDetail)
                    {
                        densityBase = smoothstep(0.24, 0.78, Fbm3(float3(center * 4.2, seed * 18.0, clusterId * 2.3)));
                    }
                    float densityBand = densityBase;
                    float streamStrength = lerp(0.24, 1.18, densityBand) * lerp(0.58, 1.12, Hash21(float2(seed * 173.0 + fi, 47.0)));
                    float streamDelay = wallLeakSurface > 0.5
                        ? (r0 * 3.4 + fi * (0.05 + r2 * 0.16) + Hash21(float2(seed * 251.0 + fi, 157.0)) * 1.1)
                        : (r0 * 2.2 + fi * 0.16);
                    float streamAge = max(0.0, leakAge - streamDelay);
                    float flowAge = streamAge;
                    if (wallLeakSurface > 0.5)
                    {
                        float speedPhase = streamAge * (0.62 + r2 * 0.54) + seed * 17.0 + fi * 2.13;
                        flowAge = max(0.0, streamAge * (0.90 + r1 * 0.18) +
                            sin(speedPhase) * (0.10 + r0 * 0.08) +
                            sin(speedPhase * 2.17 + r2 * 6.0) * 0.032);
                    }
                    float streamGrowRate = wallLeakSurface > 0.5
                        ? (0.074 + r1 * 0.062)
                        : (0.088 + r1 * 0.066);
                    float streamGrow = smoothstep(0.0, 1.0, saturate(flowAge * streamGrowRate));
                    float len = saturate(0.160 + streamGrow * (0.92 + r1 * 0.24));
)"
