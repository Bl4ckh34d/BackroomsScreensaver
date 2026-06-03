R"(
                                 0.70 + lens * 0.20 - edge * 0.12);
        float hueSeed = frac(input.material * 19.37 + 0.41);
        float hueWarm = (hueSeed - 0.5) * 0.035;
        lampBase *= float3(1.0 + hueWarm, 1.0 + hueWarm * 0.22, 1.0 - hueWarm * 0.65);
        float3 offBase = float3(0.91 + lens * 0.07 - edge * 0.055,
                                0.92 + lens * 0.07 - edge * 0.050,
                                0.86 + lens * 0.06 - edge * 0.045);
        float2 lampOrigin = gMaze0.xy + gMaze0.zw * 0.5;
        float2 lampCell = floor((input.worldPos.xz - lampOrigin) / gMaze0.zw + 0.5);
        float lampDirt = LampDirtAmount(lampCell);
        float2 dirtUv = uv + float2(Hash21(lampCell + 19.0), Hash21(lampCell + 29.0)) * 0.19;
        float grimePatch = smoothstep(0.38, 0.82,
            Fbm3(float3(dirtUv * 3.5 + lampCell * 0.071, 691.0)) +
            (Fbm3(float3(dirtUv * 11.0 - lampCell * 0.13, 709.0)) - 0.5) * 0.18);
        float amberFilm = saturate(grimePatch * lampDirt * (0.42 + edge * 0.24));
        float flyMask = 0.0;
        float flyClusterRoll = Hash21(lampCell + 311.0);
        float flyDensity = saturate((lampDirt - 0.18) * 1.35 + (flyClusterRoll - 0.52) * 1.15);
        [unroll]
        for (int fi = 0; fi < 9; ++fi)
        {
            float2 flySeed = lampCell + float2((float)fi * 17.37 + 43.0, (float)fi * 31.11 + 97.0);
            float2 center = float2(Hash21(flySeed), Hash21(flySeed + 73.0));
            center = lerp(float2(0.18, 0.20), float2(0.82, 0.80), center);
            float angle = Hash21(flySeed + 151.0) * 6.2831853;
            float2 axis = float2(cos(angle), sin(angle));
            float2 perp = float2(-axis.y, axis.x);
            float2 d = uv - center;
            float radius = lerp(0.0032, 0.0095, Hash21(flySeed + 131.0)) * lerp(0.78, 1.34, lampDirt);
            float anisotropy = lerp(1.10, 2.15, Hash21(flySeed + 167.0));
            float elongatedDist = sqrt(pow(dot(d, axis) / anisotropy, 2.0) + pow(dot(d, perp), 2.0));
            float body = exp(-(elongatedDist * elongatedDist) / max(0.00001, radius * radius * 2.6));
            float blur = smoothstep(0.011, 0.0, length(d)) * 0.045;
            float gate = smoothstep(0.20, 0.92, flyDensity + Hash21(flySeed + 211.0) * 0.36 - (float)fi * 0.055);
            flyMask = max(flyMask, saturate(body * 0.46 + blur) * gate);
        }
        float dustSpecks = smoothstep(0.80, 0.985, Noise3(float3(uv * 78.0 + lampCell * 3.0, 733.0))) * lampDirt * 0.42;
        float insectGrime = saturate(flyMask + dustSpecks * 0.45);
        float3 grimeTint = float3(0.92, 0.66, 0.23);
        lampBase = lerp(lampBase, lampBase * (0.70 + amberFilm * 0.08) + grimeTint * (0.30 + amberFilm * 0.18), amberFilm);
        lampBase = lerp(lampBase, float3(0.040, 0.026, 0.012), insectGrime * 0.82);
        offBase = lerp(offBase, offBase * float3(0.78, 0.68, 0.45) + grimeTint * 0.18, amberFilm * 0.86);
        float lampDamage = LampDamageAtWorld(input.worldPos.xz);
        float brokenVisual = materialId < 3.5 ? smoothstep(0.985, 0.995, lampDamage) : 1.0;
        float3 base = lerp(lampBase, offBase, brokenVisual);
        float dirtGlow = lerp(1.14, 0.48, smoothstep(0.04, 0.96, lampDirt));
        float emit = materialId < 3.5
            ? LampVisualPower(input.material, input.worldPos, time) * 3.15 * (1.0 - saturate(gTransition0.z)) * (1.0 - brokenVisual) *
                dirtGlow * (1.0 - insectGrime * 0.24)
            : 0.0;
        float3 emitTint = lerp(float3(1.0, 0.985, 0.90), float3(1.0, 0.70, 0.30), saturate(lampDirt + amberFilm * 0.65));
        float passiveOn = gLighting0.z;
        float passiveOff = gLighting0.z * 0.48 +
            FlashlightAmount(input.worldPos, N) * 0.86 +
            LocalLampLight(input.worldPos, N, time) * gLighting1.x * 0.22;
        float passiveLight = lerp(passiveOn, passiveOff, brokenVisual);
        float3 color = base * passiveLight + base * emit * emitTint;
        float dist = length(input.worldPos - cam);
        color = lerp(color, float3(0.0, 0.0, 0.0), SceneFogBlock(dist, input.worldPos, 1.34));
        return float4(ApplyPost(color), 1.0);
    }

    if (materialId > 11.5 && materialId < 12.5)
    {
        float dist = length(input.worldPos - cam);
        float fog = saturate((dist - gFog0.x) / max(0.01, gFog0.y - gFog0.x));
        float variant = frac(input.material);
        if (variant > 0.55)
        {
            float2 d = uv - 0.5;
            float radial = exp(-dot(d, d) * 5.4);
            float curl = Hash21(floor((uv + time * float2(0.05, -0.09)) * 9.0 + input.material * 23.0));
            float life = saturate((variant - 0.55) / 0.40);
            float alpha = radial * (0.18 + curl * 0.16) * life;
            if (alpha < 0.018) discard;
            float flashlight = FlashlightAmount(input.worldPos, N);
            float nonFleshLight = 1.0 - saturate(gTransition0.z);
            float3 color = float3(0.46, 0.50, 0.47) * (flashlight * 0.14 + (0.18 + gLighting0.z) * nonFleshLight);
            color = lerp(color, float3(0.0, 0.0, 0.0), fog * gFog0.z * 0.75);
            return float4(ApplyPost(color), alpha);
        }

        float3 materialUv = MaterialUV(rawUv, input.material);
        float4 base = gAlbedo.Sample(gSampler, materialUv);
        if (base.a < 0.03) discard;
        fog = 1.0 - exp(-fog * fog * 1.4);
        float pulse = 0.96 + 0.04 * sin(time * 7.1 + input.material * 17.0);
        float nonFleshLight = 1.0 - saturate(gTransition0.z);
        if (nonFleshLight < 0.001) discard;
        float3 color = (float3(13.0, 0.35, 0.10) * base.a * pulse + base.rgb * 3.2) * nonFleshLight;
        float fogBlock = saturate(fog * gFog0.z * 1.55);
        float fogVisibility = pow(1.0 - fogBlock, 2.7);
        color *= fogVisibility;
        return float4(saturate(ApplyPost(color) + color * 0.35 * fogVisibility), saturate(base.a * 1.55 * fogVisibility));
    }
)"
