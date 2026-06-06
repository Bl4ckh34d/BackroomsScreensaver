        if (shadowResources_.shadowDsv && settingsRuntime_.live.flashlightShadows && settingsRuntime_.live.flashlightShadowStrength > 0.001f &&
            flashlightIntensity > 0.001f && transitionFade < 0.995f) {
            renderProfile.Mark(L"BeginShadowPass");
            renderDepthShadow(shadowResources_.shadowDsv.Get(), shadowResources_.shadowMapSize, lightViewProj,
                lightPosFloat, lightDirFloat, shadowDistance, std::cos(shadowFov * 0.5f), true);
        }
        MarkGpuProfile(GpuProfileMarker::FlashlightShadow);
        if (fixtureShadowActive && settingsRuntime_.live.flashlightShadowStrength > 0.001f && transitionFade < 0.995f) {
            renderProfile.Mark(L"BeginFixtureShadow");
            renderDepthShadow(shadowResources_.fixtureShadowDsv.Get(), shadowResources_.fixtureShadowMapSize, fixtureLightViewProj,
                fixtureShadowPos, fixtureShadowDir, fixtureShadowRange, std::cos(116.0f * 0.5f * kPi / 180.0f), false);
        }
        MarkGpuProfile(GpuProfileMarker::FixtureShadow);
        UploadSceneConstants(cb);
        renderProfile.Mark(L"UploadSceneConstants");
        UploadLampDamageTexture();
        renderProfile.Mark(L"UploadLampDamageTexture");
        UpdateMenuPosterTexture();
        UpdateCustomMenuTexture();
        MarkGpuProfile(GpuProfileMarker::Uploads);
