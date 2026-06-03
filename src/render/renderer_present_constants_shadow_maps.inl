        cb.shadow0 = {
            lightPosFloat.x,
            lightPosFloat.y,
            lightPosFloat.z,
            settingsRuntime_.live.flashlightShadows ? settingsRuntime_.live.flashlightShadowStrength : 0.0f
        };
        cb.shadow1 = {
            lightDirFloat.x,
            lightDirFloat.y,
            lightDirFloat.z,
            settingsRuntime_.live.flashlightShadowBias
        };
        cb.shadow2 = {
            1.0f / static_cast<float>(std::max<UINT>(1, shadowResources_.shadowMapSize)),
            shadowDistance,
            coneOuter,
            coneInner
        };
        cb.fixtureShadow0 = {
            fixtureShadowPos.x,
            fixtureShadowPos.y,
            fixtureShadowPos.z,
            fixtureShadowActive ? std::clamp(settingsRuntime_.live.flashlightShadowStrength * 0.72f, 0.0f, 0.62f) : 0.0f
        };
        cb.fixtureShadow1 = {
            std::max(settingsRuntime_.live.flashlightShadowBias * 1.35f, 0.00062f),
            fixtureShadowRange,
            1.0f / static_cast<float>(std::max<UINT>(1, shadowResources_.fixtureShadowMapSize)),
            0.0f
        };
        fixtureShadowRuntime_.active = fixtureShadowActive;
        fixtureShadowRuntime_.position = fixtureShadowPos;
        fixtureShadowRuntime_.range = fixtureShadowRange;
