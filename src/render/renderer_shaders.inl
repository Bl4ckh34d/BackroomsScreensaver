// Renderer shader source strings, compilation, shader object creation, and input layouts.
// Included inside Renderer private section before CreateStates().

    #include "renderer_shader_sources.inl"
    #include "renderer_shader_specialization.inl"
    #include "renderer_shader_objects.inl"
    #include "renderer_shader_input_layouts.inl"

    bool CreateShaders() {
        const char* shader = MainSceneShaderSource();
        ComPtr<ID3DBlob> vs;
        ComPtr<ID3DBlob> instancedVs;
        ComPtr<ID3DBlob> hs;
        ComPtr<ID3DBlob> ds;
        ComPtr<ID3DBlob> ps;
        ComPtr<ID3DBlob> shadowPs;
        const char* overlayShader = OverlayShaderSource();
        const char* texturedOverlayShader = TexturedOverlayShaderSource();
        const char* postShader = PostShaderSource();
        std::string generalShader(shader);
        std::string liquidShader;
        std::wstring shaderSplitError;
        if (!BuildSpecializedPixelShaders(shader, generalShader, liquidShader, shaderSplitError)) {
            StartupProfileLine(L"Failed to split liquid pixel shader source: " + shaderSplitError);
            return false;
        }
        StartupProfileLine(L"Shader split: general=" + std::to_wstring(generalShader.size()) +
            L" bytes, liquid=" + std::to_wstring(liquidShader.size()) + L" bytes");
        ComPtr<ID3DBlob> overlayVs;
        ComPtr<ID3DBlob> overlayPs;
        ComPtr<ID3DBlob> texturedOverlayVs;
        ComPtr<ID3DBlob> texturedOverlayPs;
        ComPtr<ID3DBlob> postVs;
        ComPtr<ID3DBlob> postPs;
        ComPtr<ID3DBlob> liquidPs;
        if (!CompileShader(generalShader.c_str(), "VSMain", "vs_4_0", vs)) return false;
        if (!CompileShader(generalShader.c_str(), "VSInstanced", "vs_4_0", instancedVs)) return false;
        if (d3dRuntime_.featureLevel >= D3D_FEATURE_LEVEL_11_0) {
            if (!CompileShader(generalShader.c_str(), "HSMain", "hs_5_0", hs)) return false;
            if (!CompileShader(generalShader.c_str(), "DSMain", "ds_5_0", ds)) return false;
        }
        const char* mainPsProfile = d3dRuntime_.featureLevel >= D3D_FEATURE_LEVEL_11_0 ? "ps_5_0" : "ps_4_0";
        if (!CompileShader(generalShader.c_str(), "PSMain", mainPsProfile, ps)) return false;
        if (!CompileShader(liquidShader.c_str(), "PSLiquid", mainPsProfile, liquidPs)) return false;
        if (!CompileShader(generalShader.c_str(), "ShadowPS", "ps_4_0", shadowPs)) return false;
        if (!CompileShader(overlayShader, "OverlayVS", "vs_4_0", overlayVs)) return false;
        if (!CompileShader(overlayShader, "OverlayPS", "ps_4_0", overlayPs)) return false;
        if (!CompileShader(texturedOverlayShader, "TexturedOverlayVS", "vs_4_0", texturedOverlayVs)) return false;
        if (!CompileShader(texturedOverlayShader, "TexturedOverlayPS", "ps_4_0", texturedOverlayPs)) return false;
        if (!CompileShader(postShader, "PostVS", "vs_4_0", postVs)) return false;
        if (!CompileShader(postShader, "PostPS", "ps_4_0", postPs)) return false;
        if (!CreateShaderObjects(vs, instancedVs, hs, ds, ps, liquidPs, shadowPs,
            overlayVs, overlayPs, texturedOverlayVs, texturedOverlayPs, postVs, postPs)) {
            return false;
        }
        return CreateShaderInputLayouts(vs, instancedVs, overlayVs, texturedOverlayVs);
    }
