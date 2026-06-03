R"(
        fog = 1.0 - exp(-fog * fog * 3.2);
        float fogBlock = saturate(fog * gFog0.z * 1.25);
        color = lerp(color, float3(0.0, 0.0, 0.0), fogBlock);
        filmAlpha *= 1.0 - fogBlock * 0.18;
        return float4(ApplyPost(color), filmAlpha);
    }


    if (input.material > 11.05 && input.material < 11.45)
    {
        float seed = frac(input.material * 23.71);
        float2 smokeUv = AspectSoftenedBloodUv(uv, input.worldPos);
        float2 plane = float2(smokeUv.x * 2.0 - 1.0, 1.0 - smokeUv.y * 2.0);
        float edgeDist = min(min(smokeUv.x, 1.0 - smokeUv.x), min(smokeUv.y, 1.0 - smokeUv.y));
        float cardFade = smoothstep(0.110, 0.330, edgeDist);
        float2 ovalP = plane * float2(1.10, 1.04);
        float radial = dot(ovalP, ovalP);
        float ovalFade = exp(-radial * 2.65) * (1.0 - smoothstep(0.62, 1.02, radial));
        float3 rdLocal = normalize(float3(dot(-V, T), dot(-V, B), dot(-V, N)));
        float3 p0 = float3(plane.x * 0.96, plane.y * 1.05, 0.0);
        float heightFade = smoothstep(0.02, 0.30, input.worldPos.y) * (1.0 - smoothstep(2.10, 2.70, input.worldPos.y));
        float transmittance = 1.0;
        float alpha = 0.0;
        [loop]
        for (int s = 0; s < 9; ++s)
        {
            float stepT = -0.95 + ((float)s + 0.5) * (1.90 / 9.0);
            float3 p = p0 + rdLocal * stepT;
            p.xz *= 0.90 + seed * 0.16;
            p.y += sin(time * (0.31 + seed * 0.10) + seed * 21.0) * 0.10;
            float density = BlackSmokeDensity(p, seed + (float)s * 0.017, time);
            float sampleAlpha = saturate(density * 0.210);
            alpha += transmittance * sampleAlpha;
            transmittance *= 1.0 - sampleAlpha;
        }
        float reform = 0.83 + 0.17 * sin(time * (0.68 + seed * 0.21) + seed * 17.0 + input.worldPos.y * 1.7);
        alpha = saturate(alpha * heightFade * cardFade * ovalFade * reform * 1.42);
        if (alpha < 0.012) discard;
        float3 color = float3(0.0, 0.0, 0.0);

        float dist = length(input.worldPos - cam);
        float fog = saturate((dist - gFog0.x) / max(0.01, gFog0.y - gFog0.x));
        fog = 1.0 - exp(-fog * fog * 3.2);
        float fogBlock = saturate(fog * gFog0.z * 1.35);
        alpha *= pow(1.0 - fogBlock, 1.65);
        return float4(color, alpha);
    }

    if (input.material > 11.005 && input.material < 11.055)
    {
        float seed = frac(input.material * 29.73);
        float encodedSide = clamp(floor(rawUv.x + 0.0001), 0.0, 3.0);
        float rawModeEncoded = floor(rawUv.y + 0.0001);
        float floorNeighborMask = floor(rawModeEncoded / 8.0);
        float rawMode = rawModeEncoded - floorNeighborMask * 8.0;
        float warpA = Fbm3(float3(uv * (2.8 + seed * 2.2) + seed * 5.1, seed * 17.0));
        float warpB = Fbm3(float3(uv.yx * (3.4 + seed * 1.7) - seed * 4.3, seed * 29.0));
        float2 warpedUv = saturate(uv + (float2(warpA, warpB) - 0.5) * (0.075 + seed * 0.035));
        float2 d = warpedUv - 0.5;
        float border = min(min(uv.x, 1.0 - uv.x), min(uv.y, 1.0 - uv.y));
        float broad = Fbm3(float3(warpedUv * 5.7 + seed * 3.1, seed * 17.0));
        float fine = Fbm3(float3(warpedUv * 18.0 - seed * 2.0, seed * 31.0));
        float floorSurface = saturate(N.y * 2.0);
        float ceilingSurface = saturate(-N.y * 2.0);
        float horizontalSurface = saturate(abs(N.y) * 2.0);
        float vertical = 1.0 - horizontalSurface;
        float floorBridge = floorSurface * step(3.5, rawMode) * (1.0 - step(4.5, rawMode));
        float wallFromCeiling = vertical * step(2.5, rawMode) * (1.0 - step(3.5, rawMode));
        float wallFromFloor = vertical * step(3.5, rawMode);
        float ceilingCompact = ceilingSurface * (1.0 - vertical) * step(2.5, rawMode) * (1.0 - step(3.5, rawMode));
        float floorTouchdown = floorSurface * step(4.5, rawMode) * (1.0 - step(5.5, rawMode));
        float encodedMode = clamp(lerp(rawMode, 0.0, saturate(ceilingCompact + floorTouchdown)), 0.0, 2.0);
        float encodedEdge = (1.0 - vertical) * step(0.5, encodedMode);
        float edgeOnly = (1.0 - vertical) * step(1.5, encodedMode);
        float materialBand = saturate((input.material - 11.006) / 0.049);
        float edgeMode = max(smoothstep(0.32, 0.60, materialBand), encodedEdge);
        float floorMergeN = step(0.5, fmod(floor(floorNeighborMask / 1.0), 2.0));
        float floorMergeS = step(0.5, fmod(floor(floorNeighborMask / 2.0), 2.0));
        float floorMergeW = step(0.5, fmod(floor(floorNeighborMask / 4.0), 2.0));
        float floorMergeE = step(0.5, fmod(floor(floorNeighborMask / 8.0), 2.0));
        float floorMergeNW = step(0.5, fmod(floor(floorNeighborMask / 16.0), 2.0)) * saturate(floorMergeN + floorMergeW);
        float floorMergeNE = step(0.5, fmod(floor(floorNeighborMask / 32.0), 2.0)) * saturate(floorMergeN + floorMergeE);
        float floorMergeSW = step(0.5, fmod(floor(floorNeighborMask / 64.0), 2.0)) * saturate(floorMergeS + floorMergeW);
        float floorMergeSE = step(0.5, fmod(floor(floorNeighborMask / 128.0), 2.0)) * saturate(floorMergeS + floorMergeE);
        float2 mergedD = d;
        mergedD.x = d.x * lerp(1.0, 0.76, saturate(floorMergeW + floorMergeE)) +
            (floorMergeW - floorMergeE) * 0.08;
        mergedD.y = d.y * lerp(1.0, 0.76, saturate(floorMergeN + floorMergeS)) +
            (floorMergeS - floorMergeN) * 0.08;

        float puddleAngle = seed * 6.28318 + Hash21(float2(seed, 3.7)) * 2.4;
        float2 puddleRot = float2(cos(puddleAngle), sin(puddleAngle));
        float2 rd = float2(dot(mergedD, puddleRot), dot(mergedD, float2(-puddleRot.y, puddleRot.x)));
        float2 puddleAspect = float2(0.82 + Hash21(float2(seed, 5.1)) * 0.62,
                                     0.74 + Hash21(float2(seed, 7.3)) * 0.72);
)"
