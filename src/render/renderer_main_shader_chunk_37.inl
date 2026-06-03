R"(
        return float4(ApplyPost(color), 1.0);
    }

    if (materialId > 23.5 && materialId < 24.5)
    {
        float softness = saturate(frac(input.material));
        float2 p = uv * 2.0 - 1.0;
        float edgeNoise = (Fbm3(float3(input.worldPos.xz * (2.1 + softness * 1.4), softness * 17.0)) - 0.5) * (0.055 + softness * 0.070);
        float fineBreakup = Fbm3(float3(input.worldPos.xz * 8.5 + softness * 13.0, softness * 31.0));
        float r = length(p * float2(0.84, 1.16));
        float contact = 1.0 - smoothstep(0.16, 0.68 + softness * 0.24, r + edgeNoise * 0.28);
        float feather = 1.0 - smoothstep(0.26 + softness * 0.10, 1.16 + softness * 0.44, r + edgeNoise);
        float shape = saturate(max(contact * (0.26 - softness * 0.075), feather * 0.86));
        shape *= 0.82 + fineBreakup * 0.18;
        float localFixturePower = FixturePower(input.worldPos, time) * gLighting1.x;
        float flickerLinkedShadow = saturate(localFixturePower * lerp(0.42, 0.32, softness));
        float alpha = shape * flickerLinkedShadow * lerp(0.125, 0.070, softness) * (1.0 - saturate(gTransition0.z));
        if (alpha < 0.006) discard;
        return float4(0.0, 0.0, 0.0, alpha);
    }

    if ((materialId > 0.5 && materialId < 1.5) || (materialId > 1.5 && materialId < 2.5))
    {
        float floorMaterial = materialId < 1.5;
        float3 viewTS = float3(dot(V, T), dot(V, B), max(dot(V, N), 0.16));
        float parallaxScale = floorMaterial > 0.5 ? 0.012 : 0.005;
        float parallaxLayers = floorMaterial > 0.5 ? 8.0 : 5.0;
        float2 sampledRawUv = ReliefParallaxUv(rawUv, input.material, viewTS, parallaxScale, parallaxLayers);
        float mipBias = floorMaterial > 0.5 ? 0.30 : 0.08;
        float4 base = SampleMaterialAlbedo(sampledRawUv, input.material, mipBias);
        base.rgb = BackroomsBaseColor(base.rgb, materialId);
        float4 pbr = SampleMaterialProps(sampledRawUv, input.material, mipBias);
        float4 nh = SampleMaterialNormalHeight(sampledRawUv, input.material, mipBias + (floorMaterial > 0.5 ? 0.15 : 0.18));
        float3 nTex = normalize(nh.xyz * 2.0 - 1.0);
        float normalStrength = floorMaterial > 0.5 ? 0.48 : 0.22;
        nTex = normalize(float3(nTex.xy * normalStrength, nTex.z));
        float3 worldN = normalize(nTex.x * T + nTex.y * B + nTex.z * N);
        float dist = length(input.worldPos - cam);
        float flashlight = FlashlightAmount(input.worldPos, worldN);
        float3 overheadColor = LocalLampLightColor(input.worldPos, worldN, time) * gLighting1.x;
        float overhead = dot(overheadColor, float3(0.299, 0.587, 0.114));
        float sparkLight = SparkLight(input.worldPos, worldN);
        float3 exitGreen = ExitSignLight(input.worldPos, worldN, materialId);
        float exitGlow = max(exitGreen.r, max(exitGreen.g, exitGreen.b));
        float lift = gLighting0.z * (floorMaterial > 0.5 ? 0.018 : 0.040) * (1.0 - saturate(gTransition0.z));
        float aoMap = saturate(pbr.r);
        float roughness = saturate(pbr.g);
        float metallic = floorMaterial > 0.5 ? 0.0 : saturate(pbr.b);
        float3 dirtBase = base.rgb;
        ApplyWorldDirtOverlay(dirtBase, roughness, aoMap, materialId, input.worldPos, N);
        base.rgb = dirtBase;
        float3 diffuseColor = base.rgb * (1.0 - metallic);
        float3 specColor = lerp(float3(0.035, 0.035, 0.035), base.rgb, metallic);
        float ao = lerp(0.50, 1.0, aoMap);
        float3 color = diffuseColor * (gLighting0.z + flashlight + sparkLight + lift) * ao;
        color += diffuseColor * overheadColor * ao;
        color += diffuseColor * exitGreen * ao;
        float3 toLight = normalize(gShadow0.xyz - input.worldPos);
        float specFacing = saturate(dot(reflect(-toLight, worldN), V));
        float gloss = 1.0 - roughness;
        float specSharpness = lerp(10.0, 180.0, gloss);
        float specLight = flashlight + sparkLight * 0.58 + exitGlow * 0.46 + overhead * (floorMaterial > 0.5 ? 0.22 : 0.72);
        float fresnel = pow(1.0 - saturate(dot(worldN, V)), 5.0);
        float surfaceSpec = (pow(specFacing, specSharpness) * (0.08 + gloss * 0.82) +
            fresnel * (0.018 + gloss * 0.090)) * specLight;
        color += specColor * surfaceSpec * lerp(0.66, 1.0, aoMap);
        color *= 1.0 - CornerAO(input.worldPos, N);
        float fogScale = floorMaterial > 0.5 ? 1.22 : 1.34;
        color = lerp(color, float3(0.0, 0.0, 0.0), SceneFogBlock(dist, input.worldPos, fogScale));
        return float4(ApplyPost(color), 1.0);
    }

    if (materialId > 5.5 && materialId < 6.5)
    {
        float2 p = uv;
        float outerBevel = max(max(1.0 - smoothstep(0.0, 0.055, p.x),
                                   1.0 - smoothstep(0.0, 0.055, 1.0 - p.x)),
                               max(1.0 - smoothstep(0.0, 0.055, p.y),
                                   1.0 - smoothstep(0.0, 0.055, 1.0 - p.y)));
        float panelInset = max(1.0 - smoothstep(0.018, 0.050, abs(p.x - 0.18)),
                               1.0 - smoothstep(0.018, 0.050, abs(p.x - 0.82)));
        panelInset = max(panelInset, max(1.0 - smoothstep(0.018, 0.050, abs(p.y - 0.24)),
                                         1.0 - smoothstep(0.018, 0.050, abs(p.y - 0.76))));
        panelInset *= 0.35;
        float grain = Fbm3(float3(p * float2(2.2, 10.5), 4.7));
        float fineGrain = Fbm3(float3(p * float2(18.0, 84.0), 19.0));
        float plank = abs(frac(p.x * 7.0 + grain * 0.045) - 0.5);
        float plankLine = 1.0 - smoothstep(0.0, 0.030, plank);
        float grime = Fbm3(float3(input.worldPos.xz * 0.7 + p * 0.55, 21.0));
        float3 base = float3(0.34, 0.255, 0.160);
        base += (grain - 0.5) * float3(0.070, 0.050, 0.032);
        base += (fineGrain - 0.5) * float3(0.020, 0.016, 0.010);
        base -= (outerBevel * 0.24 + panelInset + plankLine * 0.28) * float3(0.045, 0.036, 0.024);
        base -= smoothstep(0.72, 0.97, grime) * float3(0.032, 0.027, 0.020);
        float3 worldN = normalize(N + T * ((grain - 0.5) * 0.020 + plankLine * 0.018));
)"
