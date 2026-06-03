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
