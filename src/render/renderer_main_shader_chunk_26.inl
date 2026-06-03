R"(
                    topThickness *= lerp(0.62, 1.0, ceilingCoverage);
                }
                alpha = merged * ceilingMask * smoothstep(0.0, 0.35, leakAge);
                alpha = smoothstep(lerp(0.014, 0.008, waterLiquid), lerp(0.092, 0.072, waterLiquid), alpha);
                thickness = saturate(topThickness * 0.84 + merged * lerp(0.46, 0.74, waterLiquid)) * alpha;
                drips = 0.0;
            }
            else if (wallLeakSurface > 0.5)
            {
                float u = bloodUv.x;
                float away = bloodUv.y;
                float edgeJitterU = (Hash21(float2(floor(away * 10.0) + seed * 31.0, seed * 17.0)) - 0.5) * 0.055;
                float edgeJitterV = (Hash21(float2(floor(u * 10.0) + seed * 37.0, seed * 23.0)) - 0.5) * 0.050;
                if (highBloodDetail)
                {
                    edgeJitterU = (Fbm3(float3(away * 5.6, seed * 9.0, u * 1.7)) - 0.5) * 0.070;
                    edgeJitterV = (Fbm3(float3(u * 5.2, seed * 11.0, away * 1.9)) - 0.5) * 0.064;
                }
                float cardEdgeFade = smoothstep(0.012 + edgeJitterU, 0.115 + edgeJitterU, u) *
                    (1.0 - smoothstep(0.885 - edgeJitterU, 0.988 - edgeJitterU, u)) *
                    smoothstep(-0.18 + edgeJitterV, -0.030 + edgeJitterV, away) *
                    (1.0 - smoothstep(0.900 - edgeJitterV, 1.095 - edgeJitterV, away));
                float ceilingEdgeNoise = (Fbm3(float3(input.worldPos.xz * 2.0 + seed * 17.0, seed * 59.0)) - 0.5) * 0.24 +
                    (Noise3(float3(input.worldPos.xz * 7.5 + seed * 23.0, seed * 83.0)) - 0.5) * 0.075;
                float ceilingSideAbs = abs(u - 0.5) * 2.0;
                float ceilingFrontEdge = 1.0 - smoothstep(0.80 + ceilingEdgeNoise, 1.12 + ceilingEdgeNoise, away);
                float ceilingSideEdge = 1.0 - smoothstep(lerp(0.98, 0.72, saturate(away)) + ceilingEdgeNoise * 0.52,
                    1.14 + ceilingEdgeNoise * 0.40, ceilingSideAbs);
                cardEdgeFade *= saturate(ceilingFrontEdge * ceilingSideEdge);
                float topSource = 0.0;
                float topThickness = 0.0;
                [loop]
                for (int i = 0; i < 32; ++i)
                {
                    float fi = (float)i;
                    if (fi >= ceilingStreamCount) break;
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
                    float streamAge = max(0.0, leakAge - r0 * 2.2 - fi * 0.16);
                    float sourceGrow = smoothstep(0.0, 1.0, saturate(streamAge * (0.065 + r1 * 0.040) * lerp(1.0, 0.72, waterLiquid)));
                    float sourceReady = smoothstep(0.25, 1.15, streamAge);
                    float spreadAway = 0.030 + sourceGrow * (0.145 + r2 * 0.095);
                    float spreadSide = (0.018 + r2 * 0.022) * streamWidthScale * (1.0 + sourceGrow * 1.10);
                    spreadAway *= lerp(1.0, 1.58, waterLiquid);
                    spreadSide *= lerp(1.0, 1.32, waterLiquid);
                    float edgeNoise = (Hash21(float2(floor(u * 12.0) + fi, floor(away * 10.0) + seed * 23.0)) - 0.5) * 0.010 * sourceGrow;
                    if (highBloodDetail)
                    {
                        edgeNoise = (Fbm3(float3(u * 8.0 + fi, away * 7.0, seed * 23.0)) - 0.5) * 0.016 * sourceGrow;
                    }
                    float sideWorld = (u - center) * bloodUvMeters.x;
                    float awayWorld = away * bloodUvMeters.y;
                    float spreadSideWorld = max(0.006, spreadSide * bloodUvMeters.x);
                    float spreadAwayWorld = max(0.010, spreadAway * bloodUvMeters.x);
                    float edgeNoiseWorld = edgeNoise * bloodUvMeters.x;
                    float skew = (Hash21(float2(seed * 19.0 + fi, 91.0)) - 0.5) * sourceGrow * 0.42;
                    float2 q = float2((sideWorld + awayWorld * skew) / spreadSideWorld, (awayWorld + edgeNoiseWorld) / spreadAwayWorld);
                    float soakRag = Fbm3(float3(input.worldPos.xz * (11.0 + r2 * 9.0) + fi * 0.29, seed * 43.0 + fi));
                    float microBreak = Noise3(float3(input.worldPos.xz * (18.0 + r2 * 14.0) + fi * 0.17, seed * 71.0 + fi));
                    float lobe = 1.0 - smoothstep(0.54, 1.03,
                        dot(q, q) + (soakRag - 0.5) * (0.36 + sourceGrow * 0.42) + (microBreak - 0.5) * 0.10);
                    float capillary = smoothstep(0.60, 0.94,
                        Fbm3(float3(input.worldPos.xz * (20.0 + r1 * 10.0) + fi, seed * 61.0))) *
                        (1.0 - smoothstep(0.72, 1.28, dot(q, q))) * sourceGrow * sourceReady * 0.18;
                    float source = max(lobe, capillary) * sourceReady * streamStrength;
                    topSource = max(topSource, source);
                    topThickness = max(topThickness, source * (0.48 + sourceGrow * 0.42));
                }
                float raggedEdge = 0.88 + 0.12 * Hash21(float2(floor(u * 26.0) + seed * 67.0, floor(away * 18.0)));
                if (highBloodDetail)
                {
                    raggedEdge = 0.80 + 0.20 * Fbm3(float3(u * 20.0, away * 14.0, seed * 67.0));
                }
                float ceilingNoise = Fbm3(float3(input.worldPos.xz * 4.8 + seed * 7.0, seed * 37.0));
                float fineNoise = Noise3(float3(input.worldPos.xz * 18.0 + seed * 5.0, seed * 83.0));
                float organicMask = smoothstep(0.22, 0.70, ceilingNoise + (fineNoise - 0.5) * 0.20);
                float rimAge = smoothstep(lerp(5.5, 4.0, waterLiquid), lerp(11.5, 17.0, waterLiquid), leakAge);
                float rimNoise = Fbm3(float3(u * 5.4 + seed * 13.0, away * 3.2, seed * 47.0));
                float rimFine = Noise3(float3(u * 19.0 + seed * 7.0, away * 9.0, seed * 71.0));
                float rimWidth = 0.026 + rimNoise * 0.034;
)"
