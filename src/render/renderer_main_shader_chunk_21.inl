R"(
                        float terminalReach = (0.34 + r2 * 0.38 + awayContinue * 0.34) * awayMeters * lerp(0.96, 1.62, waterLiquid) * lerp(1.0, 0.54, downstreamWater);
                        float channelHalf = (0.050 + r1 * 0.170 + grow * 0.210 + sideContinue * 0.045) * alongMeters;
                        float channel = exp(-(sideWorld * sideWorld) / max(0.00005, channelHalf * channelHalf));
                        float front = 1.0 - smoothstep(terminalReach + reachNoise * awayMeters * 0.20 - awayMeters * 0.10,
                            terminalReach + reachNoise * awayMeters * 0.20 + awayMeters * 0.17,
                            awayWorld);
                        finiteReachField = max(finiteReachField, channel * front * smoothstep(0.08, 0.52, grow));
                        float rag = Fbm3(float3(input.worldPos.xz * (5.4 + r2 * 5.0) + fi * 0.31 + sideSalt, seed * 37.0 + fi));
                        float fine = Noise3(float3(input.worldPos.xz * (18.0 + r1 * 11.0) + fi * 0.17, seed * 61.0 + sideSalt));
                        float sideSpread = (0.040 + grow * (0.38 + r1 * 0.48)) * alongMeters;
                        float awaySpread = (0.055 + grow * (0.92 + r2 * 0.70)) * awayMeters;
                        sideSpread *= lerp(0.94, 0.74, waterLiquid) * lerp(1.0, 0.72, downstreamWater);
                        awaySpread *= lerp(0.92, 0.82, waterLiquid) * lerp(1.0, 0.62, downstreamWater);
                        float2 q = float2(sideWorld / max(0.035, sideSpread), awayWorld / max(0.040, awaySpread));
                        float breakup = (rag - 0.5) * (0.34 + grow * 0.44) + (fine - 0.5) * 0.10;
                        float lobe = 1.0 - smoothstep(0.62, 1.07, dot(q, q) + breakup);
                        lobe *= smoothstep(0.02, 0.22, grow);
                        float feeder = exp(-(sideWorld * sideWorld) / max(0.00004, sideSpread * sideSpread * 0.18)) *
                            (1.0 - smoothstep(0.0, (0.16 + grow * 0.10) * awayMeters, awayWorld)) *
                            smoothstep(0.0, 0.32, grow);
                        float capillary = smoothstep(0.58, 0.92,
                            Fbm3(float3(input.worldPos.xz * (16.0 + r1 * 10.0) + fi, seed * 57.0 + sideSalt))) *
                            (1.0 - smoothstep(0.78, 1.42, dot(q, q))) * grow * 0.24;
                        lobe = max(lobe, capillary);
                        pooled = max(pooled, lobe);
                        finiteReachField = max(finiteReachField, lobe * (0.58 + grow * 0.34));
                        pooledField += saturate(lobe) * (0.50 + grow * 0.42) + feeder * 0.24;
                        wetRim = max(wetRim, feeder);
                        lobeThickness = max(lobeThickness, lobe * (0.54 + grow * 0.64) + feeder * 0.46);
                        float soakSpreadSide = max(0.045, sideSpread * (1.22 + grow * 0.26));
                        float soakSpreadAway = max(0.050, awaySpread * (1.12 + grow * 0.42));
                        float2 sq = float2(sideWorld / soakSpreadSide, awayWorld / soakSpreadAway);
                        float soakBreak = (Fbm3(float3(input.worldPos.xz * (9.0 + r2 * 6.0) + fi, seed * 71.0 + sideSalt)) - 0.5) *
                            (0.40 + grow * 0.34);
                        soakField = max(soakField, (1.0 - smoothstep(0.72, 1.18, dot(sq, sq) + soakBreak)) * grow * 0.42);
                    }
                }
                float centerDelay = centerCode * downstreamCode * lerp(7.5, 3.8, waterLiquid);
                float centerLeakAge = max(0.0, leakAge - centerDelay);
                if (centerCode > 0.5)
                {
                    float centerThickness = 0.0;
                    float centerSpeed = lerp(0.026, 0.0075, waterLiquid) * 0.72;
                    float centerRadius = lerp(0.62, 1.06, waterLiquid);
                    float centerSource = CenterSeepPool(cuv, input.worldPos, seed, centerLeakAge, centerSpeed, centerRadius, centerThickness);
                    float noiseBloom = smoothstep(0.24, 0.70,
                        Fbm3(float3(input.worldPos.xz * 3.6 + seed * 13.0, seed * 47.0)) +
                        (Noise3(float3(input.worldPos.xz * 13.0 + seed * 19.0, seed * 71.0)) - 0.5) * 0.18);
                    pooled = max(pooled, centerSource * noiseBloom);
                    pooledField += centerSource * (0.80 + centerThickness * 0.35);
                    lobeThickness = max(lobeThickness, centerThickness);
                    soakField = max(soakField, centerSource * noiseBloom * 0.38);
                }
                float reachGrow = smoothstep(0.0, 1.0, saturate(centerLeakAge * lerp(0.024, 0.010, waterLiquid)));
                float sourceReachMask = saturate(finiteReachField);
                if (centerCode > 0.5)
                {
                    float centerReach = IrregularPuddleSupport(cuv, input.worldPos, seed + 0.17, centerLeakAge, waterLiquid) *
                        smoothstep(0.02, 0.58, reachGrow);
                    sourceReachMask = max(sourceReachMask, centerReach);
                }
                float merged = saturate(max(pooled, smoothstep(0.42, 1.18, pooledField + wetRim * 0.42)) + soakField * 0.62 + wetRim * 0.34);
                merged *= sourceReachMask;
                wetRim *= sourceReachMask;
                soakField *= sourceReachMask;
                if (waterLiquid > 0.5 && centerCode > 0.5)
                {
                    float centerSupport = IrregularPuddleSupport(cuv, input.worldPos, seed + 0.31, centerLeakAge, waterLiquid);
                    float channelSupport = saturate(finiteReachField + wetRim * 0.72 + soakField * 0.48);
                    float supportMix = max(centerSupport, channelSupport);
                    merged *= supportMix;
                    soakField *= lerp(0.22, 1.0, supportMix);
                    wetRim *= lerp(0.36, 1.0, supportMix);
                    lobeThickness *= lerp(0.45, 1.0, supportMix);
                }
                float edgeNoiseScale = lerp(0.13, 0.30, waterLiquid);
                float edgeNoiseN = (Fbm3(float3(input.worldPos.x * 1.75 + seed * 11.0, input.worldPos.z * 0.37, seed * 23.0)) - 0.5) * edgeNoiseScale;
                float edgeNoiseS = (Fbm3(float3(input.worldPos.x * 1.63 + seed * 17.0, input.worldPos.z * 0.41, seed * 29.0)) - 0.5) * edgeNoiseScale;
                float edgeNoiseW = (Fbm3(float3(input.worldPos.z * 1.71 + seed * 13.0, input.worldPos.x * 0.39, seed * 31.0)) - 0.5) * edgeNoiseScale;
                float edgeNoiseE = (Fbm3(float3(input.worldPos.z * 1.59 + seed * 19.0, input.worldPos.x * 0.43, seed * 37.0)) - 0.5) * edgeNoiseScale;
                float edgeBase = lerp(0.090, 0.280, waterLiquid);
                float edgeMin = lerp(0.018, 0.080, waterLiquid);
                float edgeMax = lerp(0.210, 0.520, waterLiquid);
                float edgeN = clamp(edgeBase + edgeNoiseN, edgeMin, edgeMax);
                float edgeS = clamp(edgeBase + edgeNoiseS, edgeMin, edgeMax);
                float edgeW = clamp(edgeBase + edgeNoiseW, edgeMin, edgeMax);
                float edgeE = clamp(edgeBase + edgeNoiseE, edgeMin, edgeMax);
                float edgeLow = lerp(0.055, 0.160, waterLiquid);
                float edgeHigh = lerp(0.095, 0.260, waterLiquid);
                float edgeLimit = lerp(0.34, 0.68, waterLiquid);
                float fadeN = lerp(smoothstep(max(0.0, edgeN - edgeLow), min(edgeLimit, edgeN + edgeHigh), cuv.y), 1.0, edgeContinue0);
                float fadeS = lerp(smoothstep(max(0.0, edgeS - edgeLow), min(edgeLimit, edgeS + edgeHigh), 1.0 - cuv.y), 1.0, edgeContinue1);
                float fadeW = lerp(smoothstep(max(0.0, edgeW - edgeLow), min(edgeLimit, edgeW + edgeHigh), cuv.x), 1.0, edgeContinue2);
                float fadeE = lerp(smoothstep(max(0.0, edgeE - edgeLow), min(edgeLimit, edgeE + edgeHigh), 1.0 - cuv.x), 1.0, edgeContinue3);
                float tileEdgeFade = fadeN * fadeS * fadeW * fadeE;
)"
