R"(
        float sparkLight = SparkLight(input.worldPos, wetN);
        float3 exitGreen = ExitSignLight(input.worldPos, wetN, materialId);
        float exitGlow = max(exitGreen.r, max(exitGreen.g, exitGreen.b));
        float3 toLight = normalize(gShadow0.xyz - input.worldPos);
        float facing = saturate(dot(reflect(-toLight, wetN), V));
        float fresnel = pow(1.0 - saturate(dot(wetN, V)), 5.0);
        float specLight = saturate(flashlight + overhead * 0.58 + sparkLight * 0.42 + exitGlow * 0.54);
        float spec = (pow(facing, 110.0) * 0.30 + pow(facing, 30.0) * 0.052 + fresnel * 0.014) * specLight * core;
        float filmAlpha = saturate(core * (0.40 + broad * 0.145) + rim * (0.105 + fine * 0.050) +
            vertical * (wallWaterCore * 0.16 + wallWaterSoak * 0.07 + wallBottomSoak * 0.10));
        filmAlpha *= lerp(1.12, 0.82, vertical);
        if (filmAlpha < 0.035) discard;
        float3 wetFilm = float3(0.0018, 0.0024, 0.0022) * (0.26 + specLight * 0.13);
        float3 color = wetFilm + float3(0.36, 0.42, 0.39) * spec;
        float wallFlow = vertical * saturate(wallWaterCore + wallWaterSoak * 0.42);
        float wallDamp = vertical * wallBottomSoak;
        color = lerp(color, float3(0.0007, 0.0012, 0.0010) * (0.50 + specLight * 0.24), wallFlow * 0.76);
        color = lerp(color, float3(0.0008, 0.0011, 0.0010) * (0.38 + specLight * 0.16), wallDamp * 0.42);
        color += float3(0.28, 0.36, 0.33) * spec * wallFlow * 0.72;
        color += exitGreen * (0.055 + core * 0.12 + spec * 0.55);
        float dist = length(input.worldPos - cam);
        float fog = saturate((dist - gFog0.x) / max(0.01, gFog0.y - gFog0.x));
        fog = 1.0 - exp(-fog * fog * 3.2);
        float fogBlock = saturate(fog * gFog0.z * 1.28);
        color = lerp(color, float3(0.0, 0.0, 0.0), fogBlock);
        filmAlpha *= pow(1.0 - fogBlock, 1.35);
        return float4(ApplyPost(color), filmAlpha);
    }


    if (gHorror0.x > 0.01 && materialId < 11.5 && !(materialId > 3.5 && materialId < 4.5))
    {
        float flesh = saturate(gHorror0.x);
        float3 viewTS = float3(dot(V, T), dot(V, B), max(dot(V, N), 0.12));
        float2 fleshUv = rawUv * 0.72 + float2(Hash21(floor(input.worldPos.xz * 0.27)), Hash21(floor(input.worldPos.zx * 0.31))) * 0.23;
        float parallaxScale = gHorror0.w * (0.55 + flesh * 0.45);
        float layers = lerp(18.0, 9.0, saturate(viewTS.z));
        float2 stepUv = (viewTS.xy / max(viewTS.z, 0.12)) * (parallaxScale / layers);
        float2 pomUv = fleshUv;
        float layerDepth = 0.0;
        float currentDepth = 1.0 - gNormalHeight.Sample(gSampler, float3(pomUv, 15.0)).a;
        [loop]
        for (int p = 0; p < 18; ++p)
        {
            if ((float)p >= layers || layerDepth >= currentDepth) break;
            pomUv -= stepUv;
            layerDepth += 1.0 / layers;
            currentDepth = 1.0 - gNormalHeight.Sample(gSampler, float3(pomUv, 15.0)).a;
        }
        fleshUv = lerp(fleshUv, pomUv, saturate(parallaxScale * 18.0));
        float3 materialUv = float3(fleshUv, 15.0);
        float4 base = gAlbedo.Sample(gSampler, materialUv);
        float4 nh = gNormalHeight.Sample(gSampler, materialUv);
        float4 pbr = gMaterialProps.Sample(gSampler, materialUv);
        float aoMap = saturate(pbr.r);
        float sourceRoughness = saturate(pbr.g);
        float3 nTex = normalize(nh.xyz * 2.0 - 1.0);
        nTex = normalize(float3(nTex.xy * (1.28 + flesh * 0.42), nTex.z));
        float3 worldN = normalize(nTex.x * T + nTex.y * B + nTex.z * N);
        float dist = length(input.worldPos - cam);
        float flashlight = FlashlightAmount(input.worldPos, worldN);
        float3 toLight = normalize(gShadow0.xyz - input.worldPos);
        float3 reflectDir = reflect(-toLight, worldN);
        float wet = saturate(gHorror0.z);
        float ndv = saturate(dot(worldN, V));
        float fresnel = pow(1.0 - ndv, 4.0);
        float facing = saturate(dot(reflectDir, V));
        float effectiveRoughness = lerp(sourceRoughness, max(0.18, sourceRoughness * 0.42), wet);
        effectiveRoughness = saturate(effectiveRoughness - wet * 0.055);
        float gloss = 1.0 - effectiveRoughness;
        float specSharp = pow(facing, lerp(24.0, 150.0, gloss)) * lerp(0.42, 2.7, gloss);
        float specBroad = pow(facing, lerp(5.0, 22.0, gloss)) * lerp(0.48, 0.16, gloss);
        float poreSparkle = smoothstep(0.58, 0.92, Fbm3(float3(fleshUv * 42.0, 17.0))) * wet * gloss;
        float spec = (specSharp + specBroad + fresnel * (0.20 + wet * 0.36) + poreSparkle * 0.36) *
            flashlight * (0.75 + wet * 3.3 + flesh * 0.85);
        float cavity = saturate((0.58 - nh.a) * 1.9);
        float ridge = saturate((nh.a - 0.48) * 1.7);
        float3 fleshColor = base.rgb * (0.48 + flesh * 0.22) + float3(0.10, 0.005, 0.003) * flesh;
        float aoShadow = lerp(0.34, 1.0, aoMap);
        fleshColor = lerp(fleshColor, fleshColor * float3(0.28, 0.08, 0.07), saturate(cavity * 0.62 + (1.0 - aoMap) * 0.54));
        fleshColor += float3(0.085, 0.014, 0.008) * ridge * wet;
        float3 color = fleshColor * flashlight * 0.94 * aoShadow;
        color += float3(1.0, 0.21, 0.085) * spec * lerp(0.62, 0.98, aoMap);
        color *= 1.0 - CornerAO(input.worldPos, worldN) * 0.65;
        color = lerp(color, float3(0.0, 0.0, 0.0), SceneFogBlock(dist, input.worldPos, 1.0));
        return float4(ApplyPost(color), 1.0);
    }

    if ((materialId > 2.5 && materialId < 3.5) || (materialId > 4.5 && materialId < 5.5))
    {
        float edge = max(smoothstep(0.055, 0.0, min(uv.x, 1.0 - uv.x)),
                         smoothstep(0.055, 0.0, min(uv.y, 1.0 - uv.y)));
        float lens = smoothstep(0.42, 0.0, abs(uv.y - 0.5)) * smoothstep(0.46, 0.0, abs(uv.x - 0.5));
        float3 lampBase = float3(0.72 + lens * 0.24 - edge * 0.18,
                                 0.76 + lens * 0.23 - edge * 0.16,
)"
