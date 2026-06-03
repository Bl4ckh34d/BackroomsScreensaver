
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
