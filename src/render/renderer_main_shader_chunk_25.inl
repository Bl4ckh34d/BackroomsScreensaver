R"(
                        float r2 = Hash21(float2(fi + 9.0, seed * 71.0 + sideSalt));
                        float clusterCount = 2.0 + floor(Hash21(float2(seed * 83.0 + sideSalt, 19.0)) * 3.0);
                        float clusterId = floor(fmod(fi + floor(seed * 31.0 + sideSalt * 7.0), clusterCount));
                        float uniformCenter = 0.045 + ((fi + 0.20 + r0 * 0.60) / max(1.0, canvasStreamCount)) * 0.91;
                        float clusterCenter = 0.060 + Hash21(float2(seed * 89.0 + clusterId * 5.7 + sideSalt, 13.0)) * 0.88;
                        float sourceU = clamp(lerp(uniformCenter, clusterCenter + (r1 - 0.5) * 0.26, 0.66), 0.025, 0.975);
                        float flowAge = max(0.0, leakAge - r0 * lerp(8.5, 3.4, waterLiquid) - fi * 0.026);
                        float grow = smoothstep(0.0, 1.0, saturate(flowAge * lerp(0.024 + r1 * 0.022, 0.052 + r1 * 0.038, waterLiquid)));
                        float sideWorld = (sideU - sourceU) * alongMeters;
                        float awayWorld = away * awayMeters;
                        float awayContinue = sideIndex == 0 ? edgeContinue1 : (sideIndex == 1 ? edgeContinue0 : (sideIndex == 2 ? edgeContinue3 : edgeContinue2));
                        float sideContinue = sideIndex < 2 ? max(edgeContinue2, edgeContinue3) : max(edgeContinue0, edgeContinue1);
                        float reachNoise = (Fbm3(float3(input.worldPos.xz * (1.05 + r2 * 0.50) + fi * 0.11 + sideSalt, seed * 31.0)) - 0.5);
                        float terminalReach = (0.32 + r2 * 0.36 + awayContinue * 0.34) * awayMeters * lerp(0.94, 1.12, waterLiquid);
                        float channelHalf = (0.048 + r1 * 0.160 + grow * 0.195 + sideContinue * 0.045) * alongMeters;
                        float channel = exp(-(sideWorld * sideWorld) / max(0.00005, channelHalf * channelHalf));
                        float front = 1.0 - smoothstep(terminalReach + reachNoise * awayMeters * 0.22 - awayMeters * 0.11,
                            terminalReach + reachNoise * awayMeters * 0.22 + awayMeters * 0.19,
                            awayWorld);
                        topFiniteReachField = max(topFiniteReachField, channel * front * smoothstep(0.08, 0.52, grow));
                        float rag = Fbm3(float3(input.worldPos.xz * (4.8 + r2 * 5.0) + fi * 0.29 + sideSalt, seed * 43.0 + fi));
                        float fine = Noise3(float3(input.worldPos.xz * (17.0 + r1 * 10.0) + fi * 0.17, seed * 71.0 + sideSalt));
                        float sideSpread = (0.040 + grow * (0.36 + r1 * 0.46)) * alongMeters;
                        float awaySpread = (0.040 + grow * (0.76 + r2 * 0.62)) * awayMeters;
                        sideSpread *= lerp(0.98, 1.20, waterLiquid);
                        awaySpread *= lerp(1.0, 1.50, waterLiquid);
                        float skew = (Hash21(float2(seed * 19.0 + fi + sideSalt, 91.0)) - 0.5) * grow * 0.34;
                        float2 q = float2((sideWorld + awayWorld * skew) / max(0.032, sideSpread),
                            awayWorld / max(0.036, awaySpread));
                        float breakup = (rag - 0.5) * (0.40 + grow * 0.46) + (fine - 0.5) * 0.12;
                        float lobe = 1.0 - smoothstep(0.56, 1.03, dot(q, q) + breakup);
                        lobe *= smoothstep(0.018, 0.20, grow);
                        float rim = exp(-(sideWorld * sideWorld) / max(0.000035, sideSpread * sideSpread * 0.20)) *
                            (1.0 - smoothstep(0.0, (0.13 + grow * 0.11) * awayMeters, awayWorld)) *
                            smoothstep(0.0, 0.30, grow);
                        float capillary = smoothstep(0.58, 0.94,
                            Fbm3(float3(input.worldPos.xz * (18.0 + r1 * 10.0) + fi, seed * 61.0 + sideSalt))) *
                            (1.0 - smoothstep(0.74, 1.34, dot(q, q))) * grow * 0.20;
                        float source = max(lobe, capillary);
                        topSource = max(topSource, source);
                        topFiniteReachField = max(topFiniteReachField, source * (0.56 + grow * 0.34));
                        topField += saturate(source) * (0.50 + grow * 0.38) + rim * 0.26;
                        topThickness = max(topThickness, source * (0.48 + grow * 0.54) + rim * 0.34);
                    }
                }
                if (centerCode > 0.5)
                {
                    float centerThickness = 0.0;
                    float centerSpeed = lerp(0.024, 0.016, waterLiquid) * 0.70;
                    float centerRadius = lerp(0.64, 0.86, waterLiquid);
                    float centerSource = CenterSeepPool(cuv, input.worldPos, seed, leakAge, centerSpeed, centerRadius, centerThickness);
                    float organicMask = smoothstep(0.20, 0.68,
                        Fbm3(float3(input.worldPos.xz * 4.8 + seed * 7.0, seed * 37.0)) +
                        (Noise3(float3(input.worldPos.xz * 18.0 + seed * 5.0, seed * 83.0)) - 0.5) * 0.20);
                    topSource = max(topSource, centerSource * organicMask);
                    topField += centerSource * (0.72 + centerThickness * 0.30);
                    topThickness = max(topThickness, centerThickness);
                }
                float reachGrow = smoothstep(0.0, 1.0, saturate(leakAge * lerp(0.020, 0.036, waterLiquid)));
                float sourceReachMask = saturate(topFiniteReachField);
                if (centerCode > 0.5)
                {
                    float centerReach = IrregularPuddleSupport(cuv, input.worldPos, seed + 0.23, leakAge, waterLiquid) *
                        smoothstep(0.02, 0.58, reachGrow);
                    sourceReachMask = max(sourceReachMask, centerReach);
                }
                float edgeNoiseN = (Fbm3(float3(input.worldPos.x * 1.55 + seed * 11.0, input.worldPos.z * 0.31, seed * 23.0)) - 0.5) * 0.16;
                float edgeNoiseS = (Fbm3(float3(input.worldPos.x * 1.47 + seed * 17.0, input.worldPos.z * 0.35, seed * 29.0)) - 0.5) * 0.16;
                float edgeNoiseW = (Fbm3(float3(input.worldPos.z * 1.51 + seed * 13.0, input.worldPos.x * 0.33, seed * 31.0)) - 0.5) * 0.16;
                float edgeNoiseE = (Fbm3(float3(input.worldPos.z * 1.43 + seed * 19.0, input.worldPos.x * 0.37, seed * 37.0)) - 0.5) * 0.16;
                float edgeN = clamp(0.105 + edgeNoiseN, 0.020, 0.240);
                float edgeS = clamp(0.105 + edgeNoiseS, 0.020, 0.240);
                float edgeW = clamp(0.105 + edgeNoiseW, 0.020, 0.240);
                float edgeE = clamp(0.105 + edgeNoiseE, 0.020, 0.240);
                float fadeN = lerp(smoothstep(max(0.0, edgeN - 0.070), min(0.38, edgeN + 0.115), cuv.y), 1.0, edgeContinue0);
                float fadeS = lerp(smoothstep(max(0.0, edgeS - 0.070), min(0.38, edgeS + 0.115), 1.0 - cuv.y), 1.0, edgeContinue1);
                float fadeW = lerp(smoothstep(max(0.0, edgeW - 0.070), min(0.38, edgeW + 0.115), cuv.x), 1.0, edgeContinue2);
                float fadeE = lerp(smoothstep(max(0.0, edgeE - 0.070), min(0.38, edgeE + 0.115), 1.0 - cuv.x), 1.0, edgeContinue3);
                float tileEdgeFade = fadeN * fadeS * fadeW * fadeE;
                float organicMask = smoothstep(0.22, 0.72,
                    Fbm3(float3(input.worldPos.xz * 4.4 + seed * 11.0, seed * 47.0)) +
                    (Noise3(float3(input.worldPos.xz * 16.0 + seed * 13.0, seed * 79.0)) - 0.5) * 0.18);
                float merged = saturate(max(topSource, smoothstep(0.42, 1.18, topField)) * organicMask * sourceReachMask) * tileEdgeFade;
                if (waterLiquid > 0.5)
                {
                    float broadTopNoise = Fbm3(float3(input.worldPos.xz * 0.58 + seed * 23.0, seed * 37.0));
                    float midTopNoise = Fbm3(float3(input.worldPos.xz * 1.72 + seed * 11.0, seed * 53.0));
                    float fineTopNoise = Noise3(float3(input.worldPos.xz * 7.0 + seed * 17.0, seed * 67.0));
                    float ceilingShape = IrregularPuddleSupport(cuv, input.worldPos, seed + 0.61, leakAge, waterLiquid);
                    float ceilingDryAge = smoothstep(20.0, 84.0, leakAge);
                    float ceilingCoverage = smoothstep(0.20, 0.76,
                        broadTopNoise * 0.52 + midTopNoise * 0.34 + fineTopNoise * 0.14 +
                        ceilingShape * 0.30 + sourceReachMask * 0.26 - ceilingDryAge * 0.26);
                    float ceilingEdgeDissolve = smoothstep(0.03, 0.44, tileEdgeFade + broadTopNoise * 0.34 + midTopNoise * 0.16);
                    merged *= ceilingCoverage * ceilingEdgeDissolve;
)"
