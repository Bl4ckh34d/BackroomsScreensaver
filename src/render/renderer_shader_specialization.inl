// Main-scene shader source specialization.
// Included inside Renderer private section.

    bool BuildSpecializedPixelShaders(const char* shader,
        std::string& generalShader,
        std::string& liquidShader,
        std::wstring& shaderSplitError) {
            const std::string shaderText(shader);
            const std::string psMainMarker = "float4 PSMain(VSOut input) : SV_TARGET";
            const std::string setupEndMarker = "    float2 uv = frac(rawUv);\n\n";
            const std::string liquidStartMarker =
                "    if ((materialId > 13.5 && materialId < 14.5) || (materialId > 24.5 && materialId < 25.5))\n";
            const std::string nextBranchMarker =
                "    if (input.material > 11.05 && input.material < 11.45)\n";
            size_t psMainStart = shaderText.find(psMainMarker);
            if (psMainStart == std::string::npos) {
                shaderSplitError = L"PSMain marker not found";
                return false;
            }
            size_t setupEnd = shaderText.find(setupEndMarker, psMainStart);
            if (setupEnd == std::string::npos) {
                shaderSplitError = L"PSMain setup marker not found";
                return false;
            }
            setupEnd += setupEndMarker.size();
            size_t liquidStart = shaderText.find(liquidStartMarker, setupEnd);
            if (liquidStart == std::string::npos) {
                shaderSplitError = L"liquid branch marker not found";
                return false;
            }
            size_t liquidEnd = shaderText.find(nextBranchMarker, liquidStart + liquidStartMarker.size());
            if (liquidEnd == std::string::npos) {
                shaderSplitError = L"post-liquid branch marker not found";
                return false;
            }

            const std::string liquidBlock = shaderText.substr(liquidStart, liquidEnd - liquidStart);
            const std::string dynamicLiquidFallback = R"(    if ((materialId > 13.5 && materialId < 14.5) || (materialId > 24.5 && materialId < 25.5))
    {
        float waterLiquid = step(24.5, materialId);
        float rawSeed = frac(input.material);
        float2 liquidUv = frac(rawUv);
        float2 local = liquidUv * 2.0 - 1.0;
        float edge = 1.0 - smoothstep(0.74, 1.08, dot(local * float2(0.82, 1.08), local * float2(0.82, 1.08)));
        float breakup = 0.72 + 0.28 * Hash21(float2(floor(liquidUv.x * 18.0) + rawSeed * 31.0, floor(liquidUv.y * 18.0)));
        float alpha = edge * breakup;
        if (alpha < lerp(0.050, 0.028, waterLiquid)) discard;
        float3 worldN = normalize(N + T * (Hash21(floor(liquidUv * 16.0) + rawSeed) - 0.5) * 0.025);
        float flashlight = FlashlightAmount(input.worldPos, worldN);
        float overhead = LocalLampLight(input.worldPos, worldN, time) * gLighting1.x;
        float sparkLight = SparkLight(input.worldPos, worldN);
        float lightEnergy = saturate(flashlight * 0.72 + overhead * 0.28 + sparkLight * 0.34 + gLighting0.z * 0.06);
        float3 bloodColor = lerp(float3(0.40, 0.005, 0.0012), float3(0.10, 0.0007, 0.0002), alpha);
        float3 waterColor = lerp(float3(0.010, 0.011, 0.010), float3(0.0045, 0.0052, 0.0048), alpha);
        float3 color = lerp(bloodColor * (0.18 + lightEnergy * 0.90), waterColor * (0.70 + lightEnergy * 0.32), waterLiquid);
        float3 toLight = normalize(gShadow0.xyz - input.worldPos);
        float facing = saturate(dot(reflect(-toLight, worldN), V));
        color += lerp(float3(0.32, 0.010, 0.003), float3(0.12, 0.13, 0.12), waterLiquid) *
            pow(facing, lerp(88.0, 120.0, waterLiquid)) * saturate(flashlight + overhead * 0.28) * alpha;
        float dist = length(input.worldPos - cam);
        color = lerp(color, float3(0.0, 0.0, 0.0), SceneFogBlock(dist, input.worldPos, 1.0) * 0.92);
        return float4(ApplyPost(color), alpha * lerp(0.62, 0.28, waterLiquid));
    }

)";
            generalShader = shaderText;
            generalShader.replace(liquidStart, liquidEnd - liquidStart, dynamicLiquidFallback);

            std::string liquidSetup = shaderText.substr(psMainStart, setupEnd - psMainStart);
            size_t liquidEntry = liquidSetup.find("float4 PSMain");
            if (liquidEntry == std::string::npos) {
                shaderSplitError = L"liquid entry marker not found";
                return false;
            }
            liquidSetup.replace(liquidEntry, std::strlen("float4 PSMain"), "float4 PSLiquid");
            liquidShader = shaderText.substr(0, psMainStart);
            liquidShader += liquidSetup;
            liquidShader += liquidBlock;
            liquidShader += "    discard;\n    return float4(0.0, 0.0, 0.0, 0.0);\n}\n";
            return true;
    }
