R"(
                float lean = (r1 - 0.5) * y * (0.0025 + r2 * 0.0035);
                float microWobble = (Hash21(float2(fi * 5.1 + seed * 23.0, y * 3.0)) - 0.5) * (0.0007 + r2 * 0.0010);
                if (highBloodDetail)
                {
                    microWobble = (Fbm3(float3(y * 2.2, fi * 1.7, seed * 23.0)) - 0.5) * (0.0011 + r2 * 0.0018);
                }
                float wander = lean + microWobble;
                float du = u - center - wander;
                float trailGate = 1.0 - smoothstep(len, len + 0.060, y);
                float permanentTrace = smoothstep(0.02, 0.28, streamGrow);
                float flowEnd = 16.0 + r0 * 24.0 + r2 * 18.0;
                float flowFade = 7.0 + r1 * 12.0;
                float activeFlow = 1.0 - smoothstep(flowEnd, flowEnd + flowFade, flowAge);
                float wetPulse = 0.91 + 0.09 * activeFlow * sin(y * (24.0 + r1 * 18.0) - time * (0.18 + r2 * 0.16) + seed * 37.0);
                float widthNoise = 0.86 + 0.24 * Hash21(float2(fi * 13.7 + seed * 29.0, floor(y * 15.0)));
                if (highBloodDetail)
                {
                    widthNoise = 0.76 + 0.38 * Fbm3(float3(fi * 2.2 + seed * 19.0, y * 8.0, seed * 41.0));
                }
                float gravityBulge = smoothstep(0.16, 0.92, y) * (0.08 + r1 * 0.18) +
                    smoothstep(0.78, 1.0, y) * (0.16 + r2 * 0.18);
                float widthTaper = width * (0.78 + gravityBulge) * widthNoise;
                float strandBreaks = lerp(0.70, 1.0, Hash21(float2(fi * 9.0 + seed * 43.0, floor(y * 22.0))));
                if (highBloodDetail)
                {
                    strandBreaks = smoothstep(0.18, 0.50, Fbm3(float3(u * 35.0 + fi, y * 10.0, seed * 43.0)) + 0.18);
                }
                float stream = exp(-(du * du) / max(0.000008, widthTaper * widthTaper)) * trailGate * permanentTrace * wetPulse * strandBreaks * streamStrength;
                float bleedHalo = exp(-(du * du) / max(0.000018, widthTaper * widthTaper * 18.0)) *
                    trailGate * permanentTrace * streamStrength * 0.46;
                float sdfWidth = widthTaper * (5.8 + bloodQuality * 3.2);
                float field = saturate(1.0 - abs(du) / max(0.0008, sdfWidth)) *
                    trailGate * permanentTrace * streamStrength;
                sdfWet += field;
                sdfCore += field * field;
                float soakDelay = (3.2 + r2 * 10.0 + y * (7.0 + r1 * 12.0)) * lerp(1.0, 0.74, waterLiquid);
                float soakAge = max(0.0, flowAge - soakDelay);
                float soakGrow = smoothstep(0.0, 1.0, saturate(soakAge * (0.070 + r0 * 0.058) * lerp(1.0, 1.18, waterLiquid)));
                float soakNoise = 0.72 + 0.28 * Hash21(float2(fi * 17.0 + seed * 61.0, floor(y * 16.0) + floor(u * 9.0)));
                if (highBloodDetail)
                {
                    soakNoise = 0.52 + 0.48 * Fbm3(float3(u * 12.0 + fi * 0.37, y * 8.0, seed * 67.0));
                }
                float soakWidth = widthTaper * (8.0 + bloodQuality * 13.0) * (0.62 + soakGrow * 1.22);
                float soakGate = 1.0 - smoothstep(len - 0.035, len + 0.20, y);
                float localSoak = saturate(1.0 - abs(du) / max(0.0012, soakWidth)) *
                    soakGate * soakGrow * streamStrength * soakNoise;
                diffuseSoak += localSoak;
                float headY = len - 0.018;
                float bead = 0.0;
                if (highBloodDetail)
                {
                    bead = exp(-(du * du) / max(0.000018, width * width * 3.0)) *
                        exp(-((y - headY) * (y - headY)) / max(0.00005, width * width * 22.0)) * streamStrength;
                }
                float sourceWidth = width * (2.3 + r1 * 1.8) + 0.0032 * wallStreamWidthScale;
                float sourceDepth = 0.018 + r2 * 0.040;
                float sourceY = 0.010 + Hash21(float2(seed * 269.0 + fi, 167.0)) * 0.020;
                float sourceNoise = 0.72 + 0.28 * Hash21(float2(floor(u * 18.0) + fi, seed * 37.0));
                if (highBloodDetail)
                {
                    sourceNoise = 0.45 + 0.55 * Fbm3(float3(u * 12.0, seed * 37.0 + fi, y * 5.0));
                }
                float sourceEdge = (Fbm3(float3(u * 18.0 + fi, y * 22.0, seed * 53.0)) - 0.5) *
                    (0.18 + sourceReady * 0.16);
                float2 sourceQ = float2((u - center) / max(0.0012, sourceWidth),
                    (y - sourceY) / max(0.0012, sourceDepth * (0.62 + r0 * 0.35)));
                float sourceBlob = 1.0 - smoothstep(0.56, 1.12, dot(sourceQ, sourceQ) + sourceEdge);
                float sourceFeeder = exp(-((u - center) * (u - center)) / max(0.000035, sourceWidth * sourceWidth * 0.42)) *
                    (1.0 - smoothstep(sourceDepth * (0.80 + r2 * 0.40),
                        sourceDepth * (1.55 + r1 * 0.70), y)) *
                    smoothstep(0.45, 1.0, sourceReady) * 0.38;
                float source = max(sourceBlob, sourceFeeder) * sourceNoise * sourceReady * streamStrength;
                streams = max(streams, stream);
                streamAccum += saturate(stream + bleedHalo);
                beads = max(beads, bead * smoothstep(0.08, 0.98, streamGrow) * (1.0 - smoothstep(0.96, 1.0, streamGrow) * 0.30));
                topSource = max(topSource, source);
            }
            float wallFullHeightGate = smoothstep(0.0, 0.024, y) * (1.0 - smoothstep(0.994, 1.0, y));
            float wallSheet = smoothstep(0.16, 0.86, streamAccum) *
                wallFullHeightGate *
                smoothstep(8.0, 18.0, leakAge) * 0.46;
            float mergedField = smoothstep(0.70, 1.68, sdfWet) * wallFullHeightGate;
            float mergedCore = smoothstep(0.36, 1.20, sdfCore) * wallFullHeightGate;
            float wallEdgeNoise = (Fbm3(float3(u * 4.1, seed * 9.0, y * 2.3)) - 0.5) * 0.075;
            float wallTopFade = smoothstep(-0.004 + wallEdgeNoise * 0.10, 0.036 + wallEdgeNoise * 0.10, y);
            float wallVerticalFade = wallTopFade * (1.0 - smoothstep(0.992, 1.0, y));
            float wallCardFade = smoothstep(0.018 + wallEdgeNoise, 0.115 + wallEdgeNoise, u) *
                (1.0 - smoothstep(0.885 - wallEdgeNoise, 0.982 - wallEdgeNoise, u)) *
                wallVerticalFade;
            float floodNoise = Fbm3(input.worldPos * float3(4.7, 2.8, 4.7) + seed * 11.0);
            float floodFine = Noise3(input.worldPos * float3(18.0, 8.0, 18.0) + seed * 29.0);
            float organicFlood = smoothstep(0.20, 0.72, floodNoise + (floodFine - 0.5) * 0.18);
            float streamFedSoak = smoothstep(0.34, 1.30, diffuseSoak) * wallCardFade *
                organicFlood * (0.40 + smoothstep(0.74, 1.80, diffuseSoak) * 0.22);
)"
