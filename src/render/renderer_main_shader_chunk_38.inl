R"(
        float dist = length(input.worldPos - cam);
        float flashlight = FlashlightAmount(input.worldPos, worldN);
        float overhead = LocalLampLight(input.worldPos, worldN, time) * gLighting1.x;
        float sparkLight = SparkLight(input.worldPos, worldN);
        float3 exitGreen = ExitSignLight(input.worldPos, worldN, materialId);
        float3 color = base * (gLighting0.z + overhead * 0.72 + flashlight + sparkLight);
        color += base * exitGreen * 0.55;
        color += exitGreen * 0.040;
        float3 toLight = normalize(gShadow0.xyz - input.worldPos);
        float facing = saturate(dot(reflect(-toLight, worldN), V));
        color += float3(0.75, 0.56, 0.34) * pow(facing, 54.0) * 0.075 * (flashlight + sparkLight * 0.45);
        color *= 1.0 - CornerAO(input.worldPos, worldN) * 0.55;
        color = lerp(color, float3(0.0, 0.0, 0.0), SceneFogBlock(dist, input.worldPos, 1.0));
        return float4(ApplyPost(color), 1.0);
    }

    float3 viewTS = float3(dot(V, T), dot(V, B), max(dot(V, N), 0.18));
    float parallaxScale = 0.0;
    if (materialId < 0.5) parallaxScale = 0.022;
    else if (materialId < 1.5) parallaxScale = 0.012;
    else if (materialId < 2.5) parallaxScale = 0.005;
    float2 sampledRawUv = rawUv;
    if (materialId > 6.5 && materialId < 7.5)
    {
        sampledRawUv.y = 1.0 - sampledRawUv.y;
    }
    sampledRawUv = ReliefParallaxUv(sampledRawUv, input.material, viewTS, parallaxScale, materialId < 0.5 ? 10.0 : 8.0);

    float floorMipBias = (materialId > 0.5 && materialId < 1.5) ? 1.75 : 0.0;
    float4 base = SampleMaterialAlbedo(sampledRawUv, input.material, floorMipBias);
    base.rgb = BackroomsBaseColor(base.rgb, materialId);
    if (IsDoorMaterial(input.material))
    {
        float luma = dot(base.rgb, float3(0.299, 0.587, 0.114));
        base.rgb = lerp(luma * float3(0.54, 0.38, 0.20), base.rgb, 0.16);
    }
    if (IsDoorFrameMaterial(input.material))
    {
        float luma = dot(base.rgb, float3(0.299, 0.587, 0.114));
        base.rgb = lerp(float3(0.72, 0.72, 0.67), float3(0.96, 0.95, 0.88), saturate(luma));
    }
    if ((materialId > 15.5 && materialId < 17.5) || (materialId > 21.5 && materialId < 22.5))
    {
        float hi = max(base.r, max(base.g, base.b));
        float lo = min(base.r, min(base.g, base.b));
        float lum = dot(base.rgb, float3(0.299, 0.587, 0.114));
        float neutralBright = smoothstep(0.46, 0.72, lum) * (1.0 - smoothstep(0.08, 0.24, hi - lo));
        base.rgb = lerp(base.rgb, float3(0.035, 0.038, 0.036), neutralBright);
    }
    if (materialId > 21.5 && materialId < 22.5)
    {
        float seed = frac(input.material);
        float3 chairTintA = lerp(float3(0.62, 0.70, 0.64), float3(0.74, 0.60, 0.56), smoothstep(0.06, 0.30, seed));
        float3 chairTintB = lerp(float3(0.58, 0.66, 0.76), float3(0.72, 0.68, 0.54), smoothstep(0.30, 0.62, seed));
        float3 chairTintC = lerp(float3(0.58, 0.55, 0.68), float3(0.66, 0.72, 0.61), smoothstep(0.62, 0.96, seed));
        float3 chairTint = seed < 0.30 ? chairTintA : (seed < 0.62 ? chairTintB : chairTintC);
        float fabricMask = 1.0 - smoothstep(0.30, 0.72, max(base.r, max(base.g, base.b)) - min(base.r, min(base.g, base.b)));
        base.rgb = lerp(base.rgb, base.rgb * chairTint, fabricMask * 0.52);
    }
    float4 pbr = SampleMaterialProps(sampledRawUv, input.material, floorMipBias);
    if (materialId > 3.5 && base.a < 0.08) discard;

    float4 nh = SampleMaterialNormalHeight(sampledRawUv, input.material, floorMipBias);
    float3 nTex = normalize(nh.xyz * 2.0 - 1.0);
    float normalStrength = 0.55;
    if (materialId < 0.5) normalStrength = 0.72;
    else if (materialId > 0.5 && materialId < 1.5) normalStrength = 0.48;
    else if (materialId > 1.5 && materialId < 2.5) normalStrength = 0.22;
    nTex = normalize(float3(nTex.xy * normalStrength, nTex.z));
    float3 worldN = normalize(nTex.x * T + nTex.y * B + nTex.z * N);

    float dist = length(input.worldPos - cam);
    float flashlight = FlashlightAmount(input.worldPos, worldN);
    float sparkLight = SparkLight(input.worldPos, worldN);
    float3 exitGreen = ExitSignLight(input.worldPos, worldN, materialId);
    float doorLightBlock = DoorRoomSideLightBlock(input.worldPos, N, materialId);
    exitGreen *= (1.0 - doorLightBlock);
    float exitGlow = max(exitGreen.r, max(exitGreen.g, exitGreen.b));

    float fixture = FixturePower(input.worldPos, time);
    float3 overheadColor = LocalLampLightColor(input.worldPos, worldN, time) * gLighting1.x;
    float overhead = dot(overheadColor, float3(0.299, 0.587, 0.114));

    float ambient = gLighting0.z;
    float aoMap = saturate(pbr.r);
    float roughness = saturate(pbr.g);
    float metallic = saturate(pbr.b);
    if (materialId > 25.5 && materialId < 26.5)
    {
        roughness = min(roughness, 0.34);
        aoMap = max(aoMap, 0.74);
    }
    float3 dirtBase = base.rgb;
    ApplyWorldDirtOverlay(dirtBase, roughness, aoMap, materialId, input.worldPos, N);
    base.rgb = dirtBase;
)"
