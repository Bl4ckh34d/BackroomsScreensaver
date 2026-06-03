R"(
        float2 floorPuddleAspect = float2(0.94 + Hash21(float2(seed, 5.1)) * 0.14,
                                          0.94 + Hash21(float2(seed, 7.3)) * 0.14);
        float floorShape = smoothstep(0.70, 0.16, length(rd * floorPuddleAspect) + (broad - 0.5) * 0.105);
        float floorLobes = 0.0;
        [loop]
        for (int f = 0; f < 4; ++f)
        {
            float ff = (float)f;
            float enabled = step(0.22, Hash21(float2(seed * 67.0 + ff, 31.0)));
            float2 fc = float2(Hash21(float2(seed * 71.0 + ff, 37.0)),
                               Hash21(float2(seed * 73.0, ff + 41.0))) - 0.5;
            fc *= 0.18 + Hash21(float2(seed * 79.0 + ff, 43.0)) * 0.34;
            float fa = seed * 3.9 + ff * 1.21 + Hash21(float2(seed, ff + 47.0)) * 1.3;
            float2 fr = float2(cos(fa), sin(fa));
            float2 fq = mergedD - fc;
            fq = float2(dot(fq, fr), dot(fq, float2(-fr.y, fr.x)));
            fq *= float2(1.10 + Hash21(float2(ff, seed * 83.0)) * 0.82,
                         0.76 + Hash21(float2(seed * 89.0, ff)) * 0.58);
            float oval = smoothstep(0.34 + Hash21(float2(seed * 97.0, ff)) * 0.17, 0.09, length(fq));
            floorLobes = max(floorLobes, oval * enabled * (0.48 + Hash21(float2(seed * 101.0 + ff, 53.0)) * 0.40));
        }
        floorShape = max(floorShape, floorLobes);
        float touchdownNoise = (Fbm3(float3(warpedUv * (10.0 + seed * 3.0) + seed * 11.0, seed * 59.0)) - 0.5) * 0.095 +
            (fine - 0.5) * 0.030;
        float touchdownRadial = length(rd * float2(0.94 + seed * 0.08, 0.88 + Hash21(float2(seed, 61.0)) * 0.12));
        float touchdownCore = 1.0 - smoothstep(0.30, 0.47, touchdownRadial + touchdownNoise * 0.55);
        float touchdownBody = 1.0 - smoothstep(0.48, 0.64, touchdownRadial + touchdownNoise);
        float touchdownRim = (1.0 - smoothstep(0.62, 0.75, touchdownRadial + touchdownNoise * 1.15)) *
            smoothstep(0.34, 0.58, touchdownRadial + touchdownNoise * 0.65);
        float touchdownShape = saturate(max(touchdownCore, touchdownBody * 0.88) + touchdownRim * 0.22);
        floorShape = lerp(floorShape, max(touchdownShape, floorShape * 0.32), floorTouchdown);
        float floorSeamNoise = (Fbm3(float3(uv * 8.0 + seed * 13.0, seed * 41.0)) - 0.5) * 0.075;
        float floorSeamBreak = smoothstep(0.18, 0.72, broad + fine * 0.13);
        float floorAlongX = smoothstep(0.030, 0.185, uv.x) * (1.0 - smoothstep(0.815, 0.970, uv.x));
        float floorAlongY = smoothstep(0.030, 0.185, uv.y) * (1.0 - smoothstep(0.815, 0.970, uv.y));
        float floorWetN = floorMergeN * (1.0 - smoothstep(0.27 + floorSeamNoise, 0.62 + floorSeamNoise, 1.0 - uv.y)) * floorAlongX;
        float floorWetS = floorMergeS * (1.0 - smoothstep(0.27 - floorSeamNoise, 0.62 - floorSeamNoise, uv.y)) * floorAlongX;
        float floorWetW = floorMergeW * (1.0 - smoothstep(0.27 - floorSeamNoise, 0.62 - floorSeamNoise, uv.x)) * floorAlongY;
        float floorWetE = floorMergeE * (1.0 - smoothstep(0.27 + floorSeamNoise, 0.62 + floorSeamNoise, 1.0 - uv.x)) * floorAlongY;
        float cornerNW = floorMergeNW * (1.0 - smoothstep(0.24, 0.62, length(float2(uv.x, 1.0 - uv.y) * float2(1.08, 1.08)) + floorSeamNoise));
        float cornerNE = floorMergeNE * (1.0 - smoothstep(0.24, 0.62, length(float2(1.0 - uv.x, 1.0 - uv.y) * float2(1.08, 1.08)) - floorSeamNoise));
        float cornerSW = floorMergeSW * (1.0 - smoothstep(0.24, 0.62, length(float2(uv.x, uv.y) * float2(1.08, 1.08)) - floorSeamNoise));
        float cornerSE = floorMergeSE * (1.0 - smoothstep(0.24, 0.62, length(float2(1.0 - uv.x, uv.y) * float2(1.08, 1.08)) + floorSeamNoise));
        float floorSeamShape = max(max(floorWetN, floorWetS), max(floorWetW, floorWetE));
        float floorCornerShape = max(max(cornerNW, cornerNE), max(cornerSW, cornerSE));
        floorShape = max(floorShape, max(floorSeamShape * (0.34 + floorSeamBreak * 0.18),
            floorCornerShape * (0.74 + floorSeamBreak * 0.26)));


        floorShape *= 1.0 - smoothstep(0.78, 0.96,
            Fbm3(float3(warpedUv * (8.0 + seed * 3.0) + seed * 9.0, seed * 37.0))) * 0.075;

        float ceilingShape = smoothstep(0.66, 0.15, length(rd * puddleAspect) + (broad - 0.5) * 0.25);
        float ceilingSdf = 10.0;
        float ceilingField = 0.0;
        [loop]
        for (int l = 0; l < 9; ++l)
        {
            float fl = (float)l;
            float2 lc = float2(Hash21(float2(seed * 17.0 + fl, 11.0)),
                               Hash21(float2(seed * 23.0, fl + 13.0))) - 0.5;
            lc *= 0.18 + Hash21(float2(seed * 31.0 + fl, 17.0)) * 0.72;
            float la = seed * 5.1 + fl * 1.37 + Hash21(float2(seed, fl + 19.0)) * 1.9;
            float2 lr = float2(cos(la), sin(la));
            float2 q = mergedD - lc;
            q = float2(dot(q, lr), dot(q, float2(-lr.y, lr.x)));
            q *= float2(1.05 + Hash21(float2(fl, seed * 37.0)) * 1.15,
                        0.76 + Hash21(float2(seed * 41.0, fl)) * 0.95);
            float radius = 0.16 + Hash21(float2(seed * 43.0, fl)) * 0.27;
            float edgeRough = (Fbm3(float3((q + lc) * (8.0 + fl * 0.7), seed * 47.0 + fl)) - 0.5) * 0.070;
            float sdf = length(q) - radius + edgeRough;
            ceilingSdf = min(ceilingSdf, sdf);
            float lobe = 1.0 - smoothstep(-0.035, 0.125 + Hash21(float2(seed * 53.0, fl)) * 0.08, sdf);
            ceilingField += saturate(lobe) * (0.45 + Hash21(float2(seed * 47.0 + fl, 23.0)) * 0.48);
        }
        float islands = smoothstep(0.66, 0.94, fine) * smoothstep(0.84, 0.22, length(mergedD * (1.12 + seed * 0.32)));
        float mergedCeiling = max(1.0 - smoothstep(-0.015, 0.110, ceilingSdf),
            smoothstep(0.62, 1.42, ceilingField));
        float dryBreak = smoothstep(0.72, 0.94,
            Fbm3(float3(warpedUv * (9.0 + seed * 5.0) + seed * 13.0, seed * 53.0))) *
            smoothstep(0.12, 0.72, max(ceilingShape, mergedCeiling));
        float ceilingNoiseEdge = (Fbm3(float3(warpedUv * 15.0 + seed * 19.0, seed * 61.0)) - 0.5) * 0.18;
        ceilingShape = max(max(ceilingShape * 0.52, mergedCeiling), islands * 0.74) * lerp(1.0, 0.22, edgeOnly);
        ceilingShape *= 1.0 - dryBreak * 0.26;
        ceilingShape = saturate(ceilingShape + ceilingNoiseEdge * smoothstep(0.18, 0.78, ceilingShape));
        float ceilingSeamNoise = (Fbm3(float3(uv * 6.5 + seed * 17.0, seed * 73.0)) - 0.5) * 0.105;
        float ceilingSeamBreak = smoothstep(0.16, 0.76, broad + fine * 0.16);
        float ceilingAlongX = smoothstep(0.020, 0.155, uv.x) * (1.0 - smoothstep(0.845, 0.985, uv.x));
        float ceilingAlongY = smoothstep(0.020, 0.155, uv.y) * (1.0 - smoothstep(0.845, 0.985, uv.y));
        float ceilingWetN = floorMergeN * (1.0 - smoothstep(0.20 + ceilingSeamNoise, 0.58 + ceilingSeamNoise, 1.0 - uv.y)) * ceilingAlongX;
        float ceilingWetS = floorMergeS * (1.0 - smoothstep(0.20 - ceilingSeamNoise, 0.58 - ceilingSeamNoise, uv.y)) * ceilingAlongX;
        float ceilingWetW = floorMergeW * (1.0 - smoothstep(0.20 - ceilingSeamNoise, 0.58 - ceilingSeamNoise, uv.x)) * ceilingAlongY;
        float ceilingWetE = floorMergeE * (1.0 - smoothstep(0.20 + ceilingSeamNoise, 0.58 + ceilingSeamNoise, 1.0 - uv.x)) * ceilingAlongY;
        float ceilingCornerNW = floorMergeNW * (1.0 - smoothstep(0.18, 0.58, length(float2(uv.x, 1.0 - uv.y) * float2(1.0, 1.0)) + ceilingSeamNoise));
        float ceilingCornerNE = floorMergeNE * (1.0 - smoothstep(0.18, 0.58, length(float2(1.0 - uv.x, 1.0 - uv.y) * float2(1.0, 1.0)) - ceilingSeamNoise));
)"
