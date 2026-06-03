        bool fleshLightingSuppressed =
            (settingsRuntime_.live.fleshAlwaysOn && settingsRuntime_.live.fleshFlickerIntensity > 0.001f) ||
            (scareRuntime_.fleshFlickerTimer > 0.0f && scareRuntime_.fleshFlickerDuration > 0.001f);
        if (fleshLightingSuppressed) {
            // Shader uses transition0.z to keep the entire flesh event flashlight-only, including strobe gaps.
            cb.transition0.z = 1.0f;
            cb.lighting0.z = 0.0f;
            cb.lighting1.x = 0.0f;
            cb.lighting1.y = 0.0f;
            cb.lighting1.z = 0.0f;
        }
        bool useFleshTessellation = cb.horror0.x > 0.01f &&
            d3dRuntime_.featureLevel >= D3D_FEATURE_LEVEL_11_0 &&
            shaders_.hullShader && shaders_.domainShader &&
            cb.horror0.w > 0.001f;
        std::array<XMFLOAT4, 2> sparkLights = fleshLightingSuppressed
            ? std::array<XMFLOAT4, 2>{XMFLOAT4{0.0f, 0.0f, 0.0f, 0.0f}, XMFLOAT4{0.0f, 0.0f, 0.0f, 0.0f}}
            : ActiveSparkLights();
        cb.sparkLight0 = sparkLights[0];
        cb.sparkLight1 = sparkLights[1];
