R"(
            float wallFlood = saturate(streamFedSoak);
            float ceilingContactFeed = smoothstep(0.18, 0.52, topSource + streams * 0.18 + sdfWet * 0.10);
            float ceilingContact = (1.0 - smoothstep(0.0, 0.038, y)) *
                smoothstep(1.8, 7.5, leakAge) *
                ceilingContactFeed *
                smoothstep(0.015 + wallEdgeNoise, 0.090 + wallEdgeNoise, u) *
                (1.0 - smoothstep(0.910 - wallEdgeNoise, 0.990 - wallEdgeNoise, u));
            wallFlood = saturate(wallFlood + ceilingContact * 0.22);
            float floorContact = smoothstep(0.935, 1.0, y) *
                smoothstep(5.0, 12.0, leakAge) *
                smoothstep(0.06, 0.42, streams + sdfWet * 0.20 + streamAccum * 0.035 + wallFlood * 0.24) *
                smoothstep(0.015 + wallEdgeNoise, 0.090 + wallEdgeNoise, u) *
                (1.0 - smoothstep(0.910 - wallEdgeNoise, 0.990 - wallEdgeNoise, u));
            wallFlood = saturate(wallFlood + floorContact * 0.36);
            wallFlood = min(wallFlood, 0.58);
            streams = saturate(streams + wallSheet * (0.48 + bloodQuality * 0.22) +
                mergedField * (0.66 + bloodQuality * 0.24) + wallFlood * (0.36 + bloodQuality * 0.18));
            float bottomGather = smoothstep(0.90, 1.0, y) * (streams * 0.48 + topSource * 0.10) * smoothstep(4.5, 9.5, leakAge);
            float brokenEdges = 0.88 + 0.12 * Hash21(float2(floor(u * 34.0) + seed * 59.0, floor(y * 24.0)));
            if (highBloodDetail)
            {
                brokenEdges = 0.78 + 0.22 * Fbm3(float3(u * 26.0, y * 18.0, seed * 59.0));
            }
            alpha = max(max(streams * brokenEdges, beads * 1.14), max(max(topSource * 0.58, bottomGather), max(wallFlood * 0.70, max(ceilingContact * 0.72, floorContact * 0.72))));
            alpha = smoothstep(0.008, 0.046, alpha);
            drips = saturate(streams + beads + bottomGather * 0.65 + wallFlood * 0.36 + floorContact * 0.28);
            thickness = saturate(streams * 0.60 + mergedCore * 0.78 + wallFlood * 0.68 + beads * 0.82 + bottomGather * 0.46 + topSource * 0.34 + ceilingContact * 0.28 + floorContact * 0.32);
            thickness *= alpha;
            drips *= alpha;
        }


        else if (floorMask > 0.45)
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
                float downstreamCode = step(15.5, neighborCode);
                float sideActive0 = fmod(floor(sideCode / 1.0), 2.0);
                float sideActive1 = fmod(floor(sideCode / 2.0), 2.0);
                float sideActive2 = fmod(floor(sideCode / 4.0), 2.0);
                float sideActive3 = fmod(floor(sideCode / 8.0), 2.0);
                float pooled = 0.0;
                float pooledField = 0.0;
                float wetRim = 0.0;
                float soakField = 0.0;
                float lobeThickness = 0.0;
                float finiteReachField = 0.0;
                float sideSourceCount = saturate(sideActive0 + sideActive1 + sideActive2 + sideActive3);
                float delayedWallContact = (1.0 - centerCode) * sideSourceCount * (1.0 - downstreamCode);
                float downstreamFlow = downstreamCode * sideSourceCount;
                float downstreamWater = waterLiquid * downstreamFlow;
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
                        float r2 = Hash21(float2(fi + 9.0, seed * 71.0 + sideSalt));
                        float clusterCount = 2.0 + floor(Hash21(float2(seed * 83.0 + sideSalt, 19.0)) * 3.0);
                        float clusterId = floor(fmod(fi + floor(seed * 31.0 + sideSalt * 7.0), clusterCount));
                        float uniformCenter = 0.040 + ((fi + 0.20 + r0 * 0.60) / max(1.0, canvasStreamCount)) * 0.92;
                        float clusterCenter = 0.055 + Hash21(float2(seed * 89.0 + clusterId * 5.7 + sideSalt, 13.0)) * 0.89;
                        float clusterSpread = 0.030 + Hash21(float2(seed * 97.0 + clusterId + sideSalt, 29.0)) * 0.22;
                        float sourceU = clamp(lerp(uniformCenter, clusterCenter + (r1 - 0.5) * clusterSpread, 0.70), 0.025, 0.975);
                        float sideWorld = (sideU - sourceU) * alongMeters;
                        float awayWorld = away * awayMeters;
                        float contactDelay = delayedWallContact * lerp(12.0 + r0 * 4.0 + fi * 0.040, 5.8 + r0 * 2.2 + fi * 0.026, waterLiquid);
                        float travelDelay = delayedWallContact * (away * lerp(20.0, 34.0, waterLiquid));
                        float downstreamDelay = downstreamFlow * lerp(22.0 + r0 * 7.0 + away * 18.0, 18.0 + r0 * 9.0 + away * 16.0, waterLiquid);
                        float flowAge = max(0.0, leakAge - r0 * lerp(9.0, 7.8, waterLiquid) - fi * lerp(0.030, 0.070, waterLiquid) - contactDelay - travelDelay - downstreamDelay);
                        float grow = smoothstep(0.0, 1.0, saturate(flowAge * lerp(0.030 + r1 * 0.026, 0.018 + r1 * 0.016, waterLiquid) * lerp(1.0, 0.46, downstreamWater)));
                        float awayContinue = sideIndex == 0 ? edgeContinue1 : (sideIndex == 1 ? edgeContinue0 : (sideIndex == 2 ? edgeContinue3 : edgeContinue2));
                        float sideContinue = sideIndex < 2 ? max(edgeContinue2, edgeContinue3) : max(edgeContinue0, edgeContinue1);
                        float reachNoise = (Fbm3(float3(input.worldPos.xz * (1.20 + r2 * 0.55) + fi * 0.11 + sideSalt, seed * 31.0)) - 0.5);
)"
